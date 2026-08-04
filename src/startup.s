/**
 * DM2 — Startup mínimo para STM32F103C8T6 (Cortex-M3)
 *
 * Hace exactamente tres cosas:
 *   1. Define la tabla de vectores (Reset, NMI, faults)
 *   2. Copia .data de Flash → RAM
 *   3. Inicializa .bss a cero
 *   4. Llama a main()
 *
 */

    .syntax unified
    .cpu cortex-m3
    .thumb

/* ── Tabla de vectores ───────────────────────────────────────────────────── */
/* Debe quedar al inicio de Flash (0x08000000) → garantizado por linker.ld   */
    .section .isr_vector, "a", %progbits
    .type isr_vector, %object

isr_vector:
    .word   _stack_top          /* 0x00: top of stack (from linker.ld)    */
    .word   Reset_Handler       /* 0x04: reset                            */
    .word   Default_Handler     /* 0x08: NMI                              */
    .word   Default_Handler     /* 0x0C: HardFault                        */
    .word   Default_Handler     /* 0x10: MemManage                        */
    .word   Default_Handler     /* 0x14: BusFault                         */
    .word   Default_Handler     /* 0x18: UsageFault                       */
    .word   0                   /* 0x1C–0x28: reserved (4 words)          */
    .word   0
    .word   0
    .word   0
    .word   Default_Handler     /* 0x2C: SVCall                           */
    .word   Default_Handler     /* 0x30: Debug Monitor                    */
    .word   0                   /* 0x34: reserved                         */
    .word   Default_Handler     /* 0x38: PendSV                           */
    .word   Default_Handler     /* 0x3C: SysTick                          */
    /* IRQs externos: agregar aquí si se usan interrupciones */

/* ── Reset_Handler ───────────────────────────────────────────────────────── */
    .text
    .thumb_func
    .global Reset_Handler
    .type   Reset_Handler, %function

Reset_Handler:
    /* Paso 1: Copiar .data de Flash (LMA) → RAM (VMA) */
    ldr     r0, =_sdata         /* destino: inicio de .data en RAM        */
    ldr     r1, =_edata         /* fin de .data en RAM                    */
    ldr     r2, =_sidata        /* fuente: .data en Flash (load address)  */
    b       copy_check

copy_loop:
    ldr     r3, [r2], #4
    str     r3, [r0], #4

copy_check:
    cmp     r0, r1
    blt     copy_loop

    /* Paso 2: Inicializar .bss a cero */
    ldr     r0, =_sbss
    ldr     r1, =_ebss
    mov     r2, #0
    b       bss_check

bss_loop:
    str     r2, [r0], #4

bss_check:
    cmp     r0, r1
    blt     bss_loop

    /* Paso 3: Saltar a main() */
    bl      main

    /* Si main() retorna (no debería), quedar en loop infinito */
    b       .

/* ── Default_Handler ─────────────────────────────────────────────────────── */
    .thumb_func
    .global Default_Handler
    .type   Default_Handler, %function

Default_Handler:
    b       .           /* loop infinito: poner breakpoint aquí para debug */

    .size Reset_Handler, . - Reset_Handler
