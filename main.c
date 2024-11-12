#include "include/myFunctions.h"
#include "include/jobCommander.h"

#include "include/Queue.h"

/*
ps -> process status 
pkill "process name"
*/

int main(int argc, char** argv){
    /* Handle given command */
    char* cmd = extractCommand(argc, argv);
    if (cmd != NULL && strlen(cmd) > 0)
        jobCommander(cmd);
    else
        printf("Invalid input\n");
    free(cmd);
   
    return 0;
}