Lamprou Konstantinos-Georgios
sdi1900097




# About code

Compile : make all
Clean : make clean (also removes the jobExecutorServer.txt)

1) The communication between the commander and the server is done via named pipes and 
signals. 
Meaning of signals received by the server :
- SIGUSR1 : the commander has opened a named pipe and is ready to send message
- SIGCHLD : triggered when a child process (job) terminates, remove it from the running queue
and if the concurrency allows it, run the next job.

2) The names of each pipe follow this standard : 
Always begin with "fifo." succeeded by either "0." or "1.", depending on whether the process 
intends to read or write (if one process reads, the other writes so the 0-1 is inversed). Finally we 
add the process id of the COMMANDER. For example, if the commander (pid: 12345) wants to read from 
the server, it opens the named pipe with name "fifo.0.12345".

3) Every pipe is created on the tmp/ folder.

## jobCommander

main.c : Every time a jobCommander is executed the main extracts the 
command given and passes it to the jobCommander function.

The jobCommander checks if the jobExecutorServer.txt file exists. If it 
does, it opens the file and reads the server pid, otherwise it forks and exec's 
the server executable.
After getting the server pid, it opens a named pipe to pass the given command and 
signals the server that it is ready to write.
After passing the message to the server,  the jobCommander gets a reply from the 
server on each command except "setConcurrency".
- If the passed command is "setConcurrency" it unlinks the pipe and shuts down
- If not, it waits for a signal that the server is ready to respond, and opens 
a named pipe for read. It then prints the server response, unlinks every
named pipe created (for this commander) and terminates.

## jobExecutorServer

Creates two queues, one for the running jobs and the other for the queued ones.
It creates the jobExecutorServer.txt file and then waits for a signal (basically that 
a commander is ready to send a command). 
When the signal SIGUSR1 is received, the server opens a named pipe to read the message
from the commander. This message is then passed in to a function that determines 
which function (job) to run.

- issueJob : inserts jobs to queues. Based on the current concurrency and the size 
of the currently running jobs, it determines whether the job received will execute
now (therefore fork and place it in to the appropriate queue), or insert the job
in to the queued jobs queue. Returns the triplet to the commander.

- setConcurrency : changes the value of the current concurrency to the argument passed
If there are queued jobs and the concurrency allows it, they are executed. Currently 
running jobs will not be terminated. Returns nothing to the commander.

- stopJob : sends a SIGTERM signal if the job specified is running and removes it from
the running queue, removes it from the queued jobs otherwise. Returns the appropriate
message to the commander.

- poll : returns the triplets of each element of the specified queue (running, 
queued) to the commander. If there are no elements it returns appropriate message.

- exit : terminates the jobExecutorServer process. Returns message to commander.

## Queue

This is my implementation of a queue specifically designed for this 
project. Every QueueNode has the triplet we need for the jobs and the 
pid of each job (if it is a running job - otherwise -1).

## myFunctions

Functions that are used in common by both processes such us intToString and 
getPipeName.

Everything else about the functions and the code is detailed within the comments.

# Tests

## jobCommander - jobExecutorServer

For testing i ran the commands on each test*.txt. 

## multijob.sh

There are 2 files to test this script. The commands i ran my tests with are :
1)  make all
    ./jobCommander setConcurrency 5
    ./multijob.sh files/commands_1.txt
    ./multijob.sh files/commands_2.txt
    ./jobCommander exit

## allJobsStop.sh

1)  ./multijob.sh files/commands_3.txt
    ./jobCommander issueJob setConcurrency 4
    ./multijob.sh files/commands_4.txt
    ./allJobsStop.sh
    ./jobCommander poll running
    ./jobCommander poll queued
    ps -aux | grep ./progDelay

For testing purposes i compiled the given progDelay program and added the 
executable on the project folder. The files commands_3 and commands_4 
may need to be altered (if you run them).    