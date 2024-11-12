#include "../include/jobExecutorServer.h"
#include "../include/myFunctions.h"
#include <bits/sigaction.h>

#define BUFFER 2048
#define JOB "job_"
#define READ 1
#define WRITE 0

int concurrency = 1;
int stop = 0;
int jobID_number = 0;
Queue job_queue;            // queue with jobs to be run
Queue running_job_queue;    // queue with currently running jobs
int commander_ready = 0;

//find /tmp -type p -name 'fifo.*' -exec rm -f {} \; - if it all goes wrong (run on /tmp/)

int main(int argc, char** argv)
{
    job_queue = createQueue();
    running_job_queue = createQueue();
    jobExecutorServer();

    return 0;
}

char* readFromCommander(pid_t commander_pid)
{
    /* Open pipe to communicate with Commander */
    char* pipe_name = getPipeName(commander_pid, READ);
    int read_fd;
    if ((read_fd = open(pipe_name, O_RDONLY)) < 0)
    {
        perror("Server open pipe error");
        exit(1);
    }
    int n;
    char* msg = malloc(sizeof(char) * BUFFER);
    /* Make it dynamic reading */
    if ((n = read(read_fd, msg, BUFFER)) <= 0)
    {
        perror("Server read error");
        exit(1);
    }
    msg[n] = '\0';

    free(pipe_name);

    return msg;
}

void sendMessageToCommander(pid_t commander_pid, char* message)
{
    /* Signal commander that server is ready to send message */
    char* pipe_name = getPipeName(commander_pid, WRITE);    
    if ((mkfifo(pipe_name, PERM) < 0))
    {
        perror("[SERVER] Failed to create named pipe.");
        exit(1);
    }

    kill(commander_pid, SIGUSR1);
    
    /* Open pipe to write message to Commander */
    int write_fd;
    if ((write_fd = open(pipe_name, O_WRONLY)) < 0)
    {
        perror("[SERVER] Error on open pipe.");
        exit(1);
    }
    if (write(write_fd, message, strlen(message)) != strlen(message))
    {
        perror("[SERVER] Error on write.");
        exit(1);
    }

    free(pipe_name);
}

char* constructMessage(QueueNode node)
{
    char* pos = intToString(node->position);
    char* message = malloc(sizeof(char) * (strlen(node->job) + strlen(node->jobID) + strlen(pos)) + 10);
    strcpy(message, "<"); 
    strcat(message, node->jobID);
    strcat(message, ", ");
    strcat(message, node->job);
    strcat(message, ", ");
    strcat(message, pos);
    strcat(message, ">\n");

    return message;
}

void poll(Queue queue, pid_t pid, char* type)
{
    char* message;
    if (queue == NULL || queue->size == 0) 
    {
        message = malloc(strlen(type) + 26);
        strcpy(message, type);
        strcat(message, " Queue has no elements\n\n");
        sendMessageToCommander(pid, message);
    }
    else
    {
        char* individual_strings[queue->size];
        int i=0, length = 1;
        for (QueueNode node = queue->first; node != NULL; node = node->next)
        {
            individual_strings[i] = constructMessage(node);
            length += strlen(individual_strings[i]);
            i++;
        }

        message = malloc(sizeof(char) * (length + strlen(type)));
        strcpy(message, type);
        for (int j=0; j<queue->size; j++)
        {
            strcat(message, individual_strings[j]);
            free(individual_strings[j]);
        }

        sendMessageToCommander(pid, message);

        free(message);
    }
}

void stopJob(char* jobID, pid_t pid)
{
    QueueNode node;
    char* message;
    /* Search job in running queue - if found send signal to terminate */
    if ((node = queueFind(running_job_queue, jobID, -1)) != NULL)
    {
        /* if node-> pid == -1 then something went wrong */
        if (node->pid == -1)
        {
            message = malloc(strlen("Something went wrong (job pid = -1)\n") + 1);
            strcpy(message, "Something went wrong (job pid = -1)\n");
        }
        else
        {
            /* Signal process to terminate it */
            kill(node->pid, SIGTERM);

            /* Remove it from running queue */
            queueRemove(running_job_queue, jobID, -1);

            message = malloc(strlen(jobID) + strlen(" terminated\n") + 1);
            strcpy(message, jobID);
            strcat(message, " terminated\n");
        }
    }
    else if ((node = queueFind(job_queue, jobID, -1)) != NULL)
    {
        /* Remove it from queued jobs queue */
        queueRemove(job_queue, jobID, -1);

        message = malloc(strlen(jobID) + strlen(" removed\n") + 1);
        strcpy(message, jobID);
        strcat(message, " removed\n");
    }
    /* NOT FOUND */
    else
    {
        message = malloc(strlen(jobID) + strlen(" was not found in any queue!\n") + 1);
        strcpy(message, jobID);
        strcat(message, " was not found in any queue!\n");
    }

    sendMessageToCommander(pid, message);
}

void setConcurrency(char* rest)
{
    concurrency = atoi(rest);
    runJobs();
}

char** getExecArgs(char* job)
{
    /* Parse to get number of arguments by counting whitespace and newline characters */
    int argc = 1;
    for (int i=0; job[i] != '\0'; i++)
        if (job[i] == ' ' || job[i] == '\n')
            argc++;

    /* Now we have the number of arguments (i hope), so we parse the string again to get the args */
    char** args = malloc(sizeof(char*) * (argc + 1));
    int index = 0;
    char* token = strtok(job, " \n");
    while (token != NULL)
    {
        args[index] = strdup(token);
        if (args[index] == NULL)
        {
            perror("Error on strdup");
            exit(1);
        }
        index++;
        token = strtok(NULL, " \n");
    }
    args[index] = NULL;

    return args;
}

void runJobs()
{
    /* Now we check the concurrency to determine whether we can run more jobs */
    int currently_running = queueSize(running_job_queue);
    if (currently_running < concurrency)
    {
        for (int i=0; i<concurrency - currently_running; i++)
        {
            if (queueSize(job_queue) == 0) break;

            /* Get the first job that is queued */
            QueueNode node = queuePop(job_queue);

            /* Add it on running list and execute it */
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("Error on fork");
                exit(1);
            }
            if (pid == 0)
            {
                char** args = getExecArgs(node->job);
                if (execvp(args[0], args) == -1)
                {
                    perror("Execvp failed");
                    exit(-1);
                }
            }
            else
            {
                node->pid = pid;
                queueInsert(running_job_queue, node);
            }
        }
    }
}

void issueJob(char* rest, pid_t pid)
{
    /* job will be free'd on queueDestroy so i malloc */
    char* job = malloc(sizeof(char) * strlen(rest) + 1);
    strcpy(job, rest);

    char* jobId_num_str = intToString(jobID_number);
    char* jobId = malloc(sizeof(char) * (strlen(JOB) + strlen(jobId_num_str)) + 1);
    strcpy(jobId, JOB);
    strcat(jobId, jobId_num_str);

    /* Get triplet and add it on queue */
    QueueNode job_triplet = malloc(sizeof(*job_triplet));
    job_triplet->job = job;
    job_triplet->jobID = jobId;
    job_triplet->pid = -1; 

    /* Now to run the job we must check the concurrency. If we have less running jobs than 
    concurrency, we can run our job. */
    if (queueSize(running_job_queue) < concurrency)
    {
        int jobPid = fork();
        if (jobPid < 0)
        {
            perror("Error on fork");
            exit(1);
        }
        /* Create args and execute job */
        if (jobPid == 0)
        {
            char** args = getExecArgs(job);
            if (execvp(args[0], args) == -1)
            {
                perror("Execvp failed");
                exit(-1);
            }
        }
        /* Parent process - Add job to running queue (and remove it from queued jobs) */
        else
        {
            job_triplet->pid = jobPid;
            queueInsert(running_job_queue, job_triplet);
        }
    }
    /* else we can add it to the job_queue */
    else
    {
        queueInsert(job_queue, job_triplet);
    }

    char* message = constructMessage(job_triplet);
    sendMessageToCommander(pid, message);

    jobID_number++;

    free(jobId_num_str);
    free(message);
}

void identifyJob(char* cmd, pid_t pid)
{
    char* command = strtok(cmd, " "); 
    char* rest = strtok(NULL, "");
    if (strcmp(command, "issueJob") == 0)
    {
        issueJob(rest, pid);
    }
    else if (strcmp(command, "setConcurrency") == 0)
    {
        setConcurrency(rest);
    }
    else if (strcmp(command, "stop") == 0)
    {
        stopJob(rest, pid);
    }
    else if (strcmp(command, "poll") == 0)
    {
        if (strcmp(rest, "running") == 0)
        {
            poll(running_job_queue, pid, "\nRunning\n");
        }
        else if (strcmp(rest, "queued") == 0)
        {
            poll(job_queue, pid, "\nQueued\n");
        }
    }
    else if (strcmp(command, "exit") == 0)
    {
        /* Maybe wait for child processes to end ? - Nah */
        sendMessageToCommander(pid, "jobExecutorServer terminated\n");
        stop = 1;
    }
    else
    {
        sendMessageToCommander(pid, "Invalid command\n");
    }
    
    free(cmd);
}

void signal_handler(int signum, siginfo_t* info, void* context)
{
    char* cmd;
    pid_t pid;
    int status;
    switch (signum)
    {
        /* Commander want to communicate */
        case SIGUSR1:
            cmd = readFromCommander(info->si_pid);
            identifyJob(cmd, info->si_pid);
            break;
        case SIGUSR2:
            commander_ready = 1;
            break;
        /* Signal to terminate server - just for user to stop the server without ./jobCommander exit */
        case SIGINT:
            stop = 1;
            break;
        case SIGCHLD:
            /* Get terminated jobs and remove them from queue */
            while((pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0)
            {
                /* Remove job */
                queueRemove(running_job_queue, NULL, pid);
            }
            runJobs();
        default:
            break;
    }
}

void jobExecutorServer()
{
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    createServerFile("[Server]");

    while (!stop);

    unlink(SERVER_FILE_PATH);

    queueDestroy(job_queue);
    /* QueueNodes are deleted from previous queue destroy as i pass the same pointer */
    free(running_job_queue); 
}