#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

// enable or disable debug information
#define DEBUG 0

// macro to handle debug prints
#define debug(codes) if(DEBUG) {codes}

// function to fork and execute the command with given arguments
void xargs_exec(char* program, char** paraments);

// function to read input lines and execute the command with arguments
//first_arg: an array of strings that contains the initial list of arguments passed from the command line
//size: the size of the 'first_arg' array
//program_name: the name of the program to be executed
//n: the number of arguments that 'xargs' should pass to the command at a time
void xargs(char** first_arg, int size, char* program_name, int n)
{
    char buf[1024]; // buffer to store the input line
    debug(
        for (int i = 0; i < size; ++i) {
            printf("first_arg[%d] = %s\n", i, first_arg[i]);
        }
    )
    char *arg[MAXARG]; // argument list for the command
    int m = 0; // position index in the buffer

    // read from standard input, one character at a time
    while (read(0, buf + m, 1) == 1) {
        if (m >= 1024) {
            fprintf(2, "xargs: arguments too long.\n");
            exit(1);
        }
        // if newline is encountered, process the buffer
        if (buf[m] == '\n') {
            buf[m] = '\0'; // null-terminate the string
            debug(printf("this line is %s\n", buf);)
            memmove(arg, first_arg, sizeof(char*) * size);// copy the initial arguments to the arg array

            // set argument index after the initial arguments
            int argIndex = size;
            if (argIndex == 0) {
                arg[argIndex] = program_name;
                argIndex++;
            }

            // allocate memory for the new argument and copy it
            arg[argIndex] = buf;
            arg[argIndex + 1] = 0; // Ensure the argument list is null-terminated
            debug(
                for (int j = 0; j <= argIndex; ++j)
                    printf("arg[%d] = *%s*\n", j, arg[j]);
            )
            
            xargs_exec(program_name, arg);// execute the command with arguments
            m = 0; // reset the buffer index for the next input line
        } else {
            m++; // move to the next character position in the buffer
        }
    }
}

// function to fork a new process and execute the command
void xargs_exec(char* program, char** paraments)
{
    if (fork() > 0) {
        // parent process waits for the child process to complete
        wait(0);
    } else {
        //child process executes the command
        debug(
            printf("child process\n");
            printf("    program = %s\n", program);
            for (int i = 0; paraments[i] != 0; ++i) {
                printf("    paraments[%d] = %s\n", i, paraments[i]);
            }
        )
        if (exec(program, paraments) == -1) {
            // If exec fails, print an error message
            fprintf(2, "xargs: Error exec %s\n", program);
        }
        debug(printf("child exit");)
    }
}

// main function to handle command line arguments and call xargs
int main(int argc, char* argv[])
{
    debug(printf("main func\n");)
    if (argc < 2) {
        // Print usage information if insufficient arguments are provided
        fprintf(2, "Usage: xargs <command> [args...]\n");
        exit(1);
    }

    int n = 1;// Default value for the number of arguments to pass to the command
    char *name = argv[1]; // Default command to execute
    int first_arg_index = 1; // Index of the first argument in argv

    // Check if -n option is provided
    if (argc >= 4 && strcmp(argv[1], "-n") == 0) {
        n = atoi(argv[2]); // Get the value of n
        name = argv[3]; // Command to execute
        first_arg_index = 3; // Adjust the first argument index
    }

    debug(
        printf("command = %s\n", name);
    )

    // Call the xargs function with the command and arguments
    xargs(argv + first_arg_index, argc - first_arg_index, name, n);
    exit(0);
}

