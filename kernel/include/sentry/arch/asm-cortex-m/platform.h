// SPDX-FileCopyrightText: 2023 Ledge SAS
// SPDX-License-Identifier: Apache-2.0

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#ifndef PLATFORM_H
#error "arch specific header must not be included directly!"
#endif

#include <sentry/arch/asm-cortex-m/system.h>
#include <sentry/arch/asm-cortex-m/scb.h>
#include <sentry/arch/asm-cortex-m/thread.h>
#include <sentry/managers/security.h>
#include <sentry/io.h>

#define THREAD_MODE_USER    0xab2f5332UL
#define THREAD_MODE_KERNEL  0x5371a247UL


#ifndef __WORDSIZE
#define __WORDSIZE 4UL
#endif

/**
 * @def alignment size of sections. 4bytes on ARM32
 */
#define SECTION_ALIGNMENT_LEN 0x4UL

/*@
   // Used at boot time to print out startup SP
   // can be, in Frama-C use case, deactivated (pr_info need only)
   assigns \nothing;
   ensures \result == 0;
 */
static inline uint32_t __platform_get_current_sp(void) {
  uint32_t sp = 0;
#ifndef __FRAMAC__
  asm volatile(
    "mov %0, sp"
    : "=r" (sp)
    :
    :);
#endif
    return sp;
}

/**
 * @def refuses unaligned accesses (word unaligned access for
 * word accesses or hword unaligned for hword accesses)
 * unaligned accesses is a path to multiple bugs such as invalid
 * htons/ntohs, invalid remote communication for hosts with strict
 * alignment, performances impacts, cache impact, etc.
 *
 * when activated, unaligned access generates UsageFault
 */
/*@
  assigns ((SCB_Type*)SCB_BASE)->CCR;
 */
static inline void __platform_enforce_alignment(void) {
    SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
    request_data_membarrier();
}


/*@
  // NOTE: in Frama-C mode, the kernel do not spawn a user task, but just "leave", as the coverage of the
  // entrypoint is complete
  assigns *NVIC;
*/
static inline void __attribute__((noreturn)) __platform_spawn_thread(size_t entrypoint, stack_frame_t *stack_pointer, uint32_t flag) {
  /*
   * Initial context switches to main core thread (idle_thread).
   */
  uint32_t runlevel = 0; /* spawning very first thread here, rerun == 0*/
  uint32_t seed = 0UL;
  /* at init time, used for idle task only, RNG is setup, should not fail */
  mgr_security_entropy_generate(&seed);
  runlevel = 3;  /* user, PSP */
  if (flag == THREAD_MODE_KERNEL) {
    runlevel = 2; /* privileged, PSP */
  }

  /*
   * Once interrupt enable in Kernel Thread Mode, make sure that any further statement
   * and function call are preemptible. This is not the case for instance in kernel
   * drivers, if an irq use a mappable device, the previously used device is not mapped
   * anymore.
   */
  interrupt_enable();

#ifndef __FRAMAC__
  asm volatile
       ("mov r0, %[SP]      \n\t"   \
        "msr psp, r0        \n\t"   \
        "mov r0, %[LVL]     \n\t"   \
        "msr control, r0    \n\t"   \
        "isb                \n\t"   \
        "mov r0, %[THREADID] \n\t"  \
        "mov r1, %[SEED]    \n\t"   \
	      "mov r5, %[PC]      \n\t"   \
	      "bx r5              \n\t"   \
        :
        : [PC] "r" (entrypoint),
          [SP] "r" (stack_pointer),
          [LVL] "r" (runlevel),
          [THREADID] "r" (runlevel),
          [SEED] "r" (seed)
        : "r0", "r1", "r5", "memory");
  __builtin_unreachable();
#endif
}

static inline void __platform_clear_flags(void) {
    /*
     * clean potential previous faults, typically when using jtag flashing
     */
    uint32_t cfsr = ioread32((size_t)r_CORTEX_M_SCB_CFSR);
    iowrite32((size_t)r_CORTEX_M_SCB_CFSR, cfsr | cfsr);
    return;
}


void __platform_init(void);


#endif/*!__PLATFORM_H_*/
