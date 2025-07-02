# Arduino Mega 2560 Memory Editor/Viewer

**Author/Owner:** INSCCOIN

---

## Overview
This program provides a powerful serial-based memory editor and viewer for the Arduino Mega 2560. It allows you to inspect, modify, search, and script operations on both RAM and EEPROM directly from the Serial Monitor. It is designed for advanced users, developers, and embedded systems enthusiasts who need low-level access to memory for debugging, testing, or educational purposes.

---

## Features
- **Read/Write RAM and EEPROM:**
  - 8/16/32-bit and float support
  - Multi-value and auto-increment operations
- **Flexible Output Formatting:**
  - Hexadecimal, decimal, and ASCII
- **Memory Fill/Clear:**
  - Fill RAM or EEPROM with a value
- **Memory Search:**
  - Search for 8/16/32-bit values in RAM or EEPROM
- **Scripting:**
  - Batch execution of multiple commands
- **Comprehensive Help System:**
  - Built-in help command listing all features and usage
- **Bounds Checking:**
  - EEPROM access is always checked for safety

---

## Usage

### Getting Started
1. Upload the sketch to your Arduino Mega 2560.
2. Open the Serial Monitor at 115200 baud.
3. Type commands as shown below and press Enter.

### Example Commands
See `MegaEditExamples.txt` for a full set of basic and advanced command examples.

#### Common Commands
- `r 0x200 16` — Read 16 bytes from RAM at address 0x200
- `w 0x200 0xAA 0xBB 0xCC` — Write bytes to RAM
- `e 0 16` — Read 16 bytes from EEPROM
- `ew 0 1 2 3 4` — Write bytes to EEPROM
- `r16 0x200 4` — Read 4 16-bit values from RAM
- `w16 0x210 0x1234` — Write 16-bit value to RAM
- `rf 0x400 2` — Read 2 floats from RAM
- `wf 0x230 3.14` — Write float to RAM
- `fill 0x400 32 0xFF` — Fill RAM with 0xFF
- `rsearch 0xAA` — Search RAM for byte value
- `format ascii` — Set output format to ASCII
- `script w 0x200 1 2 3; r 0x200 3` — Run a batch of commands
- `help` or `?` — Show help

---

## Command Reference

| Command | Description |
|---------|-------------|
| `r <addr> <len> [+]` | Read RAM (hexdump, auto-increment optional) |
| `w <addr> <val1> [val2 ...]` | Write bytes to RAM |
| `r16 <addr> <count> [+]` | Read 16-bit RAM values |
| `w16 <addr> <val> [+]` | Write 16-bit RAM value |
| `r32 <addr> <count> [+]` | Read 32-bit RAM values |
| `w32 <addr> <val> [+]` | Write 32-bit RAM value |
| `rf <addr> <count> [+]` | Read float(s) from RAM |
| `wf <addr> <float> [+]` | Write float to RAM |
| `e <addr> <len> [+]` | Read EEPROM (hexdump) |
| `ew <addr> <val1> [val2 ...]` | Write bytes to EEPROM |
| `e16 <addr> <count> [+]` | Read 16-bit EEPROM values |
| `ew16 <addr> <val> [+]` | Write 16-bit EEPROM value |
| `e32 <addr> <count> [+]` | Read 32-bit EEPROM values |
| `ew32 <addr> <val> [+]` | Write 32-bit EEPROM value |
| `ef <addr> <count> [+]` | Read float(s) from EEPROM |
| `ewf <addr> <float> [+]` | Write float to EEPROM |
| `fill <addr> <len> <val>` | Fill RAM with value |
| `efill <addr> <len> <val>` | Fill EEPROM with value |
| `rsearch <val>` | Search RAM for byte value |
| `esearch <val>` | Search EEPROM for byte value |
| `rsearch16 <val>` | Search RAM for 16-bit value |
| `esearch16 <val>` | Search EEPROM for 16-bit value |
| `rsearch32 <val>` | Search RAM for 32-bit value |
| `esearch32 <val>` | Search EEPROM for 32-bit value |
| `format hex|dec|ascii` | Set output format |
| `script <cmds;...>` | Run batch of commands separated by ';' |
| `help` or `?` | Show help |
| `+` (at end) | Auto-increment address |

---

## Advanced Usage
- See the "Advanced Examples" section in `MegaEditExamples.txt` for scripting, pattern search, and more complex workflows.

---

## Safety & Notes
- **EEPROM bounds are always checked.**
- **RAM access is direct and not bounds-checked:** Use with care to avoid overwriting critical memory.
- **Scripting** allows batch execution but does not support conditional logic or loops.
- **Output format** can be changed at any time for flexible viewing.

---

## License
GPL-3.0 license
Copyright (c) INSCCOIN 2025. All rights reserved.

---

## Contact
For questions, improvements, or contributions, contact INSCCOIN.
