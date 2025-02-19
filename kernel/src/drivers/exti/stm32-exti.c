// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: Apache-2.0

/**
 * \file STM32 External interrupts controler driver (see ST RM0090 datasheet)
 */
#include <assert.h>
#include <stdbool.h>
#include <stdatomic.h>

#include <sentry/arch/asm-cortex-m/layout.h>
#include <sentry/io.h>
#include <sentry/bits.h>
#include <sentry/ktypes.h>
#include <sentry/managers/memory.h>
#include "exti_defs.h"
#include "stm32-exti-dt.h"

#if defined(CONFIG_SOC_SUBFAMILY_STM32L4) || \
    defined(CONFIG_SOC_SUBFAMILY_STM32U5) || \
    defined(CONFIG_SOC_SUBFAMILY_STM32WB)
/* there are 40 possible interrupts and events sources, meaning that
 * there are two register banks, redirecting default naming to bank 1 */
#if !defined(CONFIG_SOC_SUBFAMILY_STM32WB)
#define EXTI_IMR_REG EXTI_IMR1_REG
#define EXTI_EMR_REG EXTI_EMR1_REG
#else
#define EXTI_IMR_REG EXTI_C1IMR1_REG
#define EXTI_EMR_REG EXTI_C1EMR1_REG
#endif

#define EXTI_RTSR_REG EXTI_RTSR1_REG
#define EXTI_FTSR_REG EXTI_FTSR1_REG
#define EXTI_SWIER_REG EXTI_SWIER1_REG
# if !defined(CONFIG_SOC_SUBFAMILY_STM32U5)
/* XXX: U5 got 2 registers for rising edge pending and falling edge pending */
#define EXTI_PR_REG EXTI_PR1_REG
# endif
#endif

static inline kstatus_t exti_map(void)
{
    stm32_exti_desc_t const * desc = stm32_exti_get_desc();
    return mgr_mm_map_kdev(desc->base_addr, desc->size);
}
/* for simplicity sake, but unmaping a kernel device is generic */
static inline kstatus_t exti_unmap(void) {
    return mgr_mm_unmap_kdev();
}

static inline void __exti_clear_pending(uint32_t mask)
{
#if defined(CONFIG_SOC_SUBFAMILY_STM32U5)
    iowrite32(EXTI_BASE_ADDR + EXTI_RPR1_REG, mask);
    iowrite32(EXTI_BASE_ADDR + EXTI_FPR1_REG, mask);
#else
    iowrite32(EXTI_BASE_ADDR + EXTI_PR_REG, mask);
#endif
}

/**
 * @brief probe for EXTI controller
 *
 * EXTI fields are set to their reset value
 *
 * @return K_STATUS_OKAY
 */
/*
  // to be replaced with ghost functions
  // assigns all registers
  assigns *(uint32_t*)(EXTI_BASE_ADDR .. EXTI_BASE_ADDR + EXTI_PR_REG);
  ensures \result == K_STATUS_OKAY;
 */
kstatus_t exti_probe(void)
{
    kstatus_t status = K_STATUS_OKAY;
    if (unlikely((status = exti_map()) != K_STATUS_OKAY)) {
        goto err;
    }

#if !defined(CONFIG_SOC_SUBFAMILY_STM32U5)
    if (unlikely(ioread32(EXTI_BASE_ADDR + EXTI_PR_REG))) {
        /** FIXME: there is some pending interrupts here, action to define */
        /** INFO: the register read had cleared the pending interrupts */
    }
#endif
    iowrite32(EXTI_BASE_ADDR + EXTI_IMR_REG, 0x0UL);
    iowrite32(EXTI_BASE_ADDR + EXTI_EMR_REG, 0x0UL);
    iowrite32(EXTI_BASE_ADDR + EXTI_RTSR_REG, 0x0UL);
    iowrite32(EXTI_BASE_ADDR + EXTI_FTSR_REG, 0x0UL);
    iowrite32(EXTI_BASE_ADDR + EXTI_SWIER_REG, 0x0UL);

    /* clear pending (rc_w1) */
#if defined(CONFIG_SOC_SUBFAMILY_STM32U5)
    __exti_clear_pending(0x03ffffffUL);
#else
    __exti_clear_pending(0x7fffffUL);
#endif
#if CONFIG_SOC_SUBFAMILY_STM32L4
    iowrite32(EXTI_BASE_ADDR + EXTI_IMR2_REG, 0x0UL);
    iowrite32(EXTI_BASE_ADDR + EXTI_EMR2_REG, 0x0UL);
    iowrite32(EXTI_BASE_ADDR + EXTI_RTSR2_REG, 0x0UL);
    iowrite32(EXTI_BASE_ADDR + EXTI_FTSR2_REG, 0x0UL);
    iowrite32(EXTI_BASE_ADDR + EXTI_SWIER2_REG, 0x0UL);
#endif
    exti_unmap();
err:
    return status;
}

/**
 * @brief mask external interrupt itn
 */
/*
  // to be replaced with ghost functions
  assigns *(uint32_t*)(EXTI_BASE_ADDR + EXTI_IMR_REG);
  ensures itn > EXTI_NUM_EVENTS <==> \result == K_ERROR_INVPARAM;
  ensures itn <= EXTI_NUM_EVENTS <==> \result == K_STATUS_OKAY;
 */
kstatus_t exti_mask_interrupt(uint8_t itn)
{
    kstatus_t status = K_STATUS_OKAY;
    if (unlikely(itn > EXTI_NUM_EVENTS)) {
        status = K_ERROR_INVPARAM;
        goto err;
    }
    if (unlikely((status = exti_map()) != K_STATUS_OKAY)) {
        goto err;
    }
    if (itn < 32) {
        size_t reg = ioread32(EXTI_BASE_ADDR + EXTI_IMR_REG);
        reg &= ~(1UL << itn);
        iowrite32(EXTI_BASE_ADDR + EXTI_IMR_REG, reg);
    }
# if defined(CONFIG_SOC_SUBFAMILY_STM32L4)
    else {
        size_t reg = ioread32(EXTI_BASE_ADDR + EXTI_IMR2_REG);
        reg &= ~(1UL << (itn % 32));
        iowrite32(EXTI_BASE_ADDR + EXTI_IMR2_REG, reg);
    }
#endif
    exti_unmap();
err:
    return status;
}

/**
 * @brief unmask external interrupt itn
 */
/*
  // to be replaced with ghost functions
  assigns *(uint32_t*)(EXTI_BASE_ADDR + EXTI_IMR_REG);
  ensures itn > EXTI_NUM_EVENTS <==> \result == K_ERROR_INVPARAM;
  ensures itn <= EXTI_NUM_EVENTS <==> \result == K_STATUS_OKAY;
 */
kstatus_t exti_unmask_interrupt(uint8_t itn)
{
    kstatus_t status = K_STATUS_OKAY;
    if (unlikely(itn > EXTI_NUM_EVENTS)) {
        status = K_ERROR_INVPARAM;
        goto err;
    }
    if (unlikely((status = exti_map()) != K_STATUS_OKAY)) {
        goto err;
    }
    if (itn < 32) {
        size_t reg = ioread32(EXTI_BASE_ADDR + EXTI_IMR_REG);
        reg |= (1UL << itn);
        iowrite32(EXTI_BASE_ADDR + EXTI_IMR_REG, reg);
    }
# if defined(CONFIG_SOC_SUBFAMILY_STM32L4)
    else {
        size_t reg = ioread32(EXTI_BASE_ADDR + EXTI_IMR2_REG);
        reg &= ~(1UL << (itn % 32));
        iowrite32(EXTI_BASE_ADDR + EXTI_IMR2_REG, reg);
    }
#endif
    exti_unmap();
err:
    return status;
}

/**
 * @brief mask external event evn
 */
/*
  // to be replaced with ghost functions
  assigns *(uint32_t*)(EXTI_BASE_ADDR + EXTI_EMR_REG);
  ensures evn > EXTI_NUM_EVENTS <==> \result == K_ERROR_INVPARAM;
  ensures evn <= EXTI_NUM_EVENTS <==> \result == K_STATUS_OKAY;
 */
kstatus_t exti_mask_event(uint8_t evn)
{
    kstatus_t status = K_STATUS_OKAY;
    if (unlikely(evn > EXTI_NUM_EVENTS)) {
        status = K_ERROR_INVPARAM;
        goto err;
    }
    if (unlikely((status = exti_map()) != K_STATUS_OKAY)) {
        goto err;
    }
    if (evn < 32) {
        size_t reg = ioread32(EXTI_BASE_ADDR + EXTI_EMR_REG);
        reg &= ~(1UL << evn);
        iowrite32(EXTI_BASE_ADDR + EXTI_EMR_REG, reg);
    }
# if defined(CONFIG_SOC_SUBFAMILY_STM32L4)
    else {
        size_t reg = ioread32(EXTI_BASE_ADDR + EXTI_EMR2_REG);
        reg &= ~(1UL << (evn % 32));
        iowrite32(EXTI_BASE_ADDR + EXTI_EMR2_REG, reg);
    }
#endif
    exti_unmap();
err:
    return status;
}

/**
 * @brief unmask external event evn
 */
/*
  // to be replaced with ghost functions
  assigns *(uint32_t*)(EXTI_BASE_ADDR + EXTI_EMR_REG);
  ensures evn > EXTI_NUM_EVENTS <==> \result == K_ERROR_INVPARAM;
  ensures evn <= EXTI_NUM_EVENTS <==> \result == K_STATUS_OKAY;
 */
kstatus_t exti_unmask_event(uint8_t evn)
{
    kstatus_t status = K_STATUS_OKAY;
    if (unlikely(evn > EXTI_NUM_EVENTS)) {
        status = K_ERROR_INVPARAM;
        goto err;
    }
    if (unlikely((status = exti_map()) != K_STATUS_OKAY)) {
        goto err;
    }
    if (evn < 32) {
        size_t reg = ioread32(EXTI_BASE_ADDR + EXTI_EMR_REG);
        reg |= (1UL << evn);
        iowrite32(EXTI_BASE_ADDR + EXTI_EMR_REG, reg);
    }
# if defined(CONFIG_SOC_SUBFAMILY_STM32L4)
    else {
        size_t reg = ioread32(EXTI_BASE_ADDR + EXTI_EMR2_REG);
        reg &= ~(1UL << (evn % 32));
        iowrite32(EXTI_BASE_ADDR + EXTI_EMR2_REG, reg);
    }
#endif
    exti_unmap();
err:
    return status;
}

/**
 * @brief generate interrupt identified by itn (software triggered)
 */
/*
  // to be replaced with ghost functions
  assigns *(uint32_t*)(EXTI_BASE_ADDR + EXTI_EMR_REG);
  ensures itn > EXTI_NUM_INTERRUPTS <==> \result == K_ERROR_INVPARAM;
  ensures itn <= EXTI_NUM_INTERRUPTS ==>
      \result == K_STATUS_OKAY || \result == K_ERROR_BADSTATE;
 */
kstatus_t exti_generate_swinterrupt(uint8_t itn)
{
    kstatus_t status = K_STATUS_OKAY;
    size_t reg;
    if (unlikely(itn > EXTI_NUM_INTERRUPTS)) {
        status = K_ERROR_INVPARAM;
        goto err;
    }
    if (unlikely((status = exti_map()) != K_STATUS_OKAY)) {
        goto err;
    }
    if (itn < 32) {
        reg = ioread32(EXTI_BASE_ADDR + EXTI_IMR_REG);
        if (unlikely((reg & (0X1UL << itn)) == 0)) {
            /* interrupt is masked */
            status = K_ERROR_BADSTATE;
            goto err;
        }
        reg = ioread32(EXTI_BASE_ADDR + EXTI_SWIER_REG);
        if (unlikely((reg & (0X1UL << itn)))) {
            /* bit already set */
            status = K_ERROR_BADSTATE;
            goto err;
        }
        reg |= (1UL << itn);
        iowrite32(EXTI_BASE_ADDR + EXTI_EMR_REG, reg);
    }
#if defined(CONFIG_SOC_SUBFAMILY_STM32L4)
    else {
        reg = ioread32(EXTI_BASE_ADDR + EXTI_IMR2_REG);
        if (unlikely((reg & (0X1UL << (itn % 32))) == 0)) {
            /* interrupt is masked */
            status = K_ERROR_BADSTATE;
            goto err;
        }
        reg = ioread32(EXTI_BASE_ADDR + EXTI_SWIER2_REG);
        if (unlikely((reg & (0X1UL << (itn % 32))))) {
            /* bit already set */
            status = K_ERROR_BADSTATE;
            goto err;
        }
        reg |= (1UL << (itn % 32));
        iowrite32(EXTI_BASE_ADDR + EXTI_EMR2_REG, reg);
    }
#endif
    exti_unmap();
err:
    return status;
}

/**
 * @brief Clear pending interrupt flag for itn
 */
/*
  // to be replaced with ghost functions
  assigns *(uint32_t*)(EXTI_BASE_ADDR + EXTI_PR_REG);
  ensures itn > EXTI_NUM_INTERRUPTS <==> \result == K_ERROR_INVPARAM;
  ensures itn <= EXTI_NUM_INTERRUPTS ==>
      \result == K_STATUS_OKAY;
 */
kstatus_t exti_clear_pending(uint8_t itn)
{
    kstatus_t status = K_STATUS_OKAY;
    if (unlikely(itn > EXTI_NUM_INTERRUPTS)) {
        status = K_ERROR_INVPARAM;
        goto err;
    }
    if (unlikely((status = exti_map()) != K_STATUS_OKAY)) {
        goto err;
    }
    if (itn < 32) {
        /* clear pending (rc_w1) */
        __exti_clear_pending(0x1UL << itn);
    }
#if defined(CONFIG_SOC_SUBFAMILY_STM32L4)
    else {
        iowrite32(EXTI_BASE_ADDR + EXTI_PR2_REG, (0x1UL << (itn % 32)));
    }
#endif
    exti_unmap();
err:
    return status;
}
