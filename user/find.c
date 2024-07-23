#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

//recursive function to find files with a specific name within a directory and its subdirectories
void find(char *path, char *target) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    //attempt to open the directory specified by 'path'
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // get the directory status
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // check if it's a directory
    if (st.type != T_DIR) {
        fprintf(2, "find: %s is not a directory\n", path);
        close(fd);
        return;
    }

    // copy the path to the buffer and prepare to append filenames
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    // read directory entries one by one 
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        // skip entries with an inum of 0(invalid entries)
        if (de.inum == 0)
            continue;

        // skip "." and ".."
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;

        // copy the directory entry name to the buffer
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;// add '/0'

        // get the status of the directory entry
        if (stat(buf, &st) < 0) {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }

        // if the name matches the target, print the full path
        if (strcmp(de.name, target) == 0) {
            printf("%s\n", buf);
        }

        // if it's a directory, recursively find within it
        if (st.type == T_DIR) {
            find(buf, target);
        }
    }

    // close the directory
    close(fd);
}

int main(int argc, char *argv[]) {
    // check if the correct number of arguments are provided
    if (argc < 3) {
        fprintf(2, "Usage: find <path> <filename>\n");
        exit(1);
    }
    // call the find function with the provided path and target filename
    find(argv[1], argv[2]);
    exit(0);
}

