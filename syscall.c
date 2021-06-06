/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * System call APIs
 */
 
#include "syscall.h"
#include "kernel.h"

/*
 * Anatomy of a system call
 *
 * Data to/from the kernel is exchanged via the CPU trapframe
 * The system call is triggered via an interrupt that the kernel processes
 * Data can be sent to and received from the kernel in a single system call
 *
 * Data from the kernel:
 * int MySyscall() {
 *     int x;
 *     asm("movl %1, %%eax;"    // eax register indicates the syscall
 *         "int $0x80;"         // trigger the syscall via interrupt 0x80
 *         "movl %%ebx, %0;"    // pull data back from the kernel via register
 *         : "=g" (x)           // operand 0 is data returned from the kernel
 *         : "g" (SYSCALL_FOO)  // operand 1 is the syscall
 *         : "eax", "ebx");     // restore the registers that were used
 *     return x;
 * }
 *
 * Data to the kernel:
 * void MySyscall(int x) {
 *     int x;
 *     asm("movl %0, %%eax;"    // eax register indicates the syscall
 *         "movl %1, %%ebx;"    // send data to the kernel
 *         "int $0x80;"         // trigger the syscall via interrupt 0x80
 *         :                    // no operands for return data
 *         : "g" (SYSCALL_FOO), // operand 0 is the syscall
 *           "g" (x)            // operand 1 is the data we are sending
 *         : "eax", "ebx");     // restore the registers that were used
 * }
 *
 * Data to and from the kernel:
 * int MySyscall(int x) {
 *     int y;
 *     asm("movl %1, %%eax;"    // eax register indicates the syscall
 *         "movl %2, %%ebx;"    // send data to the kernel
 *         "int $0x80;"         // trigger the syscall via interrupt 0x80
 *         "movl %%ebx, %3;"    // pull data back from the kernel via register
 *         : "=g" (y)           // operand 0 is data returned from the kernel
 *         : "g" (SYSCALL_FOO), // operand 1 is the syscall
 *           "g" (x)            // operand 2 is the data we are sending
 *         : "eax", "ebx");     // restore the registers that were used
 *     return y;
 * }
 */

/**
 * Causes the process to exit
 *
 * @return  none
 */
void proc_exit()
{
    // trigger the system call
    // no data sent to the kernel
    // no data returned from the kernel
	
	asm("movl %0, %%eax;"
		"int %0x80;"    
        :
        : "g"(SYSCALL_PROC_EXIT)
        : "%eax");
		
	
} 

/**
 * Returns the current system time (in seconds)
 *
 * @return integer value for the system time in seconds
 */
int get_sys_time()
{
    // trigger the system call
    // no data sent to the kernel
    // data is returned from the kernel
	int SystemTime;
	
	asm("movl %1, %%eax;"
		"int $0x80;"
		"movl %%ebx, %0;"
		: "=g"(SystemTime)
		: "g"(SYSCALL_GET_SYS_TIME)
		: "%eax", "%ebx"		
		);
		
	return SystemTime;
}

/**
 * Returns the currently running (calling) process' process ID
 *
 * @return integer value representing the process ID; -1 on error
 */
int get_proc_pid() {

    int pid = 0;
    asm("movl %1, %%eax;"			//eax register indicates the syscall
        "int $0x80;"				//trigger the syscall
        "movl %%ebx, %0;"			//pull data back from the kernel via handler
        : "=g"(pid)					//op 0 is returned from kernel
        : "g"(SYSCALL_GET_PROC_PID)	//op 1 is the syscall
        : "%eax", "%ebx"			//restore the registers
		);

    return pid;
}

/**
 * Returns the currently running (calling) process' name
 *
 * @param   name
 * @return  0 upon success, other value upon error
 */
int get_proc_name(char *name) 
{
    // trigger the system call
    // destination pointer is sent to the kernel
    // no data is returned from the kernel
	
	asm("movl %0, %%eax;"
        "movl %1, %%ebx;"
		"int $0x80;"
		:
        :"g"(SYSCALL_GET_PROC_NAME),
		 "g"(name)
		:"%eax", "%ebx"
		);
		
	if (name != '\0')
	{
		return 0;
	}
	else
		panic_warn("Error at get_proc_name");

	return -1;

}

/**
 * Puts the currently running (calling) process to sleep for the
 * specified number of seconds.
 *
 * @param   seconds - number of seconds for the process to sleep
 * @return  none
 */
void sleep(int seconds)
{
    // trigger the system call
    // sleep amount (in seconds) is sent to the kernel
    // no data is returned from the kernel
/* 	asm("movl %1, %%eax;"
		"movl %2, %%ebx;"
		"int $0x80;"
		: "g"(SYSCALL_SLEEP)
		: "g"(seconds)
		: "%eax", "%ebx"
		); */
/* 	asm("movl %0, %%eax; int $0x80"
        :
        : "g"(seconds), "g"(SYSCALL_SLEEP)
        : "eax"
    ); */
	asm("movl %0, %%eax;"    // eax register indicates the syscall
          "movl %1, %%ebx;"    // send data to the kernel
          "int $0x80;"         // trigger the syscall via interrupt 0x80
          :                    // no operands for return data
          : "g" (SYSCALL_SLEEP), // operand 0 is the syscall
            "g" (seconds)            // operand 1 is the data we are sending
          : "%eax", "%ebx");     // restore the registers that were used
		  
}

void sem_init(sem_t *sem)
{	
	asm("movl %0, %%eax;"
		"movl %1, %%ebx;"
		"int $0x80;"
		:
		: "g" (SYSCALL_SEM_INIT),
		  "g" (sem)
		: "eax", "ebx");
}

void sem_wait(sem_t *sem)
{
	asm("movl %0, %%eax;"
		"movl %1, %%ebx;"
		"int $0x80;"
		:
		: "g" (SYSCALL_SEM_WAIT),
		  "g" (sem)
		: "eax", "ebx");
}

void sem_post(sem_t *sem)
{
	asm("movl %0, %%eax;"
		"movl %1, %%ebx;"
		"int $0x80;"
		:
		: "g" (SYSCALL_SEM_POST),
		  "g" (sem)
		: "eax", "ebx");
}
/*
	    asm("movl %0, %%eax;"    // eax register indicates the syscall
 *         "movl %1, %%ebx;"    // send data to the kernel
 *         "int $0x80;"         // trigger the syscall via interrupt 0x80
 *         :                    // no operands for return data
 *         : "g" (SYSCALL_FOO), // operand 0 is the syscall
 *           "g" (x)            // operand 1 is the data we are sending
 *         : "eax", "ebx");     // restore the registers that were used
 * */

void msg_send(msg_t *msg, int mbox_num)
{
	asm("movl %0, %%eax;"
		"movl %1, %%ebx;"
		"movl %2, %%ecx;"
		"int $0x80;"
		:
		: "g" (SYSCALL_MSG_SEND),
		  "g" (msg),
		  "g" (mbox_num)
		: "eax", "ebx", "ecx");
}

void msg_recv(msg_t *msg, int mbox_num)
{
	asm("movl %0, %%eax;"
		"movl %1, %%ebx;"
		"movl %2, %%ecx;"
		"int $0x80;"
		:
		: "g" (SYSCALL_MSG_RECV),
		  "g" (msg),
		  "g" (mbox_num)
		: "eax", "ebx", "ecx");
}
