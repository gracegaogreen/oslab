#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    int p2c[2], c2p[2]; //two pipes: p2c for parent-to-child, c2p for child-to-parent

    // create the pipes
    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        fprintf(2, "Error: pipe() error.\n");
        exit(1);
    }
    
    // fork the process to create a child
    int pid = fork();
    if (pid == -1) {
        fprintf(2, "Error: fork() error.\n");
        exit(1);
    }
    
    // child process
    if (pid == 0) {
        char buffer[1];
        close(p2c[1]); // close the write end of p2c
        close(c2p[0]); // close the read end of c2p

        // read data from parent
        if (read(p2c[0], buffer, 1) == -1) {
            fprintf(2, "Error: read() error in child.\n");
            exit(1);
        }
        
        close(p2c[0]); // close the read end of p2c
        printf("%d: received ping\n", getpid());// print received message
        
        // write data back to parent
        if (write(c2p[1], buffer, 1) == -1) {
            fprintf(2, "Error: write() error in child.\n");
            exit(1);
        }
        close(c2p[1]); // close the write end of c2p

        exit(0);
    } 
    // parent process
    else {
        char buffer[1];
        buffer[0] = 'a'; // data to send

        close(p2c[0]); // close the read end of p2c
        close(c2p[1]); // close the write end of c2p

        // 2rite data to child
        if (write(p2c[1], buffer, 1) == -1) {
            fprintf(2, "Error: write() error in parent.\n");
            exit(1);
        }
        close(p2c[1]); // close the write end of p2c
        wait(0);// wait for the child process to complete

        // read data from the child
        if (read(c2p[0], buffer, 1) == -1) {
            fprintf(2, "Error: read() error in parent.\n");
            exit(1);
        }
        close(c2p[0]); // close the read end of c2p

        // print received message
        printf("%d: received pong\n", getpid());

        exit(0);
    }
}

