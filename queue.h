/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Queue Utilities
 */
#ifndef QUEUE_H
#define QUEUE_H

#include "global.h"

#define QUEUE_SIZE PROC_MAX

// Queue data structure
typedef struct {
    int items[QUEUE_SIZE];  // queue items
    int head;               // queue head
    int tail;               // queue tail
    int size;               // queue size
} queue_t;

/**
 * Function declarations
 */
int enqueue(queue_t *queue, int item);
int dequeue(queue_t *queue, int *item);
int initializeQueue(queue_t *queue);
#endif
