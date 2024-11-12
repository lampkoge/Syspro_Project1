#pragma once

#include "Queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <signal.h>

void jobExecutorServer();

/* Opens named pipe using commanders pid and reads given message. */
char* readFromCommander(pid_t commander_pid);

/* Creates a named pipe using commanders pid and signals it that server is ready 
to send a message. Then writes the message in pipe. */
void sendMessageToCommander(pid_t commander_pid, char* message);

/* Constructs give node's string */
char* constructMessage(QueueNode node);

/* Given a queue, gets the triplets of every element and sends them to commander for print.
If queue size = 0, it sends the commander an appropriate message. */
void poll(Queue queue, pid_t pid, char* type);

/* Given the jobID, it searches running jobs queue and queued jobs queue to find it. 
If job is running, it sends a signal to terminate it and removes it from running queue.
If job is queued, it removes it from queued jobs queue. If job was not found it sends 
an appropriate message to commander. */
void stopJob(char* jobID, pid_t pid);

/* Changes concurrency to given number - then calls runJobs to check whether we 
can run more jobs. If more jobs than concurrency are already running it does NOT stop them */
void setConcurrency(char* rest);

/* Gets job string as input and returns an array of strings, for the arguments 
to be used in exec later */
char** getExecArgs(char* job);

/* Checks whether we can run jobs based on concurrency. If there are queued jobs,
 and concurrency allows it, we pop the first job and run it. 
 Called when concurrency is changed or when a job is terminated.  */
void runJobs();

/* Executes or adds job to queue based on concurrency. Sends message to commander */
void issueJob(char* rest, pid_t pid);

/* Extracts job from command and calls the appropriate function */
void identifyJob(char* cmd, pid_t pid);

/* Handler for signals SIGUSR1 and SIGINT */
void signal_handler(int signum, siginfo_t* info, void* context);