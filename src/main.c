/**
 * DM2 — Práctica 1: Bautismo Eléctrico
 * Bare minimum blinky: STM32F103C8T6 (Blue Pill)
 * LED: PC13 (activo BAJO — enciende con 0, apaga con 1)
 * Timer: TIM2
 *
 * Sin HAL. Sin CMSIS. Solo registros.
 * Referencia: RM0008 (STM32F103 Reference Manual)
 */

#include <stdint.h>

/* ── Register base addresses ─────────────────────────────────────────────── */
/* Source: RM0008 Table 3 — Memory map                                        */

#define RCC_BASE    0x40021000UL
#define GPIOC_BASE  0x40011000UL
#define TIM2_BASE   0x40000000UL

/* ── RCC registers ───────────────────────────────────────────────────────── */
#define RCC_APB2ENR  (*(volatile uint32_t *)(RCC_BASE + 0x18U))  /* GPIOC clock */
#define RCC_APB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x1CU))  /* TIM2 clock  */

/* ── GPIOC registers ─────────────────────────────────────────────────────── */
/* F1 series: GPIO config usa CRL (pins 0-7) y CRH (pins 8-15)               */
/* Cada pin ocupa 4 bits: CNF[1:0] | MODE[1:0]                                */
#define GPIOC_CRH  (*(volatile uint32_t *)(GPIOC_BASE + 0x04U))
#define GPIOC_ODR  (*(volatile uint32_t *)(GPIOC_BASE + 0x0CU))
#define GPIOC_BSRR (*(volatile uint32_t *)(GPIOC_BASE + 0x10U))  /* set/reset atómico */

/* ── TIM2 registers ──────────────────────────────────────────────────────── */
#define TIM2_CR1  (*(volatile uint32_t *)(TIM2_BASE + 0x00U))
#define TIM2_SR   (*(volatile uint32_t *)(TIM2_BASE + 0x10U))
#define TIM2_PSC  (*(volatile uint32_t *)(TIM2_BASE + 0x28U))
#define TIM2_ARR  (*(volatile uint32_t *)(TIM2_BASE + 0x2CU))

/* ── Pin definitions ─────────────────────────────────────────────────────── */
/* PC13: LED del Blue Pill — ACTIVO BAJO (LOW = encendido, HIGH = apagado)    */
#define LED_PIN  13U

/* ── Function prototypes ─────────────────────────────────────────────────── */
static void gpio_init(void);
static void timer_init(void);

/* ── main ────────────────────────────────────────────────────────────────── */
int main(void)
{
    gpio_init();
    timer_init();

    while (1) {
        /* TODO (Actividad 11):
         * - Comprobar si el flag UIF (bit 0 de TIM2_SR) está seteado
         * - Si es así: limpiar el flag y hacer toggle de PC13
         *
         * Pista:
         *   if (TIM2_SR & (1 << 0)) { ... }
         *
         * Recordar: PC13 es activo BAJO.
         * Toggle con XOR: GPIOC_ODR ^= (1 << LED_PIN);
         */
    }
}

/* ── gpio_init ───────────────────────────────────────────────────────────── */
static void gpio_init(void)
{
    /* TODO (Actividad 9):
     *
     * Paso 1: Habilitar clock del periférico GPIOC
     *   → RCC_APB2ENR bit 4 (IOPCEN)
     *
     * Paso 2: Configurar PC13 como salida push-pull a 10 MHz
     *   PC13 está en CRH (pines 8–15). Bits [23:20] corresponden a PC13.
     *   Cada pin usa 4 bits: CNF[1:0] | MODE[1:0]
     *
     *   Salida push-pull @ 10 MHz: CNF = 00, MODE = 01 → nibble = 0b0001
     *
     *   Secuencia lectura-modificación-escritura:
     *     GPIOC_CRH &= ~(0xFU << 20);   // limpiar los 4 bits de PC13
     *     GPIOC_CRH |=  (0x1U << 20);   // CNF=00, MODE=01
     *
     * Paso 3: Inicializar el pin en HIGH (LED apagado — activo BAJO)
     *   GPIOC_ODR |= (1 << LED_PIN);
     *
     * Referencia: RM0008 §9.2.2 (GPIOx_CRH)
     */
}

/* ── timer_init ──────────────────────────────────────────────────────────── */
static void timer_init(void)
{
    /* TODO (Actividad 10):
     *
     * Objetivo: overflow de TIM2 cada 500 ms → toggle a 1 Hz
     *
     * Clock del sistema: 8 MHz (HSI por defecto, sin PLL configurado)
     *
     * Paso 1: Habilitar clock de TIM2
     *   → RCC_APB1ENR bit 0 (TIM2EN)
     *
     * Paso 2: Configurar prescaler (PSC)
     *   f_tim = f_clk / (PSC + 1)
     *   Para f_tim = 1 kHz → PSC = 7999
     *
     * Paso 3: Configurar auto-reload (ARR)
     *   t_overflow = (ARR + 1) / f_tim
     *   Para t = 500 ms → ARR = 499
     *
     * Paso 4: Arrancar el timer
     *   → TIM2_CR1 bit 0 (CEN = Counter Enable)
     *
     * Referencia: RM0008 §15.4 (TIM2 registers)
     */
}
