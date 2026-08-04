# DM2 — Práctica 1: Bautismo Eléctrico

## Toolchain

Este proyecto **no usa PlatformIO ni CMSIS** — es bare-metal puro con `arm-none-eabi-gcc` y un `Makefile`.

| Herramienta | Uso | Instalar |
|-------------|-----|---------|
| `arm-none-eabi-gcc` | Compilador + linker | `sudo apt install gcc-arm-none-eabi` |
| `openocd` | Flash via ST-Link v2 | `sudo apt install openocd` |
| `st-flash` *(alternativa)* | Flash directo | `sudo apt install stlink-tools` |

Verificar:
```bash
arm-none-eabi-gcc --version
openocd --version
```

`make` genera `blinky.elf` + `blinky.bin` sin dependencias adicionales.

---



**Diseño Mecatrónico II · IOR444 · Universidad de San Andrés**
Sesión 1 · 2026-03-03

---

## Objetivos

Al finalizar esta práctica van a poder:

1. Operar con seguridad en el laboratorio de electrónica
2. Medir voltaje, corriente y resistencia con multímetro
3. Armar un circuito LED+R y verificar la Ley de Ohm con sus propias manos
4. Compilar y flashear un proyecto bare-metal en STM32 (sin HAL)
5. Hacer parpadear un LED usando un timer — **sin `delay()`**
6. Medir la frecuencia real del blink con osciloscopio

---

## Prerequisitos (antes de llegar)

- [ ] PlatformIO o toolchain `arm-none-eabi-gcc` instalado — evidencia: screenshot de proyecto que compila
- [ ] Módulo 0 de nivelación aprobado (test de seguridad eléctrica)
- [ ] Este repo clonado en la notebook del grupo

```bash
git clone git@github.com:udesa-ai/dm2-26c1.git
cd dm2-26c1
```

---

## Material y equipamiento

| Ítem | Cantidad |
|------|----------|
| Placa STM32F103C8T6 (Blue Pill) | 1 |
| Programador ST-Link v2 (el chino) + cables | 1 |
| Cable USB-A → micro-USB (para alimentación / ST-Link) | 1 |
| Breadboard | 1 |
| LED rojo (5 mm) | 1 |
| Resistencias surtidas (100 Ω, 330 Ω, 1 kΩ, 10 kΩ) | 1 juego |
| Multímetro + cables | 1 |
| Osciloscopio o analizador lógico | 1 (compartido) |
| Cables Dupont M-M y M-F | varios |

---

## Fase 0 — Seguridad y Acceso al Laboratorio (≈ 10 min)

> **Bloqueante:** no continuar sin completar esta fase.

### Actividad 1 — Acceso al laboratorio

- Leer y firmar la licencia de uso del laboratorio
- Identificar matafuego y salidas de emergencia
- Revisar rangos de voltaje seguros: **3.3 V y 5 V** para esta sesión
- Protocolo de incendio eléctrico: CO₂, **NO agua**
- **Sin aprobación = prohibido operar circuitos**

### Actividad 2 — Checklist eléctrico mínimo

Completar y registrar en este README (sección "Evidencia" al final):

- [ ] GND común entre todos los instrumentos antes de energizar
- [ ] Límite de corriente por pin GPIO: **25 mA máximo** (STM32F103, 80 mA total por puerto)
- [ ] Verificar polaridad antes de conectar la placa

### Actividad 3 — Checkout del multímetro

1. Poner el multímetro en modo **VDC**
2. Medir el riel de **3.3 V** de la placa → anotar: `V_3V3 = ___`
3. Medir el riel de **5 V** (VBUS del USB) → anotar: `V_5V = ___`
4. Poner el multímetro en modo **continuidad** (símbolo de diodo/beep)
5. Medir entre chasis metálico y GND de la placa → debe **sonar** (continuidad = ok)

---

## Fase 1 — Electrónica de Banco (≈ 25 min)

### Actividad 4 — Medir una resistencia

1. Tomar una resistencia de **1 kΩ** (código de colores: marrón-negro-rojo)
2. Medir con multímetro en modo **Ω**
3. Anotar: nominal = 1000 Ω · medido = `___` Ω · error = `___` %

> ¿Qué tolerancia tiene? ¿El valor medido cae dentro de esa tolerancia?

### Actividad 5 — Calcular y armar circuito LED + R

Queremos hacer circular **20 mA** por un LED rojo (V~F~ ≈ 2.0 V) con V~CC~ = 3.3 V.

$$R = \frac{V_{CC} - V_F}{I_F} = \frac{3.3\ \text{V} - 2.0\ \text{V}}{20\ \text{mA}} = $$

- Elegir el valor comercial más cercano disponible: **68 Ω** o **100 Ω**
- Armar el circuito en breadboard: `3.3V → R → LED → GND`

```
3V3 ──┤R (68Ω)├──┤LED (ánodo)├──┤LED (cátodo)├── GND
```

> ⚠️ El ánodo del LED es la pata más larga.

### Actividad 6 — Verificar Ley de Ohm

Con el circuito armado y energizado:

1. Medir **V~R~** (caída en la resistencia): colocar puntas del multímetro en ambos extremos de R
2. Medir **V~LED~** (caída en el LED)
3. Verificar: V~R~ + V~LED~ ≈ 3.3 V
4. Calcular la corriente real: **I = V~R~ / R**

Anotar:
- `V_R = ___` V
- `V_LED = ___` V
- `I_real = ___` mA (¿razonable para un LED encendido?)

### Actividad 7 — (Opcional / lab abierto) Divisor de tensión

Armar un divisor con R1 = 10 kΩ y R2 = 4.7 kΩ:

$$V_{out} = V_{CC} \cdot \frac{R2}{R1 + R2}$$

- Predecir V~out~ teórico → anotar
- Medir V~out~ real → anotar
- Comparar: ¿qué lo aleja del ideal? (pista: tolerancia de resistencias, impedancia del multímetro)

---

## Fase 2 — Toolchain y Firmware (≈ 40 min)

> **Objetivo:** un LED parpadeando a ~1 Hz usando un timer hardware, sin ningún `delay()`.

### Estructura del proyecto

El starter code de este repo tiene:

```
src/
├── main.c       ← esqueleto con TODOs
├── startup.s    ← tabla de vectores + Reset_Handler
└── linker.ld    ← mapa de memoria Flash/RAM
Makefile         ← arm-none-eabi-gcc
```

### Actividad 8 — Compilar el proyecto bare minimum

```bash
make
```

Debería generar `blinky.elf` y `blinky.bin`. Si falla, verificar que `arm-none-eabi-gcc` está en el PATH.

Inspeccionar el tamaño:
```bash
arm-none-eabi-size blinky.elf
```

> ¿Cuántos bytes ocupa `.text`? ¿Cuántos `.data` y `.bss`?

### Actividad 9 — Configurar GPIO de salida

Abrir `src/main.c`. Completar la función `gpio_init()`:

```c
// 1. Habilitar clock del periférico GPIOC
//    RCC_APB2ENR bit 4 (IOPCEN)
RCC_APB2ENR |= (1 << 4);

// 2. Configurar PC13 como salida push-pull a 10 MHz
//    En F1, cada pin usa 4 bits en CRH (pins 8-15):
//    CNF[1:0] | MODE[1:0] — push-pull @ 10 MHz: CNF=00, MODE=01 → 0b0001
//    PC13 ocupa los bits [23:20] de CRH
GPIOC_CRH &= ~(0xFU << 20);   // limpiar
GPIOC_CRH |=  (0x1U << 20);   // CNF=00, MODE=01

// 3. Inicializar HIGH (LED apagado — activo BAJO)
GPIOC_ODR |= (1 << 13);
```

> **PC13** es el LED del Blue Pill — **activo BAJO**: escribir `0` enciende, `1` apaga.

> 🔍 Verificar en el Reference Manual (RM0008): buscar "GPIOx_CRH" y confirmar la dirección base de GPIOC `0x40011000`.

### Actividad 10 — Configurar el timer

Completar `timer_init()`:

```c
// Clock del sistema = 8 MHz (HSI por defecto, sin PLL configurado)
// Queremos overflow cada 500 ms → toggle a 1 Hz

// 1. Habilitar clock de TIM2 (RCC_APB1ENR bit 0)
RCC_APB1ENR |= (1 << 0);

// 2. Prescaler: divide 8 MHz → 1 kHz
//    f_tim = f_clk / (PSC + 1)
//    PSC = 7999 → f_tim = 1 kHz
TIM2_PSC = 7999;

// 3. Auto-reload: overflow cada 500 ms
//    t = (ARR + 1) / f_tim
//    ARR = 499 → t = 500 ms
TIM2_ARR = 499;

// 4. Arrancar el timer
TIM2_CR1 |= (1 << 0);   // CEN = 1
```

### Actividad 11 — Implementar Blinky con timer

Completar el `while(1)` en `main()`:

```c
while (1) {
    // Esperar al flag de overflow (UIF = Update Interrupt Flag)
    if (TIM2_SR & (1 << 0)) {
        TIM2_SR &= ~(1 << 0);           // limpiar el flag
        GPIOC_ODR ^= (1 << 13);         // toggle PC13 (activo BAJO → invierte)
    }
    // La CPU no está bloqueada — puede hacer otras cosas aquí
}
```

Compilar, flashear y verificar que el LED parpadea:

```bash
make flash        # openocd + ST-Link v2
# o alternativamente:
make flash-stlink # st-flash directo
```

---

## Fase 3 — Medición y Observabilidad (≈ 15 min)

### Actividad 12 — Conectar el osciloscopio

1. Canal 1: probe en **PC13** (pin del LED, header derecho del Blue Pill), GND al GND de la placa
2. Escala temporal: **500 ms/div**
3. Trigger: flanco ascendente en CH1
4. Ajustar hasta ver una señal cuadrada estable

> ¿Es 50% duty cycle? ¿La señal tiene glitches?

### Actividad 13 — Confirmar frecuencia

Leer la frecuencia del display del osciloscopio (o medir el período manualmente).

- Frecuencia configurada: **1 Hz** (período = 1000 ms)
- Frecuencia medida: `___` Hz
- Período medido: `___` ms
- Delta: `δ = (f_medida - 1.0) / 1.0 × 100 = ___` %

> Un delta > 5% indica que el clock del MCU no está corriendo exactamente a 8 MHz. ¿Por qué? Investigar el HSI accuracy en el datasheet STM32F103C8.

### Actividad 14 — (Opcional / tarea) Map file

```bash
# Abrir blinky.map con cualquier editor de texto
grep -A5 "\.text\b" blinky.map
grep -A5 "\.bss"    blinky.map
```

Ubicar dónde están `.text`, `.data`, `.bss` en el espacio de memoria. ¿Coincide con `linker.ld`?

### Actividad 15 — (Opcional / tarea) Primer commit con evidencia

```bash
git add README.md
git add img/circuito_led.jpg       # foto del circuito armado
git add img/oscilo_blinky.jpg      # foto/screenshot del osciloscopio
git commit -m "meas(s01): blinky 1.02 Hz, delta +2%, circuito LED ok"
git push
```

Convención de commits del curso: `tipo(scope): descripción`
Tipos: `feat` · `fix` · `meas` · `docs` · `refactor`

### Actividad 16 — Cierre grupal

Completar la sección de evidencia al final de este README y hacer el commit final.

---

## Evidencia — Completar antes de salir

### Checklist eléctrico

- [ ] GND común verificado
- [ ] Polaridad verificada
- [ ] Límites de corriente respetados

### Mediciones

| Medición | Valor |
|----------|-------|
| V riel 3.3 V | |
| V riel 5 V | |
| Resistencia nominal | 1000 Ω |
| Resistencia medida | |
| V_R (caída en R del LED) | |
| V_LED (caída en LED) | |
| I real (calculada) | |
| Frecuencia configurada | 1 Hz |
| Frecuencia medida | |
| Delta (%) | |

### Fotos / capturas

- [ ] Foto del circuito LED+R en breadboard
- [ ] Captura del osciloscopio (señal cuadrada del blink)

---

## Referencias

- [RM0008 — STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) — Registros GPIO (cap. 9) y TIM2 (cap. 15)
- [STM32F103C8 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf) — Pinout, límites eléctricos
- [AN4013 — Introduction to timers for STM32 MCUs](https://www.st.com/resource/en/application_note/an4013-introduction-to-timers-for-stm32-mcus-stmicroelectronics.pdf)
- [GNU Arm Toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain)
- [OpenOCD — flashing STM32](https://openocd.org/doc/html/Flash-Commands.html)

---

*DM2 · IOR444 · Universidad de San Andrés · 2026*
