/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * Kernel Interrupt Service Routines
 */
#ifndef KISR_H
#define KISR_H

/**
 * Interrupt Service Routine definitions
 */

#define TIMER_INTR 0x20                                     // Interrupt definitions
#define KSTACK_SIZE 16384                                   // kernel's stack size in bytes
#define KCODE 0x08                                          // kernel's code segment                         
#define KDATA 0x10                                          // kernel's data segment
#define SYSCALL_INTR 0x80                                   // system call interupt

#ifndef ASSEMBLER
/**
 * Function declarations
 */

void kisr_timer();                                          // Timer ISR
void kisr_syscall();                                        // Syscall ISR

/* Defined in kisr_entry.S */
__BEGIN_DECLS

// Kernel interrupt entries
extern void kisr_entry_timer();
extern void syscall_interrupt();

__END_DECLS
#endif
#endif
