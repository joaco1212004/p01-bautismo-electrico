/**
 * DM2 — Linker script mínimo para STM32F103C8T6 (Blue Pill)
 *
 * Flash: 64 KB @ 0x08000000
 * RAM:   20 KB @ 0x20000000
 *
 * Referencia: RM0008 §4 (Memory and bus architecture)
 *
 * Este script define:
 *   - MEMORY: regiones físicas disponibles
 *   - SECTIONS: cómo mapear las secciones del .o al espacio de memoria
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Top of stack = fin de RAM */
_stack_top = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(Reset_Handler)

SECTIONS
{
    /* .text: código + constantes → Flash */
    .text :
    {
        KEEP(*(.isr_vector))    /* tabla de vectores siempre primero */
        *(.text*)
        *(.rodata*)
    } > FLASH

    /* .data: variables inicializadas
     * VMA (dirección en runtime) = RAM
     * LMA (dirección en Flash)   = después de .text
     */
    .data :
    {
        _sdata = .;             /* inicio de .data en RAM  */
        *(.data*)
        _edata = .;             /* fin de .data en RAM     */
    } > RAM AT > FLASH

    /* _sidata: dirección en Flash donde empieza la copia de .data */
    _sidata = LOADADDR(.data);

    /* .bss: variables no inicializadas → solo en RAM (no ocupa Flash) */
    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    /* Stack: crece hacia abajo desde _stack_top */
    /* (no necesita sección explícita — lo gestiona el startup) */
}
