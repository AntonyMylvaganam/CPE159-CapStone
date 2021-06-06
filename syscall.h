#ifndef SYSCALL_H
#define SYSCALL_H

// IPC data structures (needed for forward declarations below)
#include "ipc.h"

/*
 * Forces a process to exit
 */
void proc_exit(void);

/*
 * Obtains the current system uptime (in seconds)
 * @return time in seconds
 */
int get_sys_time(void);

/*
 * Obtains the running process' id (pid)
 * @return integer < 0 on error, positive integer referring to the
 *         process id
 */
int get_proc_pid(void);

/*
 * Obtains the running process' name
 * @param name - pointer to the string where the name should be stored
 * @return 0 on success, other value upon error
 */
int get_proc_name(char *name);

/*
 * Forces the process to "sleep" for the specified number of seconds
 * @param seconds - number of seconds to sleep
 */
void sleep(int seconds);

/*
 * Initialize a semaphore
 * @param sem - pointer to the semaphore identifier
 * @return none
 */
void sem_init(sem_t *sem);

/*
 * Waits on a semaphore to be posted
 * @param sem - pointer to the semaphore identifier
 * @return none
 */
void sem_wait(sem_t *sem);

/*
 * Posts a semaphore
 * @param sem - pointer to the semaphore identifier
 * @return none
 */
void sem_post(sem_t *sem);

/*
 * Send a message to the specified mailbox
 * @param  msg - pointer to the message data structure for the
 *         message to be sent
 * @param  mailbox - mailbox number
 * @return none
 */
void msg_send(msg_t *msg, int mbox_num);

/*
 * Receive a message from the specified mailbox
 * @param  msg - pointer to the message data structure for the
 *         received message
 * @param  mailbox - mailbox number
 * @return none
 */
void msg_recv(msg_t *msg, int mbox_num);

#endif
