<div align="center">

<br/>

```
 _____ ____  _____ _____ ____  _____ ___  ____
|  ___|  _ \| ____| ____|  _ \|_   _/ _ \/ ___|
| |_  | |_) |  _| |  _| | |_) | || | | | \___ \
|  _| |  _ <| |___| |___|  _ <  | || |_| |___) |
|_|   |_| \_\_____|_____|_| \_\ |_| \___/|____/
```

<h1>LPC2138 · FreeRTOS · Semaphore & Mutex Demo</h1>

<p><strong>Real concurrency on real hardware.</strong><br/>
Three buttons. Three tasks. One semaphore pool. Zero race conditions.</p>

<br/>

[![FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-00d084?style=for-the-badge&logoColor=white)](https://www.freertos.org)
[![Platform](https://img.shields.io/badge/MCU-LPC2138-0078d7?style=for-the-badge)](https://www.nxp.com)
[![Core](https://img.shields.io/badge/Core-ARM7TDMI-ff6b35?style=for-the-badge)](https://developer.arm.com)
[![Language](https://img.shields.io/badge/Language-C-a8b9cc?style=for-the-badge&logo=c&logoColor=white)]()
[![License](https://img.shields.io/badge/License-Open%20Source-9b59b6?style=for-the-badge)]()
[![Status](https://img.shields.io/badge/Status-Active-success?style=for-the-badge)]()
[![Simulation](https://img.shields.io/badge/Simulation-Proteus-e74c3c?style=for-the-badge)]()

<br/>

</div>

---

## 🧭 Table of Contents

- [What This Is](#-what-this-is)
- [How It Works](#-how-it-works)
- [Architecture](#-architecture)
- [Schematic](#-schematic)
- [Hardware Connections](#-hardware-connections)
- [UART Configuration](#-uart-configuration)
- [Build & Flash](#️-build--flash)
- [Full Source Code](#-full-source-code)
- [Repository Structure](#-repository-structure)
- [License](#-license)
- [Author](#-author)

---

## 🎯 What This Is

A **bare-metal embedded systems demo** that brings RTOS synchronization primitives to life using physical hardware — not diagrams, not slides, real buttons and a real serial terminal.

Running **FreeRTOS** on the **NXP LPC2138** (ARM7TDMI), it demonstrates:

| Primitive | Handle | Purpose |
|---|---|---|
| Counting Semaphore | `m` | Guards a pool of 5 resource tokens |
| Mutex | `m2` | Prevents interleaved UART output |

> Push **Switch 1 or 3** → a task **takes** a token → reads shared data → prints count.  
> Push **Switch 2** → a task **gives** a token back → count rises.  
> The **mutex** keeps every print atomic — no garbled output, ever.

---

## 🧠 How It Works

```
┌─────────────────────────────────────────────────────────┐
│                     Shared State                        │
│                                                         │
│   int a[5] = { 8, 3, 5, 6, 7 }   ← shared resource    │
│   int index = 0                   ← cyclic pointer     │
│                                                         │
│   SemaphoreHandle_t m             ← counting sem (0–5) │
│   SemaphoreHandle_t m2            ← mutex (UART guard) │
└─────────────────────────────────────────────────────────┘
```

**When you press a TAKE button (SW1 or SW3):**

```
1. Task polls GPIO pin → detects HIGH
2. Reads current semaphore count (before)
3. xSemaphoreTake(m, 1000)  → acquires token, count drops by 1
4. xSemaphoreTake(m2, 3000) → locks UART (mutex)
5. Prints: array value · before count · after count
6. xSemaphoreGive(m2)       → releases UART
7. Waits for button release  (debounce)
```

**When you press the GIVE button (SW2):**

```
1. Task polls GPIO pin → detects HIGH
2. xSemaphoreTake(m2, 3000) → locks UART
3. Reads count before
4. xSemaphoreGive(m)        → returns token, count rises by 1
5. Reads count after, prints both
6. Waits for button release
7. xSemaphoreGive(m2)       → releases UART
```

> ⚠️ If the semaphore count is already **0**, `xSemaphoreTake()` blocks for up to **1000 ticks** before timing out.  
> ⚠️ If the pool is **full (5)**, `xSemaphoreGive()` is silently ignored by FreeRTOS.

---

## 📐 Architecture

```
                     ┌──────────────────────────────────┐
                     │          FreeRTOS Kernel          │
                     └────────────────┬─────────────────┘
                                      │ vTaskStartScheduler()
              ┌───────────────────────┼───────────────────────┐
              │                       │                       │
      ┌───────▼───────┐       ┌───────▼───────┐       ┌───────▼───────┐
      │    Task 1     │       │    Task 2     │       │    Task 3     │
      │   lcd1()      │       │   lcd2()      │       │   lcd3()      │
      │   Priority 0  │       │   Priority 0  │       │   Priority 0  │
      └───────┬───────┘       └───────┬───────┘       └───────┬───────┘
              │ SW1 P0.12             │ SW2 P0.14             │ SW3 P0.13
              │ Take(m)               │ Give(m)               │ Take(m)
              └───────────────────────┼───────────────────────┘
                                      │
                         ┌────────────▼────────────┐
                         │  Counting Semaphore (m)  │
                         │   ● ● ● ● ●  count=5    │
                         │   max=5 / initial=5      │
                         └────────────┬────────────┘
                                      │
                         ┌────────────▼────────────┐
                         │        Mutex (m2)        │
                         │   UART0 critical section │
                         │   P0.0 TX → Terminal     │
                         └─────────────────────────┘
```

---

## 💡 Schematic

<p align="center">
  <img src="./schematic.png" alt="Circuit Schematic" width="700"/>
  <br/><em>Proteus simulation — buttons with pull-down resistors and UART virtual terminal</em>
</p>

---

## ⚡ Hardware Connections

| Component | LPC2138 Pin | Direction | Description |
|---|---|---|---|
| Switch 1 — TAKE | `P0.12` | INPUT | Task 1 takes a semaphore token |
| Switch 3 — TAKE | `P0.13` | INPUT | Task 3 takes a semaphore token |
| Switch 2 — GIVE | `P0.14` | INPUT | Returns a token to the pool |
| UART0 TX | `P0.0` | OUTPUT | Serial output to virtual terminal |
| UART0 RX | `P0.1` | INPUT | Optional — not used in this demo |
| Pull-down resistors | All SW pins | — | Keeps lines LOW when buttons open |

> All GPIO pins are configured as inputs via `IO0DIR = 0`.  
> `PINSEL` registers route `P0.0/P0.1` to UART0; all others remain GPIO.

---

## 🔧 UART Configuration

```c
PINSEL0 = 1 | 1 << 2;  // Route P0.0→TX, P0.1→RX to UART0

U0LCR = 0x83;           // Set DLAB=1 to access baud rate divisors
U0DLL = 98;             // DLL = 98  →  9600 baud @ PCLK 15 MHz
U0DLM = 0;              // DLM = 0   (divisor fits in low byte)
U0LCR = 0x03;           // Clear DLAB | 8-bit | No parity | 1 stop
```

| Parameter | Value |
|---|---|
| Baud rate | 9600 bps |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| PCLK | 15 MHz |
| Divisor (U0DLL) | 98 |

---

## ⚙️ Build & Flash

### 1 — Hardware Setup

```
LPC2138 Board
├── P0.12 ──[SW1]── VCC     (with 10kΩ pull-down to GND)
├── P0.13 ──[SW3]── VCC     (with 10kΩ pull-down to GND)
├── P0.14 ──[SW2]── VCC     (with 10kΩ pull-down to GND)
└── P0.0  ──────────────── USB-UART adapter or virtual terminal
```

### 2 — Compile with Keil µVision

1. Clone this repository
2. Open the project in **Keil µVision 4 or 5**
3. Confirm FreeRTOS port files for LPC2138 are in the project tree
4. Set the target device to **LPC2138** with correct PCLK settings
5. Build → `counting.c`

### 3 — Simulate in Proteus

1. Open `schematic.pdsprj`
2. Load the compiled `.hex` into the LPC2138 component
3. Add a **Virtual Terminal** connected to UART0 TX (`P0.0`)
4. Run the simulation — click the buttons and watch the terminal

### Expected Terminal Output

```
Successfully Created Counting Semaphore
Successfully Created Mutex

Array Value is 8
In Take
Before Taken - 5
After Taken - 4

Array Value is 3
In Take
Before Taken - 4
After Taken - 3

In give
Before Given - 3
After Given - 4
```

---

## 💻 Full Source Code

<details>
<summary><strong>📄 counting.c — click to expand</strong></summary>

```c
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ─── Shared resource ─────────────────────────────────── */
int a[5]  = {8, 3, 5, 6, 7};
int index = 0;

/* ─── Synchronization handles ─────────────────────────── */
SemaphoreHandle_t m;   /* counting semaphore — max 5      */
SemaphoreHandle_t m2;  /* mutex — guards UART output      */

/* ─── Forward declarations ────────────────────────────── */
void lcd1(void *parm);
void lcd2(void *parm);
void lcd3(void *parm);
void display(const char *s);
void trans(char c);

/* ─── Entry point ─────────────────────────────────────── */
int main(void)
{
    /* UART0 init — 9600 baud @ PCLK 15 MHz */
    PINSEL0 = 1 | 1 << 2;
    U0LCR   = 0x83;
    U0DLL   = 98;
    U0DLM   = 0;
    U0LCR   = 0x03;

    /* GPIO: all P0 pins as input */
    PINSEL1 = 0;
    IO0DIR  = 0;
    IO1DIR  = ~0;

    /* Create counting semaphore (max=5, initial=5) */
    m = xSemaphoreCreateCounting(5, 5);
    display(m == NULL
        ? "Failed To Create Counting Semaphore\r\n"
        : "Successfully Created Counting Semaphore\r\n");

    /* Create mutex for UART protection */
    m2 = xSemaphoreCreateMutex();
    display(m2 == NULL
        ? "Failed To Create Mutex\r\n"
        : "Successfully Created Mutex\r\n");

    /* Launch all three tasks at the same priority */
    xTaskCreate(lcd1, "Task1", 90, NULL, 0, NULL);
    xTaskCreate(lcd2, "Task2", 90, NULL, 0, NULL);
    xTaskCreate(lcd3, "Task3", 90, NULL, 0, NULL);

    vTaskStartScheduler();
    while (1);
}

/* ─── UART helpers ────────────────────────────────────── */
void trans(char a)
{
    while ((U0LSR & (1 << 5)) == 0);  /* poll until TX FIFO ready */
    U0THR = a;
}

void display(const char *a)
{
    while (*a) trans(*a++);
}

/* ─── Task 1 : SW1 / P0.12 — TAKE ────────────────────── */
void lcd1(void *parm)
{
    char b;
    while (1)
    {
        if ((IO0PIN & (1 << 12)) == (1 << 12))
        {
            b = uxSemaphoreGetCount(m);
            if (xSemaphoreTake(m, 1000) == 1)
            {
                if (xSemaphoreTake(m2, 3000) == 1)
                {
                    display("\r\nArray Value is ");
                    trans(a[index++] + 48);
                    if (index == 5) index = 0;
                    display("\rIn Take");
                    display("\rBefore Taken - "); trans(b + 48);
                    b = uxSemaphoreGetCount(m);
                    display("\rAfter Taken - ");  trans(b + 48);
                    xSemaphoreGive(m2);
                }
                while ((IO0PIN & (1 << 12)) == (1 << 12));  /* debounce */
            }
        }
    }
}

/* ─── Task 3 : SW3 / P0.13 — TAKE ────────────────────── */
void lcd3(void *parm)
{
    char b;
    while (1)
    {
        if ((IO0PIN & (1 << 13)) == (1 << 13))
        {
            b = uxSemaphoreGetCount(m);
            if (xSemaphoreTake(m, 1000) == 1)
            {
                if (xSemaphoreTake(m2, 3000) == 1)
                {
                    display("\r\nArray Value is ");
                    trans(a[index++] + 48);
                    if (index == 5) index = 0;
                    display("\rIn Take");
                    display("\rBefore Taken - "); trans(b + 48);
                    b = uxSemaphoreGetCount(m);
                    display("\rAfter Taken - ");  trans(b + 48);
                    xSemaphoreGive(m2);
                }
                while ((IO0PIN & (1 << 13)) == (1 << 13));  /* debounce */
            }
        }
    }
}

/* ─── Task 2 : SW2 / P0.14 — GIVE ────────────────────── */
void lcd2(void *parm)
{
    char b;
    while (1)
    {
        if ((IO0PIN & (1 << 14)) == (1 << 14))
        {
            if (xSemaphoreTake(m2, 3000) == 1)
            {
                b = uxSemaphoreGetCount(m);
                display("\rIn give");
                display("\rBefore Given - "); trans(b + 48);
                xSemaphoreGive(m);
                b = uxSemaphoreGetCount(m);
                display("\rAfter Given - ");  trans(b + 48);
                while ((IO0PIN & (1 << 14)) == (1 << 14));  /* debounce */
                xSemaphoreGive(m2);
            }
        }
    }
}
```

</details>

---

## 📁 Repository Structure

```
lpc2138-freertos-semaphore/
│
├── counting.c          ← Full application source
├── schematic.png       ← Proteus circuit diagram
├── schematic.pdsprj    ← Proteus project file
└── README.md           ← You are here
```

---

## 📜 License

This project is **open-source** and freely available for educational and research purposes.  
If you build on it or use it in your coursework — a ⭐ star or a credit is always appreciated!

---

## 👩‍💻 Author

<div align="center">

**Ashika K**

[![GitHub](https://img.shields.io/badge/GitHub-Ashika005-181717?style=for-the-badge&logo=github)](https://github.com/Ashika005)

<br/>

*If this demo helped you understand RTOS concurrency or saved hours of debugging,*  
*hit that* ⭐ *Star button — and share it with your classmates!*

<br/>

---

*Happy embedded coding!* 🚀

</div>
