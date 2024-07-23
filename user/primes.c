#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// function to implement the prime sieve logic
void prime_sieve(int p[2]) {
    int prime;
    int n;
    close(p[1]);// Close the write end of the current pipe
    
    // Read the first number from the pipe, which is the prime
    if(read(p[0], &prime, sizeof(prime)) == 0) {
        // if no number is read, close the read end and exit
        //end of the recursion
        close(p[0]);
        exit(0);
    }
    printf("prime %d\n", prime);
   
    //create a new pipe
    int next_pipe[2];
    pipe(next_pipe);

    //create a new process
    if(fork() == 0) {
        //child process
        close(p[0]);
        prime_sieve(next_pipe);//recursion
    } 
    else {
        //parent process
        close(next_pipe[0]);
        
        //read numbers from the current pipe
        //filter out the multiples of the prime
        //write the rest to the next pipe
        while(read(p[0], &n, sizeof(n)) > 0) {
            if(n % prime != 0) {
                write(next_pipe[1], &n, sizeof(n));
            }
        }
        
        close(p[0]);
        close(next_pipe[1]);
        wait(0);//wait for the child process to finish
    }
}

int main(int argc, char *argv[]) {
    //create the initial pipe
    int p[2];
    pipe(p);
    
    //create the first sieve process
    if(fork() == 0) {
        //child process: start the sieve with the initial pipe
        prime_sieve(p);
    } else {
        //parent process
        close(p[0]);//close the read end of the initial pipe
        
        //write into the pipe
        for(int i = 2; i <= 35; i++) {
            write(p[1], &i, sizeof(i));
        }

        close(p[1]);//close the write end of the initial pipe
        wait(0);//wait for the sieve process to finish
        exit(0);
    }
    return 0;
}

