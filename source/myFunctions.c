#include "../include/myFunctions.h"

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

/* Converts an integer to string - NEED TO BE free'd */
char* intToString(int num){
    int len = snprintf(NULL, 0, "%d", num);
    char *str = malloc(sizeof(char) * (len + 1));
    snprintf(str, len + 1, "%d", num);
    return str;
}

/* 
    Creates the pipe name given the jobCommander's pid, 
    based on mode (0-read, 1-write) - for server is (0-write, 1-read)
    ex. pipe_name = "fifo.0.12345"
    Pipe name should be free'd
*/
char* getPipeName(int pid, int mode){
    char* mode_str = intToString(mode);
    char* str_pid = intToString(pid);
    char* pipe_name = malloc(sizeof(char) * strlen(FIFO) + strlen(str_pid) + strlen(mode_str) + 2);
    strcpy(pipe_name, FIFO);
    strcat(pipe_name, mode_str);
    strcat(pipe_name, ".");
    strcat(pipe_name, str_pid);

    free(str_pid);
    free(mode_str);

    return pipe_name;
}

/* Gets commands passed to jobCommander - if non are passed it returns null */
char* extractCommand(int argc, char** argv){
    int length = 0;
    for (int i=1; i<argc; i++)
        length += strlen(argv[i]) + 1;
    if (length == 0 || argc <= 1)
        return NULL;
    char* cmd = malloc(sizeof(char) * length);
    for (int i=1; i<argc; i++){
        strcat(cmd, argv[i]);
        if (i != argc-1)
            strcat(cmd, " ");
    }
    return cmd;
}

/* Creates server file - the argument passed is the origin for error handling reasons */
void createServerFile(char* origin){
    char* pid_str = intToString(getpid());
    pid_t server_fd = creat(SERVER_FILE_PATH, 0644);
    
    if (server_fd == -1){
        /* Create error message */
        char* error = "Error when creating server file.";
        char* concat_error = malloc(strlen(error) + strlen(origin) + 1);
        strcpy(concat_error, error);
        strcat(concat_error, origin);

        perror(concat_error);
        free(concat_error);
        exit(-1);
    }

    write(server_fd, pid_str, strlen(pid_str));
    
    free(pid_str);
    close(server_fd);
}