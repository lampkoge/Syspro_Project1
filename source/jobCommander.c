#include "../include/jobCommander.h"
#include "../include/myFunctions.h"
#include "../include/jobExecutorServer.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <signal.h>

#define READ 0
#define WRITE 1

#define BUFFER 2048

/* For synchronization between fork'd server and commander */
int server_open = 0;
int ready = 0;

void signal_handler(int signum, siginfo_t* info, void* context)
{
    switch (signum)
    {
        case SIGUSR1:
            ready = 1;
            break;
        default:
            break;
    }
}

void jobCommander(char* cmd)
{
    pid_t pid, server_pid;

    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);

    /* If server file does not exist - create it and run server */
    int server_fd = open(SERVER_FILE_PATH, O_RDONLY);
    if (server_fd == -1)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("[Client] Error on server fork");
            exit(-1);
        }
        /* Child process [Server] */
        if (pid == 0)
        {
            server_pid = getpid();

            char* args[3] = { SERVER_EXE, "[jobCommander]" };
            /* The array of pointers MUST be terminated by a NULL pointer */
            args[2] = NULL;
            if (execvp(SERVER_EXE, args) == -1)
            {
                perror("Execvp failed");
                exit(-1);
            }
        }
        /* Parent process [jobCommander] */
        else
        {
            server_pid = pid;
        }
    }
    /* If file exists, get server pid */
    else
    {
        char* str = malloc(sizeof(ssize_t) + 1);
        ssize_t size = read(server_fd, str, sizeof(ssize_t) + 1);
        str[size] = '\0';
        server_pid = atoi(str);

        server_open = 1;
    }

    /* 
    Determine whether the server is up and running 
        - either there is no file so wait until forked process creates it
        - or there is a file so server_open == 1
    */
    while (!server_open && (open(SERVER_FILE_PATH, O_RDONLY) == -1));

    pid = getpid();

    /* Create the named pipes to communicate with the server */
    char* pipe_name = getPipeName(pid, WRITE);
    if ((mkfifo(pipe_name, PERM) < 0))
    {
        perror("[Client] Failed to create named pipe.");
        exit(1);
    }

    /* Signal server that commander is ready to send message */
    kill(server_pid, SIGUSR1);

    int write_fd;
    if ((write_fd = open(pipe_name, O_WRONLY)) < 0)
    {
        perror("[Client] Error on open pipe.");
        exit(1);
    }
    /* Write message to server */
    if (write(write_fd, cmd, strlen(cmd)) != strlen(cmd))
    {
        perror("[Client] Error on write.");
        exit(1);
    }

    /* Wait for server to write message back - that happens for all commands except setConcurrency */
    char* command = strtok(cmd, " ");
    if (strcmp(cmd, "setConcurrency") != 0)
    {
        /* Wait for server to send signal to read from pipe */
        char* pipe_name_r = getPipeName(pid, READ);
        
        while(!ready);

        int read_fd;
        if ((read_fd = open(pipe_name_r, O_RDONLY)) < 0)
        {
            perror("Client open pipe error");
            exit(1);
        }
        int n;
        char* msg = malloc(sizeof(char) * BUFFER);
        if ((n = read(read_fd, msg, BUFFER)) <= 0)
        {
            perror("Client read error");
            exit(1);
        }
        msg[n] = '\0';
        
        printf("%s", msg);

        unlink(pipe_name_r);

        free(pipe_name_r);
        free(msg);
    }

    unlink(pipe_name);

    free(pipe_name);
}

