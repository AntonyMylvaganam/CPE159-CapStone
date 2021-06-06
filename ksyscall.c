/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * System call APIs - Kernel Side
 */
#include "spede.h"
#include "kernel.h"
#include "kproc.h"
#include "string.h"
#include "queue.h"
#include "ksyscall.h"
#include "ipc.h"

// Foward Declarations
int mbox_enqueue(msg_t *msg, int mbox_num);
int mbox_dequeue(msg_t *msg, int mbox_num);

/**
 * System call kernel handler: get_sys_time
 * Returns the current system time (in seconds)
 */
void ksyscall_get_sys_time() {
    
    // Don't do anything if the running PID is invalid
    if(run_pid < 0 || run_pid > PID_MAX)
        panic("Invalid PID");

    // Copy the system time from the kernel to the eax register via the running process' trapframe
    pcb[run_pid].trapframe_p -> ebx = system_time / CLK_TCK;        //Taken from demo code

}

/**
 * System call kernel handler: get_proc_id
 * Returns the currently running process ID
 */
void ksyscall_get_proc_pid() {

    // Don't do anything if the running PID is invalid
    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID");
    }

    // Copy the running pid from the kernel to the ebx register via the running process' trapframe
    pcb[run_pid].trapframe_p->ebx = run_pid;

}

/**
 * System call kernel handler: get_proc_name
 * Returns the currently running process name
 */
void ksyscall_get_proc_name() {

    // Don't do anything if the running PID is invalid
    if(run_pid < 0 || run_pid > PID_MAX)
        panic("Invalid PID");

    // Set the destination pointer to the address passed in via EBX Copy the string name from the PCB to the destination
    sp_strcpy((char *)pcb[run_pid].trapframe_p->ebx, pcb[run_pid].name);

}

/**
 * System call kernel handler: sleep
 * Puts the currently running process to sleep
 */
void ksyscall_sleep() {

    // Don't do anything if the running PID is invalid
    if(run_pid < 0 || run_pid > PID_MAX)
        panic("Invalid PID");

    // Calculate the wake time for the currently running process and store it in pcb's wakeTime member
    pcb[run_pid].wake_time = system_time + CLK_TCK * pcb[run_pid].trapframe_p->ebx;

    // Move the currently running process to the sleep queue
    //pcb[run_pid].queue = &sleep_q;
    enqueue(&sleep_q, run_pid);

    // Change the running process state to SLEEP
    pcb[run_pid].state = SLEEPING;

    // Clear the running PID so the process scheduler will run
    run_pid = -1;

}

// Function to initialize the semaphores
void ksyscall_sem_init()
{
	int sem_num;
	int *sem_ptr;
	
	if (run_pid < 0 || run_pid > PID_MAX)
	{
		panic("Invalid PID");
	}
	
	// obtain the passed semaphore_id fromt he trapframe
	sem_ptr = (int *)pcb[run_pid].trapframe_p->ebx;

	// If the call to sem_init is on initialized semaphore, then ensure that count is initialized to 0
	if(*sem_ptr != SEMAPHORE_UNINITIALIZED && semaphores[*sem_ptr].init == SEMAPHORE_INITIALIZED){

		semaphores[*sem_ptr].count = 0;

	}
	else if (*sem_ptr == SEMAPHORE_UNINITIALIZED)
	{
		if (dequeue(&semaphore_q, &sem_num) != 0)
		{
			panic("Invalid Semaphore");
		}
		*sem_ptr = sem_num;
		semaphores[*sem_ptr].count = 0;
		semaphores[*sem_ptr].init = SEMAPHORE_INITIALIZED;
	}

}

// Function to tell the semaphore to wait
void ksyscall_sem_wait()
{
	int *sem_num;
	
	if (run_pid < 0 || run_pid > PID_MAX)
	{
		panic("Invalid PID");
	}
	
	// Obtain the passed semaphore from the trapframe
	sem_num = (int *)pcb[run_pid].trapframe_p->ebx;
	//sem_ptr = (int *)pcb[run_pid].trapframe_p->ebx;
	
	// check if the semaphore is valid or not
	if (*sem_num < -1 || *sem_num > SEMAPHORE_MAX)
	{
		panic("Invalid Semaphore");
	}
	
	// if the semaphore count > 0, then ther eis atleast one process in the wait queue
	if (semaphores[*sem_num].count > 0)
	{
		if (enqueue(&semaphores[*sem_num].wait_q, run_pid) != 0)
		{
			panic("Cannot Nq process");
		}
		pcb[run_pid].state = WAITING;
		//run_pid = -1;

	}
	
	semaphores[*sem_num].count++;
}

// Function to tell the semaphore to post
void ksyscall_sem_post()
{
	int *sem_num;
	int pid = -1;
	
	if (run_pid < 0 || run_pid > PID_MAX)
	{
		panic("Invalid PID");
	}
	
	// obtain the semaphore from the trapframe
	sem_num = (int *)pcb[run_pid].trapframe_p->ebx;
	
	// check if the semaphore is valid or not
	if (*sem_num < 0 || *sem_num > SEMAPHORE_MAX)
	{
		panic("Invalid Semaphore");
	}

	// check if the semaphore has a process in waiting
	if (semaphores[*sem_num].wait_q.size > 0){

		if (dequeue(&(semaphores[*sem_num].wait_q), &pid) != 0){
			panic("Cannot dq process");
		}
		pcb[pid].state = READY;
		enqueue(&run_q, pid);

	}
	if(semaphores[*sem_num].count > 0)
		semaphores[*sem_num].count--;
	/*
	// how to check if hte process is in waiting?
	if (semaphores[sem_num].wait_q.size > 0)
	{
		if (dequeue(&semaphores[sem_num].wait_q, &pid) != 0)
		{
			panic("Cannot Dq process");
		}
		
		if (enqueue((pcb[pid].queue), pid) != 0)
		{
			panic("Cannot Nq process");
		}
		pcb[pid].state = READY;
	}
	
	if (semaphores[sem_num].count > 0)
	{
		semaphores[sem_num].count--;
	}*/
}

// Function to send the message
void ksyscall_msg_send()
{
	int mbox_num;
	int waiting_pid = -1;
	msg_t *msg_sender = NULL;
	msg_t *msg_reciever = NULL;
	
	if (run_pid < 0 || run_pid > PID_MAX)
	{
		panic("Invalid PID");
	}
	
	// retrieve the data from the kennel
	msg_sender = (msg_t *)pcb[run_pid].trapframe_p->ebx;
	mbox_num = pcb[run_pid].trapframe_p->ecx;
	
	// check if the message sender is valid
	if(msg_sender == NULL)
	{
		panic("Invalid mailbox pointer");
	}
	
	// check if hte message box is valid
	if (mbox_num < 0 || mbox_num > MBOX_MAX)
	{
		panic("Invalid mailbox indentifier");
	}
	
	// 
	if (mbox_enqueue(msg_sender, mbox_num) != 0)
	{
		panic("Cannot Send Message");
	}
	
	// if the mailbox has a process in it...
	if (mailboxes[mbox_num].wait_q.size > 0)
	{
		if (dequeue(&(mailboxes[mbox_num].wait_q), &waiting_pid) != 0)
		{
			panic("Cannot Dq waiting PID");
		}
		
		if (enqueue(&run_q, waiting_pid) != 0)
		{
			panic("Cannot Nq waiting PID");
		}
		pcb[waiting_pid].state = READY;
		msg_reciever = (msg_t *)pcb[waiting_pid].trapframe_p->ebx;
		mbox_dequeue(msg_reciever, mbox_num);
	}
}

// Function to receive the message
void ksyscall_msg_recv()
{
	int mbox_num;
	msg_t *msg_reciever = NULL;
	
	// check if hte run_pid is valid
	if (run_pid < 0 || run_pid > PID_MAX)
	{
		panic("Invalid PID");
	}
	
	msg_reciever = (msg_t *)pcb[run_pid].trapframe_p->ebx;
	mbox_num = pcb[run_pid].trapframe_p->ecx;
	
	if (msg_reciever == NULL)
	{
		panic("Invalid mailbox pointer");
	}
	
	if (mbox_num < 0 || mbox_num > MBOX_MAX)
	{
		panic("Invalid mailbox indentifier");
	}
	
	// if the mailbox has a message...
	if (mailboxes[mbox_num].size > 0)
	{
		if (mbox_dequeue(msg_reciever, mbox_num) != 0)
		{
			panic("No Message to Dq");
		}
	}
	else
	{
		if (enqueue(&(mailboxes[mbox_num].wait_q), run_pid) != 0)
		{
			panic("No Message to Nq");
		}
		pcb[run_pid].state = WAITING;
		run_pid = -1;
	}
		
}

// Helper function for the enqueuing the messages for a given mailbox
int mbox_enqueue(msg_t *msg, int mbox_num)
{
	mailbox_t *mb;
	if (msg == NULL)
	{
		panic("Message: INVALID"); // error checking
	}

	if (mbox_num < 0 || mbox_num > MBOX_MAX)
	{
		panic("Mailbox ID: INVALID"); // error checking
	}
	mb = &mailboxes[mbox_num]; // easy pointer. DOUBLE CHECK

	if (mb->size == MBOX_SIZE)
	{
		return -1; // if full
	}
	
	msg->sender = run_pid;  // get the receiving process ID
	msg->time_sent = system_time / CLK_TCK; // new calculation?
	sp_memcpy(&mb->messages[mb->tail], msg, sizeof(msg_t)); //From process to mailbox
	mb->tail++;
	
	if (mb->tail == MBOX_SIZE)
	{
		mb->tail = 0; // loop the queue
	}
	mb->size++;
	return 0; // it worked
}

// Helper function for the dequeuing for the messages for a given mailbox
int mbox_dequeue(msg_t *msg, int mbox_num)
{
	mailbox_t *mb;
	if (msg == NULL)
	{
		panic("Message: INVALID"); // error checking
	}

	if (mbox_num < 0 || mbox_num > MBOX_MAX)
	{
		panic("Mailbox ID: INVALID"); // error checking
	}
	
	mb = &mailboxes[mbox_num]; // easy pointer. DOUBLE CHECK

	if (mb->size == 0)
	{
		return -1; // if empty
	}
	
	sp_memcpy(msg, &mb->messages[mb->head], sizeof(msg_t)); // From mailbox to process
	mb->head++;
	
	if (mb->head == MBOX_SIZE)
	{
		mb->head = 0; // loop the queue 
	}
	
	mb->size--; 			// reduce size of Q
	msg->time_received = system_time / CLK_TCK;	//system time / clock tick
	return 0; // it worked
}

