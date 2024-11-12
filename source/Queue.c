#include "../include/Queue.h"

#include <string.h>

Queue createQueue(){
    Queue queue = malloc(sizeof(*queue));
    queue->size = 0;
    queue->first = NULL;
    queue->last = NULL;
}

int queueSize(Queue queue){
    return queue->size;
}

/* Fifo - node is malloc'd outside */
void queueInsert(Queue queue, QueueNode node){
    if (queue->first == NULL)
        queue->first = node;
    else
        queue->last->next = node;

    node->position = queue->size;

    node->next = NULL;
    queue->last = node;

    queue->size++;
}

/* Gets the first node from queue (and removes it) */
QueueNode queuePop(Queue queue)
{
    if (queue->first == NULL) return NULL;
    QueueNode node = queue->first;
    updatePositions(queue, node);
    queue->size--;
    return node;
}

/* Remove node with given jobID (or pid for running processes) - then update positions */
void queueRemove(Queue queue, char* jobID, int pid){
    QueueNode node = queueFind(queue, jobID, pid);
    
    if (node == NULL) return;

    updatePositions(queue, node);
    
    free(node);

    queue->size--;
}

void updatePositions(Queue queue, QueueNode to_be_removed){
    if (queue->first == to_be_removed)
        queue->first = to_be_removed->next;
    int counter = 0;
    for (QueueNode node = queue->first; node!=NULL; node = node->next){
        if (node->next == to_be_removed){
            node->next = to_be_removed->next;
            if (queue->last == to_be_removed){
                queue->last = node;                
            }
        }
        if (node == to_be_removed) continue;
        node->position = counter++; 
    }
}

QueueNode queueFind(Queue queue, char* jobID, int pid){
    for (QueueNode node = queue->first; node != NULL; node = node->next){
        if (pid == -1 && jobID != NULL)
        {
            if (strcmp(node->jobID,jobID) == 0)
                return node;
        }
        else 
        {
            if (node->pid == pid)
                return node;
        }
    }
    return NULL;
}
/* For debugging purposes */
void queuePrint(Queue queue)
{
    if (queue == NULL || queue->size == 0) 
    {
        printf("\n\tQueue has no elements\n");
    }
    else
    {
        printf("\n");
        for (QueueNode node = queue->first; node != NULL; node = node->next)
        {
            printf("\t <jobID: %s, job: %s, pos: %d, pid: %d> \n", node->jobID, node->job, node->position, node->pid);
        }
        printf("\n");
    }
}

void queueDestroy(Queue queue){
    if (queue->size == 0 || queue->first == NULL)
        return;

    QueueNode node = queue->first;
    while(node != NULL){
        QueueNode next = node->next;

        free(node->jobID);
        free(node->job);
        free(node);
        node = next;
    }
    free(queue);
}