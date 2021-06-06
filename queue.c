/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Queue Utilities
 */

#include "queue.h"
#include "spede.h"
#include "string.h"

/**
 * Adds an item to the end of a queue
 * @param  queue - pointer to the queue
 * @param  item  - the item to add
 * @return -1 on error; 0 on success
 */
int enqueue(queue_t *queue, int item) {

    //Doing a null pointer check - return -1 if null
    if(queue == NULL)
        return -1;

    // Return an error (-1) if the queue is full
    if(queue -> size == QUEUE_SIZE){
        //printf("Enqueue error");
        return -1;
    }

    queue -> items[queue -> tail] = item;                   // Add the item to the tail of the queue
    queue -> tail ++;                                       // Move the tail forward
    
    // If we are at the end of the array, move the tail to the beginning
    if(queue -> tail == QUEUE_SIZE)
        queue -> tail = 0;

    queue -> size++; 
    //printf("%d", queue -> size);
    //printf("enqueue is working\n");                                       // Increment size (since we just added an item to the queue)

    //Return 0 since the process was successful
    return 0;
    
}

/**
 * Pulls an item from the specified queue
 * @param  queue - pointer to the queue
 * @return -1 on error; 0 on success
 */
int dequeue(queue_t *queue, int *item) {

    // Return an error (-1) if queue is empty
    if(queue -> size == 0){
        //printf("size error\n");
        return -1;
    }

    //Null reference checking
    if(!queue)
        return -1;

    //Possible problem here VVV - does item need * or & to store???
    *item = queue -> items[queue -> head];                   // Get the item from the head of the queue
    queue -> head++;                                        // Move the head forward
    
    // If we are at the end of the array, move the head to the beginning
    if(queue -> head == QUEUE_SIZE)
        queue -> head = 0;
    
    queue -> size--;                                        // Decrement size (since we just removed an item from the queue)

    //Return 0 since the process was successful
    return 0;
}

/**
 * Initializes the specified queue with the given item
 * @param  queue - pointer to the queue
 * @return 0 if successful, else return 1
 */
int initializeQueue(queue_t *queue){

    //Declr. & initialz. counter var.
    int i = 0;

    if(!queue){

        return 1;

    }

    //Set the head, tail and size variable to zero for initialization
    queue -> size = 0;
    queue -> head = 0;
    queue -> tail = 0;

    //Initializing the queue -> items arr.
    for(i = 0; i < QUEUE_SIZE; i++)
        queue -> items[i] = -1;
    
    //Added type cast to char * based on demo soln.
    sp_memset(queue, 0, sizeof(queue_t)); //updated-Oct31

    return 0;

}
