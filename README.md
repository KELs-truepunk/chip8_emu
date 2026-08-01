# CHIP-8 Emulator

A lightweight **CHIP-8 interpreter** written in C using **SDL2** for graphics and input handling.

---

## 🛠 Features

* **Full CHIP-8 Opcode Support:** All 35 original instructions implemented (including $V_F$ collision flags, subroutines, and BCD decomposition).
* **60 Hz Timers:** Accurate implementation of `delay_timer` and `sound_timer`.
* **Configurable Scaling:** Custom window resolution scaling via command-line arguments.
* **Cross-Platform:** Builds natively on Linux (Arch/Ubuntu/etc.), macOS, and Windows via CMake.

---

## 🎮 Keypad Mapping

Original CHIP-8 hex keypad mapped to standard QWERTY keyboard layout:

| CHIP-8 Keypad | Keyboard Mapping |
| :---: | :---: |
| `1` `2` `3` `C` | `1` `2` `3` `4` |
| `4` `5` `6` `D` | `Q` `W` `E` `R` |
| `7` `8` `9` `E` | `A` `S` `D` `F` |
| `A` `0` `B` `F` | `Z` `X` `C` `V` |

---

## 🚀 Building & Running

### Requirements
* **C Compiler** (GCC, Clang, or MSVC)
* **CMake** $\ge$ 3.16
* **SDL2 Development Libraries**

### Build Instructions

```bash
# Clone the repository
git clone [https://github.com/your-username/chip8_emu.git](https://github.com/your-username/chip8_emu.git)
cd chip8_emu

# Create build directory and compile
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make