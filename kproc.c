/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Kernel Process Handling
 */
#include "spede.h"
#include "kernel.h"
#include "kproc.h"
#include "queue.h"
#include "string.h"

//Process scheduler
void kproc_schedule() {

    //If active process, return
    if (run_pid >= 0) {
        return;
    }

    if (dequeue(&run_q, &run_pid) == 0) {                           // Retrieve a process from the run queue
        pcb[run_pid].state = RUNNING;
    } 
	else if(dequeue(&idle_q, &run_pid) == 0){                       // Retrieve a process from the idle queue
		pcb[run_pid].state = RUNNING;
	}
	else {
        panic("No tasks scheduled to run");                         // no process to run
    }

    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID");                                       // invalid process or PID
    }

}

/**
 * Start a new process
 * @param proc_name The process title
 * @param proc_ptr  function pointer for the process
 * @param queue     the run queue in which this process belongs
 */
void kproc_exec(char *proc_name, void *proc_ptr, queue_t *queue) {

    int pid;
    // Ensure that valid parameters have been specified
    if (proc_name == NULL) 
        panic("Invalid process title");
    if (proc_ptr == NULL) 
        panic("Invalid function pointer");
    if (queue == NULL) 
        panic("Invalid queue specified");

    // Dequeue the process from the available queue
    if (dequeue(&available_q, &pid) != 0) {
        panic_warn("Unable to retrieve process from available queue");
        return;
    }

    sp_memset(&pcb[pid], 0, sizeof(pcb_t));                                 // Initialize the PCB

    pcb[pid].state = READY;                                                 // Set the process state to READY
    pcb[pid].time = 0;                                                      // initializing other values to default values
    pcb[pid].total_time = 0;

    sp_strncpy(pcb[pid].name, proc_name, PROC_NAME_LEN);                    // Copy the process name to the PCB
    sp_memset(stack[pid], 0, sizeof(stack[pid]));                           // Ensure the stack for the process is cleared

    // Allocate the trapframe data
    pcb[pid].trapframe_p = (trapframe_t *)&stack[pid][PROC_STACK_SIZE - sizeof(trapframe_t)];

    // Set the instruction pointer in the trapframe
    pcb[pid].trapframe_p->eip = (unsigned int)proc_ptr;

    // Set INTR flag
    pcb[pid].trapframe_p->eflags = EF_DEFAULT_VALUE | EF_INTR;

    // Set each segment in the trapframe
    pcb[pid].trapframe_p->cs = get_cs();
    pcb[pid].trapframe_p->ds = get_ds();
    pcb[pid].trapframe_p->es = get_es();
    pcb[pid].trapframe_p->fs = get_fs();
    pcb[pid].trapframe_p->gs = get_gs();

    // Set the process run queue
    pcb[pid].queue = queue;

    // Move the proces into the associated run queue
    enqueue(pcb[pid].queue, pid);

    printf("Started process %s (pid=%d)\n", pcb[pid].name, pid);

}

/**
 * Exit the currently running process
 */
void kproc_exit() {

    //printf("\n\nInside kproc_exit() function\n\n");
    // if idle task, don't exit
    //printf("RUN_PID in exit function is: %d    and     PID in exit function is: %d \n\n\n", run_pid, pid);

    if (run_pid == 0) {
        //printf("Idle process ...\n");
        return;
    }
    
    // Panic if we have an invalid PID
    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID");
    }

    //printf("Right before print statements \n\n");
    //cons_printf("Exiting process %s (pid=%d)\n", pcb[run_pid].name, run_pid);
    printf("Exiting process %s (pid=%d)\n", pcb[run_pid].name, run_pid);
    cons_printf("Process Exited\n");                                // Indicate that the process has exited, on the target
    
    pcb[run_pid].state = AVAILABLE;                                 // Change the state of the running process to AVAILABLE
    enqueue(&available_q, run_pid);                                 // Queue it back to the available queue
    run_pid = -1;                                                   // clear the running pid
    kproc_schedule();                                               // Trigger the scheduler to load the next process

}

/**
 * Kernel idle task
 */
void ktask_idle() {
    
    cons_printf("Idle_task started\n");                             // Indicate that the Idle Task has started, on the target
    // Process run loop
    while (1) {
        asm("hlt");
    }
}
