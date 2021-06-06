/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Main Kernel Entry point
 */

#include "spede.h"
#include "global.h"
#include "kernel.h"
#include "kisr.h"
#include "kproc.h"
#include "queue.h"
#include "string.h"
#include "user_proc.h"

// Local function definitions
void kdata_init();
void idt_init();
void idt_entry_add(int entry_num, func_ptr_t func_ptr);

/**
 * Kernel data structures
 */

int system_time, run_pid;                               // Current system time and current running process ID
queue_t available_q, run_q ,idle_q, sleep_q;            // Process queues
pcb_t pcb[PROC_MAX];  									// Process table

// Semaphores
semaphore_t semaphores[SEMAPHORE_MAX];
queue_t semaphore_q;

// Mailboxes
mailbox_t mailboxes[MBOX_MAX];   

char stack[PROC_MAX][PROC_STACK_SIZE];                  // runtime stacks of processes
struct i386_gate *idt_p;								// Interrupt descriptor table

/**
 * Main entry point for our kernel/operating system
 * This function initializes all kernel data and hardware then begins
 * process scheduling and operations.
 */
int main() {

    kdata_init();                                       // Initialize kernel data structures
    idt_init();                                         // Initialize the IDT
    kproc_exec("ktask_idle", &ktask_idle, &idle_q);                         // Launch the kernel idle task
    kproc_exec("dispatcher_proc", &dispatcher_proc, &run_q);         // Launch the dispatcher process
    kproc_exec("printer_proc", &printer_proc, &run_q);               // Launch the printer process
    kproc_schedule();                                   // Start the process scheduler
    kproc_load(pcb[run_pid].trapframe_p);               // Load the first scheduled process (effectively: the idle task)
    return 0;                                           // should never be reached

}

/**
 * Kernel data structure initialization
 * Initializes all kernel data structures and variables
 */
void kdata_init() {

    int i, check = 0;                                  // Counter variable for the for loop and check for initialize qs

    // Initialize all of our kernel queues
	check = initializeQueue(&available_q);
	check += initializeQueue(&run_q);
    check += initializeQueue(&idle_q);
    check += initializeQueue(&sleep_q);
    check += initializeQueue(&semaphore_q);

    //If the initializeQueue function returns a non zero value, then queues were not initialized. Call panic()
    if(check != 0)
        panic("Error, the queues were not initialized properly. Null pointer found\n");

    // Initialize process control blocks and stacks
    sp_memset((char *)&pcb, 0, sizeof(pcb));
    sp_memset((char *)&stack, 0, sizeof(stack));
    sp_memset((char *)&semaphores, 0, sizeof(semaphores));
    sp_memset((char *)&mailboxes, 0, sizeof(mailboxes));

    // Ensure that all processes are initially in our available queue
    for (i = 0; i < PROC_MAX; i++) {
        enqueue(&available_q, i);
        pcb[i].state = AVAILABLE;                   // State of processes should be AVAILABLE
    }

    // Ensure that all semaphores and the corresponding array is initialized properly
    for(i = 0; i < SEMAPHORE_MAX; i++){

        enqueue(&semaphore_q, i);
        semaphores[i].count = 0;
        semaphores[i].init = SEMAPHORE_UNINITIALIZED;
        if(initializeQueue(&(semaphores[i].wait_q)) != 0)
           panic("Error, the semaphore wait_q could not be initialized\n");

    }

    //sem = SEMAPHORE_UNINITIALIZED;                  // Defalult value 
    system_time = 0;                                // Initialize system time
    run_pid = -1;                                   // Initializa the running pid'
    
}

/**
 * Interrupt Descriptor Table initialization
 * This adds entries to the IDT and then enables interrupts
 */
void idt_init() {
    // Get the IDT base address
    idt_p = get_idt_base();

    // Add an entry for each interrupt into the IDT
    idt_entry_add(TIMER_INTR, kisr_entry_timer);
    idt_entry_add(SYSCALL_INTR, syscall_interrupt); 

    // Clear the PIC mask to enable interrupts
    outportb(0x21, ~1);
}

/**
 * Adds an entry to the IDT
 * @param  entry_num - interrupt/entry number
 * @param  func_ptr  - pointer to the function (interrupt service routine) when
 *                     the interrupt is triggered.
 */
void idt_entry_add(int entry_num, func_ptr_t func_ptr) {
    struct i386_gate *gateptr;

    // Ensure that we have a valid entry
    if (entry_num < 0 || entry_num > 255) {
        panic("Invalid interrupt");
    }

    // Obtain the gate pointer for the entry
    gateptr = &idt_p[entry_num];

    // Fill the gate
    fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE, 0);
}

/**
 * Kernel run loop
 *  - Process interrupts
 *  - Run the process scheduler
 *  - Load processes to run
 *
 * @param  trapframe - pointer to the current trapframe
 */
void kernel_run(trapframe_t *trapframe) {
    char key;

    if (run_pid < 0 || run_pid > PID_MAX) {
        panic("Invalid PID!");
    }

    // save the trapframe into the PCB of the currently running process
    pcb[run_pid].trapframe_p = trapframe;

    // Process the current interrupt and call the appropriate service routine
    switch (trapframe->interrupt) {
        // Timer interrupt
        case TIMER_INTR:
            kisr_timer();
            break;

        case SYSCALL_INTR:
            kisr_syscall();
            break;

        default:
            panic("Invalid interrupt");
            break;
    }

    // Process special developer/debug commands
    if (cons_kbhit()) {
        key = cons_getchar();

        switch (key) {
            case 'b':
                // Set a breakpoint
                breakpoint();
                break;

            case 'n':
                // Create a new process
                kproc_exec("user_proc", &user_proc, &run_q);
                break;

            case 'p':
                // Trigger a panic (aborts)
                panic("User requested panic!");
                break;

            case 'x':
                // Exit the currently running process
                //printf("Inside case x\n");
                printf("RUN_PID inside X case : %d", run_pid);
                proc_exit();
                //printf("After kproc_exit() call");
                break;

            case 'q':
                // Exit our kernel
                cons_printf("Exiting Kernel!!!\n");
                printf("Exiting Kernel!!!\n");
                exit(0);
                break;

            default:
                // Display a warning (no abort)
                panic_warn("Unknown command entered");
                break;
        }
    }

    // Run the process scheduler
    kproc_schedule();

    // Load the next process
    kproc_load(pcb[run_pid].trapframe_p);
    
}
