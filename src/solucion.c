/**
 * DM2 — Práctica 1: Bautismo Eléctrico — SOLUCIÓN
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
        /* Esperar al flag de overflow (UIF = Update Interrupt Flag, bit 0) */
        if (TIM2_SR & (1U << 0)) {
            TIM2_SR &= ~(1U << 0);          /* limpiar el flag                */
            GPIOC_ODR ^= (1U << LED_PIN);   /* toggle PC13 (activo BAJO)      */
        }
        /* La CPU no está bloqueada — puede hacer otras cosas aquí */
    }
}

/* ── gpio_init ───────────────────────────────────────────────────────────── */
static void gpio_init(void)
{
    /* Paso 1: Habilitar clock del periférico GPIOC
     * RCC_APB2ENR bit 4 = IOPCEN                                             */
    RCC_APB2ENR |= (1U << 4);

    /* Paso 2: Configurar PC13 como salida push-pull a 10 MHz
     * PC13 está en CRH (pines 8-15). Bits [23:20] corresponden a PC13.
     * Cada pin usa 4 bits: CNF[1:0] | MODE[1:0]
     * Salida push-pull @ 10 MHz: CNF=00, MODE=01 → nibble = 0b0001           */
    GPIOC_CRH &= ~(0xFU << 20);   /* limpiar los 4 bits de PC13              */
    GPIOC_CRH |=  (0x1U << 20);   /* CNF=00, MODE=01                         */

    /* Paso 3: Inicializar HIGH (LED apagado — activo BAJO)                   */
    GPIOC_ODR |= (1U << LED_PIN);
}

/* ── timer_init ──────────────────────────────────────────────────────────── */
static void timer_init(void)
{
    /* Clock del sistema: 8 MHz (HSI por defecto, sin PLL configurado)
     * Objetivo: overflow cada 500 ms → toggle a 1 Hz                        */

    /* Paso 1: Habilitar clock de TIM2
     * RCC_APB1ENR bit 0 = TIM2EN                                             */
    RCC_APB1ENR |= (1U << 0);

    /* Paso 2: Prescaler → divide 8 MHz a 1 kHz
     * f_tim = f_clk / (PSC + 1) → PSC = 7999                                */
    TIM2_PSC = 7999U;

    /* Paso 3: Auto-reload → overflow cada 500 ms
     * t = (ARR + 1) / f_tim → ARR = 499                                     */
    TIM2_ARR = 499U;

    /* Paso 4: Arrancar el timer (CEN = bit 0 de CR1)                         */
    TIM2_CR1 |= (1U << 0);
}
