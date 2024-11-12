#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_FILE_NAME "jobExecutorServer.txt"
#define SERVER_FILE_PATH "jobExecutorServer.txt"
#define SERVER_EXE "./jobExecutorServer"
#define FIFO "/tmp/fifo." 

#define PERM 0777

char* extractCommand(int argc, char** argv);

char* intToString(int num);

char* getPipeName(int pid, int mode);

void createServerFile(char* origin);