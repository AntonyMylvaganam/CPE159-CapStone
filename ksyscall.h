/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * System call APIs - Kernel Side
 */
#ifndef KSYSCALL_H
#define KSYSCALL_H

void ksyscall_get_sys_time();                                       // system information

/* Process information */
void ksyscall_get_proc_pid();
void ksyscall_get_proc_name();

// kernel system calls for data passing
void ksyscall_sem_init();
void ksyscall_sem_wait();
void ksyscall_sem_post();
void ksyscall_msg_send();
void ksyscall_msg_recv();

/* Additional functionality */
void ksyscall_sleep();                                        

#endif