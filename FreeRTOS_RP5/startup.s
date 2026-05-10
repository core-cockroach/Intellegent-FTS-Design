/*
 * startup.s – Secondary core (CPU3) boot for FreeRTOS.
 * Load address: 0xF0000000.
 * Expects to be woken by CPU0 via SEV after firmware parked this core.
 */

.extern main
.extern __bss_start
.extern __bss_end
.extern irqBlock
.extern vPortYieldProcessor
.extern vFreeRTOS_ISR

/* ========= Vector Table (must be first 32 bytes) ========= */
.section .vectors
.globl _start
_start:
    ldr pc, reset_handler
    ldr pc, undef_handler
    ldr pc, swi_handler
    ldr pc, prefetch_handler
    ldr pc, data_handler
    ldr pc, unused_handler
    ldr pc, irq_handler
    ldr pc, fiq_handler

reset_handler:      .word reset
undef_handler:      .word undefined_instruction
swi_handler:        .word vPortYieldProcessor
prefetch_handler:   .word prefetch_abort
data_handler:       .word data_abort
unused_handler:     .word unused
irq_handler:        .word vFreeRTOS_ISR
fiq_handler:        .word fiq

/* ========= All code in .text (close to other functions) ========= */
.section .text

/* CPU3 entry point – firmware branches here after SEV */
reset:
    /* Disable local interrupts (IRQ, FIQ) */
    mrs r0, cpsr
    orr r0, r0, #0xC0          /* I=1, F=1 */
    msr cpsr, r0

    /* Set Vector Base Address Register to this table (0xF0000000) */
    ldr r0, =_start
    mcr p15, 0, r0, c12, c0, 0   /* VBAR */

    /* ---- Set up stacks for each CPU mode ---- */
    /* Use a private stack area for core 3: 1 MB above load address */
    ldr r2, =0xF0100000          /* top of stacks */

    /* IRQ mode */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x12
    msr cpsr, r0
    mov sp, r2
    sub r2, r2, #0x1000

    /* FIQ mode */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x11
    msr cpsr, r0
    mov sp, r2
    sub r2, r2, #0x1000

    /* SVC mode (main execution) */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x13
    msr cpsr, r0
    mov sp, r2
    sub r2, r2, #0x4000

    /* ABT and UND modes – minimal stacks */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x17
    msr cpsr, r0
    mov sp, r2
    sub r2, r2, #0x200

    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x1B
    msr cpsr, r0
    mov sp, r2
    sub r2, r2, #0x200

    /* Return to SVC mode */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x13
    msr cpsr, r0

    /* Clear BSS */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
bss_loop:
    cmp r0, r1
    strlo r2, [r0], #4
    blo bss_loop

    /* Per‑core initialisation (GIC CPU interface, timer) */
    bl irqBlock

    /* Start FreeRTOS */
    bl main

/* Exception handlers – should never be hit */
undefined_instruction:
    b undefined_instruction
prefetch_abort:
    b prefetch_abort
data_abort:
    b data_abort
unused:
    b unused
fiq:
    b fiq

hang:
    wfi
    b hang