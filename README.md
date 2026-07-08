# Flash Memory

> An Arduino-based memory game inspired by Simon Says — built with C++ for the Arduino Uno.

## About

Flash Memory challenges players to watch a randomly generated sequence of colored LED flashes and recreate it by pressing the matching colored buttons in the correct order. The sequence grows longer with each level, testing both memory and focus.

A buzzer sounds with every LED flash during the sequence display, giving players an audio cue to count flashes by ear alongside the visual pattern.

---

##  Gameplay

1. **Watch** — colored LEDs flash a random sequence with an accompanying buzzer beep per flash
2. **Repeat** — press the matching colored buttons in the same order
3. **Progress** — complete the sequence correctly to advance to the next level
4. **Win** — complete all 5 levels

### Wrong Guess
- All 4 colored LEDs flash simultaneously
- Buzzer sounds to signal the incorrect answer
- A short delay prevents further input
- The same sequence replays automatically — unlimited attempts per level

### Replay Button
Forgot the sequence mid-guess? Press the dedicated replay button at any time during the input phase to rewatch the same sequence without generating a new one.

---

##  Levels

| Level | Sequence Length | Progress LED |
|-------|----------------|-------------|
| 1 | 2 flashes | ⚪ |
| 2 | 3 flashes | ⚪⚪ |
| 3 | 4 flashes | ⚪⚪⚪ |
| 4 | 5 flashes | ⚪⚪⚪⚪ |
| 5 | 6 flashes | ⚪⚪⚪⚪⚪ |

Five white LEDs connected to a 74HC595 shift register light up one by one as each level is completed, giving a persistent visual indicator of progress.

---

## Randomization

Every game generates a unique sequence using the Arduino `EEPROM` library as a persistent seed generator. On each power cycle, the stored seed is read, incremented, and written back, ensuring a different sequence every playthrough, even across power cycles.

---

## Hardware

| Component | Quantity | Notes |
|-----------|----------|-------|
| Arduino Uno | 1 | Microcontroller |
| Colored LEDs | 4 | Red, Green, Yellow, Blue |
| Colored buttons | 4 | Matching colors |
| White LEDs | 5 | Level progress indicator |
| 74HC595 shift register | 1 | Drives white LEDs + buzzer |
| Active buzzer | 1 | Connected to Q0 on shift register |
| 16x2 LCD display | 1 | Shows title and game state |
| Replay button | 1 | Replays current sequence |
| Resistors | Various | Current limiting for LEDs |
| Potentiometer | 1 | Controls LCD brightness

---

##  Pin Map

| Pin | Assignment |
|-----|-----------|
| `2` | LCD D7 |
| `3` | Blue button |
| `4` | Yellow button |
| `5` | Green button |
| `6` | Red button |
| `7` | 74HC595 Clock |
| `8` | 74HC595 Latch |
| `9` | 74HC595 Data |
| `10` | Red LED |
| `11` | Green LED |
| `12` | Yellow LED |
| `13` | Blue LED |
| `A0` | LCD RS |
| `A1` | LCD Enable |
| `A2` | LCD D4 |
| `A3` | LCD D5 |
| `A4` | LCD D6 |
| `A5` | Replay button |

### 74HC595 Output Map
| Bit | Assignment |
|-----|-----------|
| Q0 | Buzzer |
| Q1–Q5 | White LEDs 1–5 |
| Q6–Q7 | Unused |

---

## Libraries

```cpp
#include <LiquidCrystal.h>   // LCD display control
#include <EEPROM.h>           // Persistent seed storage for randomization
```

---

## Technical Notes

- Sequence timing uses **`millis()`** for non-blocking LED flashing — the Arduino remains responsive during the display phase
- **`delay()`** is used selectively to gate player input at key moments (wrong guess lockout, level transition pause)
- The buzzer and white LEDs share the same 74HC595 — all shift register writes preserve the full byte state to prevent unintentional outputs
- `EEPROM.get()` / `EEPROM.put()` store a 4-byte `unsigned long` seed (0 to ~4 billion) for maximum sequence variety

---

## Getting Started

1. Wire components according to the pin map above
2. Install the Arduino IDE
3. Open `FlashMemory.ino`
4. Select **Arduino Uno** as the board
5. Upload and play

---
