/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Global Definitions
 */
#ifndef GLOBAL_H
#define GLOBAL_H

// Maximum number of processes we will support
#define PROC_MAX 20
#define PROC_NAME_LEN 32

// Number of times to loop over IO_DELAY() to delay for one second
#define IO_DELAY_LOOP 1666666

// void-return function pointer type
typedef void (*func_ptr_t)();

#endif
