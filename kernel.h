/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Internal Kernel APIs and Data structures
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "global.h"
#include "queue.h"
#include "trapframe.h"
#include "ipc.h"

// Global Definitions******************

#define PID_MAX PROC_MAX-1                                      // Maximum process ID possible (0-based PIDs)
#define PROC_STACK_SIZE 8196                                    // Process runtime stack size
#define PROC_TICKS_MAX 50                                       // Maximum number of ticks a process may run before being rescheduled
#define SEMAPHORE_MAX PROC_MAX                                  // Maximum number of semaphores
#define MBOX_MAX PROC_MAX                                       // Maximum number of mailboxes
#define MBOX_SIZE PROC_MAX                                      // Size of each mailboxes

/**
 * Kernel data types and definitions
 */

// Process states
typedef enum { AVAILABLE, READY, RUNNING, SLEEPING, WAITING} state_t;

// The process control block for each process
typedef struct {
    char name[PROC_NAME_LEN+1];     // Process name/title
    state_t state;                  // current process state
    queue_t *queue;                 // queue the process belongs to
    int time;                       // run time since loaded
    int total_time;                 // total run time since created
    trapframe_t *trapframe_p;       // process trapframe
	int wake_time;					// time when proc. is done sleeping
} pcb_t;

//Syscall definitions
typedef enum{
	SYSCALL_PROC_EXIT,
	SYSCALL_GET_SYS_TIME,
	SYSCALL_GET_PROC_PID,
	SYSCALL_GET_PROC_NAME,
	SYSCALL_SLEEP,
    SYSCALL_SEM_INIT,
    SYSCALL_SEM_WAIT,
    SYSCALL_SEM_POST,
    SYSCALL_MSG_SEND,
    SYSCALL_MSG_RECV
}syscall_t;

// Semaphore data structure
typedef struct {
    int count;                      // Semaphore count
    int init;                       // Indicates if initialized
    queue_t wait_q;                 // Wait queue for the semaphore
} semaphore_t;

// Mailbox data structures
typedef struct {
    msg_t messages[MBOX_SIZE]; // Incoming messages
    int head;                       // First message
    int tail;                       // Last message
    int size;                       // Total messages
    queue_t wait_q;                 // Processes waiting for messages
} mailbox_t;

/**
 * Kernel data structures - available to the entire kernel
 */

extern char stack[PROC_MAX][PROC_STACK_SIZE];                   // runtime stacks of processes
extern pcb_t pcb[PROC_MAX];                                     // process table
extern int system_time;                                         // System time
extern int run_pid;                                             // ID of running process, -1 means not set
extern semaphore_t semaphores[SEMAPHORE_MAX];                   // Semaphore DT
extern mailbox_t mailboxes[MBOX_MAX];                           // mailbox DT

// *****Queues*****
extern queue_t available_q, run_q, idle_q, sleep_q;             // Process queues
extern queue_t semaphore_q;                                     // Semaphore Queue


/**
 * Function declarations
 */

/**
 * Triggers a kernel panic that does the following:
 *   - Displays a panic message
 *   - Triggers a breakpiont (if running through GDB)
 *   - aborts/exits
 * @param msg   message to be displayed
 */
void panic(char *msg);

/**
 * Triggers a kernel panic that does the following:
 *   - Displays a warning message
 *   - Triggers a breakpoint (if running through GDB)
 * @param msg   message to be displayed
 */
void panic_warn(char *msg);


/**
 * Prints to host console if DEBUG flag is enabled
 * @param format    string format to be used for printing
 * @param ...       variable arguments for format string
 */
void debug_printf(char *format, ...);

#endif
