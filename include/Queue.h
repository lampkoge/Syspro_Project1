#include <stdio.h>
#include <stdlib.h>

typedef struct queue_node* QueueNode;

struct queue_node{
    char* jobID;
    char* job;
    int position;
    int pid; // Will be used for running queue
    QueueNode next;
};

struct queue{
    int size;
    QueueNode first;
    QueueNode last;
};

typedef struct queue* Queue;

Queue createQueue();

int queueSize(Queue queue);

void queueInsert(Queue queue, QueueNode node);

void queueRemove(Queue queue, char* jobID, int pid);

void updatePositions(Queue queue, QueueNode to_be_removed);

QueueNode queueFind(Queue queue, char* jobID, int pid);

void queueDestroy(Queue queue);

QueueNode queuePop(Queue queue);

void queuePrint(Queue queue);