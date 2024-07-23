#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    if(argc != 2){
        fprintf(2, "Usage: sleep times\n");
        exit(1);
    }
    int time = atoi(argv[1]);
    if(time <= 0){
        fprintf(2, "Error: time must be a positive integer.\n");
        exit(1);
    }
    if(sleep(time) != 0){
        fprintf(2, "Error in sleep sys_call!\n");
        exit(1);
    }
    exit(0);
}

