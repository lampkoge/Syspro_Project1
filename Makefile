OBJS = main.o source/jobCommander.o source/myFunctions.o source/Queue.o
OBJS2 = source/jobExecutorServer.o source/myFunctions.o source/Queue.o
SOURCE = main.c source/jobCommander.c source/myFunctions.c source/Queue.c
SOURCE2 = source/jobExecutorServer.c source/myFunctions.c source/Queue.c
HEADER = include/myFunctions.h include/jobCommander.h include/jobExecutorServer.h include/Queue.h
OUT = jobCommander
OUT2 = jobExecutorServer
CC = gcc
FLAGS = -g -c
INCLUDES = -Iinclude

all: $(OUT) $(OUT2)

$(OUT): $(OBJS)
	$(CC) -g $(OBJS) -o $@ 

$(OUT2): $(OBJS2)
	$(CC) -g $(OBJS2) -o $@

main.o: main.c
	$(CC) $(FLAGS) $(INCLUDES) main.c -o $@

source/Queue.o: source/Queue.c
	$(CC) $(FLAGS) $(INCLUDES) source/Queue.c -o $@

source/jobCommander.o: source/jobCommander.c
	$(CC) $(FLAGS) $(INCLUDES) source/jobCommander.c -o $@

source/jobExecutorServer.o: source/jobExecutorServer.c
	$(CC) $(FLAGS) $(INCLUDES) source/jobExecutorServer.c -o $@

source/myFunctions.o: source/myFunctions.c
	$(CC) $(FLAGS) $(INCLUDES) source/myFunctions.c -o $@

clean:
	rm -f $(OBJS) $(OUT)
	rm -f $(OBJS2) $(OUT2)
	rm -f jobExecutorServer.txt