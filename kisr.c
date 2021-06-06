/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Kernel Interrupt Service Routines
 */

#include "spede.h"
#include "kernel.h"
#include "kisr.h"
#include "kproc.h"
#include "queue.h"
#include "string.h"
#include "ksyscall.h"

/**
 * Kernel Interrupt Service Routine: Timer (IRQ 0)
 */
void kisr_timer() {
	
    //Variables for wake and processID
    int wakeProcess, i = 0;
    system_time++;                                                        // Increment the system time

    // If the running PID is invalid, just return
    if (run_pid == -1) {

        // Dismiss IRQ 0 (Timer)
        outportb(0x20, 0x60);
        return;

    }
	
	//CHECK THIS FUNCTION CONDITION - changed acc. to demo code
    //determine what processes should be woken up from the sleep_q
    while(i < sleep_q.size){

        if(dequeue(&sleep_q, &wakeProcess) != 0){

            panic("Error! Unable to get process from the sleep_q");

        } 
        if(pcb[wakeProcess].wake_time <= system_time){
            if(enqueue(pcb[wakeProcess].queue, wakeProcess) != 0)
                panic("Error! Unable to add process to the run_q");
            pcb[wakeProcess].state = READY;
        }
        else
            if(enqueue(&sleep_q, wakeProcess) != 0)
                panic("Error! Unable to add process to the sleep_q");
        
        i++;

    }

    pcb[run_pid].time++;                                                // Increment the running process' current run time

    // Once the running process has exceeded the maximum number of ticks, it needs to be unscheduled:
    if (pcb[run_pid].time >= PROC_TICKS_MAX) {

        pcb[run_pid].total_time += pcb[run_pid].time;                   // set the total run time
        pcb[run_pid].time  = 0;                                         // reset the current running time
        pcb[run_pid].state = READY;                                     // set the state to ready
        enqueue(pcb[run_pid].queue, run_pid);                           // queue the process back into the runnning queue
        run_pid = -1;                                                   // clear the running pid

    }

    outportb(0x20, 0x60);                                               // Dismiss IRQ 0 (Timer)

}

void kisr_syscall(){
	
    if(run_pid < 0 || run_pid > PID_MAX)
        panic("Invalid PID");

    //Error here (fixed) - changed from syscall to ksyscall********************************************
    //Determine the type of system call performed and then call appropriate functions
    if(pcb[run_pid].trapframe_p -> eax == SYSCALL_GET_PROC_PID)
        ksyscall_get_proc_pid();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_GET_PROC_NAME)
        ksyscall_get_proc_name();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_GET_SYS_TIME)
        ksyscall_get_sys_time();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_PROC_EXIT)
        kproc_exit();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_SLEEP)
        ksyscall_sleep();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_SEM_INIT)
        ksyscall_sem_init();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_SEM_POST)
        ksyscall_sem_post();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_SEM_WAIT)
        ksyscall_sem_wait();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_MSG_RECV)
        ksyscall_msg_recv();
    else if(pcb[run_pid].trapframe_p -> eax == SYSCALL_MSG_SEND)
        ksyscall_msg_send();
    else
        panic("Invalid syscall");    
	
}
