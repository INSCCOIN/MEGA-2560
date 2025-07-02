// Arduino Mega 2560 Memory Editor/Viewer
// Usage via Serial Monitor:
//   r <address> <length>   - Read memory
//   w <address> <value>    - Write memory
//   e <address> <length>   - Read EEPROM
//   ew <address> <value>   - Write EEPROM
#include <EEPROM.h>
#include <ctype.h>

// Output format: 0=hex, 1=dec, 2=ascii
uint8_t outputFormat = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("Memory Editor/Viewer Ready");
  Serial.println("Commands: r addr len | w addr val | e addr len | ew addr val");
}

void loop() {
  static String inputLine = "";
  while (Serial.available()) {
    char ch = Serial.read();
    // Handle end of line (user pressed Enter)
    if (ch == '\r' || ch == '\n') {
      if (inputLine.length() > 0) {
        Serial.println();
        String cmd = inputLine;
        inputLine = "";
        cmd.trim();
        if (cmd.length() == 0) return;
        // === Help command ===
        if (cmd == "help" || cmd == "?") {
          Serial.println("\n=== Memory Editor/Viewer Commands ===");
          Serial.println("r <addr> <len> [+]         - Read RAM (hexdump, auto-increment optional)");
          Serial.println("w <addr> <val1> [val2 ...] - Write bytes to RAM");
          Serial.println("r16 <addr> <count> [+]     - Read 16-bit RAM values");
          Serial.println("w16 <addr> <val> [+]       - Write 16-bit RAM value");
          Serial.println("r32 <addr> <count> [+]     - Read 32-bit RAM values");
          Serial.println("w32 <addr> <val> [+]       - Write 32-bit RAM value");
          Serial.println("rf <addr> <count> [+]      - Read float(s) from RAM");
          Serial.println("wf <addr> <float> [+]      - Write float to RAM");
          Serial.println("e <addr> <len> [+]         - Read EEPROM (hexdump)");
          Serial.println("ew <addr> <val1> [val2 ...]- Write bytes to EEPROM");
          Serial.println("e16 <addr> <count> [+]     - Read 16-bit EEPROM values");
          Serial.println("ew16 <addr> <val> [+]      - Write 16-bit EEPROM value");
          Serial.println("e32 <addr> <count> [+]     - Read 32-bit EEPROM values");
          Serial.println("ew32 <addr> <val> [+]      - Write 32-bit EEPROM value");
          Serial.println("ef <addr> <count> [+]      - Read float(s) from EEPROM");
          Serial.println("ewf <addr> <float> [+]     - Write float to EEPROM");
          Serial.println("fill <addr> <len> <val>    - Fill RAM with value");
          Serial.println("efill <addr> <len> <val>   - Fill EEPROM with value");
          Serial.println("rsearch <val>              - Search RAM for byte value");
          Serial.println("esearch <val>              - Search EEPROM for byte value");
          Serial.println("rsearch16 <val>            - Search RAM for 16-bit value");
          Serial.println("esearch16 <val>            - Search EEPROM for 16-bit value");
          Serial.println("rsearch32 <val>            - Search RAM for 32-bit value");
          Serial.println("esearch32 <val>            - Search EEPROM for 32-bit value");
          Serial.println("format hex|dec|ascii       - Set output format");
          Serial.println("script <cmds;...>          - Run batch of commands separated by ';'");
          Serial.println("help or ?                  - Show this help");
          Serial.println("+ (at end of command)      - Auto-increment address");
          Serial.println("========================================\n");
          return;
        }
        bool autoInc = false;
        if (cmd.endsWith("+")) {
          autoInc = true;
          cmd = cmd.substring(0, cmd.length() - 1);
          cmd.trim();
        }
        // Output format command
        if (cmd.startsWith("format ")) {
          String fmt = cmd.substring(7);
          fmt.trim();
          if (fmt == "hex") outputFormat = 0;
          else if (fmt == "dec") outputFormat = 1;
          else if (fmt == "ascii") outputFormat = 2;
          else Serial.println("Unknown format. Use: hex, dec, ascii");
          Serial.print("Output format set to: "); Serial.println(fmt);
          return;
        }
        // EEPROM search
        if (cmd.startsWith("esearch ")) {
          unsigned long val;
          if (sscanf(cmd.c_str(), "esearch %lu", &val) == 1) {
            bool found = false;
            for (int i = 0; i < EEPROM.length(); i++) {
              if (EEPROM.read(i) == (uint8_t)val) {
                Serial.print("Found at EEPROM[0x"); Serial.print(i, HEX); Serial.println("]");
                found = true;
              }
            }
            if (!found) Serial.println("Value not found in EEPROM.");
          } else {
            Serial.println("Usage: esearch <value>");
          }
          return;
        }
        // RAM search (search first 8KB by default)
        if (cmd.startsWith("rsearch ")) {
          unsigned long val;
          if (sscanf(cmd.c_str(), "rsearch %lu", &val) == 1) {
            bool found = false;
            unsigned long base = 0x200; // skip zero page
            unsigned long max = base + 8192;
            for (unsigned long i = base; i < max; i++) {
              if (*((uint8_t*)i) == (uint8_t)val) {
                Serial.print("Found at RAM[0x"); Serial.print(i, HEX); Serial.println("]");
                found = true;
              }
            }
            if (!found) Serial.println("Value not found in RAM (first 8KB).");
          } else {
            Serial.println("Usage: rsearch <value>");
          }
          return;
        }
        // Fill/clear commands
        if (cmd.startsWith("fill ")) {
          unsigned long addr, len, val;
          if (sscanf(cmd.c_str(), "fill %lx %lu %lu", &addr, &len, &val) == 3) {
            for (unsigned long i = 0; i < len; i++) *((uint8_t*)(addr + i)) = (uint8_t)val;
            Serial.print("Filled RAM[0x"); Serial.print(addr, HEX); Serial.print("] for "); Serial.print(len); Serial.print(" bytes with 0x"); Serial.println(val, HEX);
          } else {
            Serial.println("Usage: fill <address> <length> <value>");
          }
          return;
        } else if (cmd.startsWith("efill ")) {
          unsigned long addr, len, val;
          if (sscanf(cmd.c_str(), "efill %lx %lu %lu", &addr, &len, &val) == 3) {
            if (addr >= (unsigned)EEPROM.length() || (addr+len) > (unsigned)EEPROM.length()) {
              Serial.println("EEPROM access out of range!");
              return;
            }
            for (unsigned long i = 0; i < len; i++) EEPROM.write(addr + i, (uint8_t)val);
            Serial.print("Filled EEPROM[0x"); Serial.print(addr, HEX); Serial.print("] for "); Serial.print(len); Serial.print(" bytes with 0x"); Serial.println(val, HEX);
          } else {
            Serial.println("Usage: efill <address> <length> <value>");
          }
          return;
        }
        // Multi-value write: w <address> <val1> <val2> ...
        if (cmd.startsWith("w ")) {
          unsigned long addr;
          int n = sscanf(cmd.c_str(), "w %lx", &addr);
          if (n == 1) {
            int idx = cmd.indexOf(' ');
            idx = cmd.indexOf(' ', idx+1);
            if (idx < 0) {
              Serial.println("Usage: w <address> <val1> <val2> ...");
              return;
            }
            String vals = cmd.substring(idx+1);
            vals.trim();
            char *cstr = strdup(vals.c_str());
            char *token = strtok(cstr, " ");
            unsigned long a = addr;
            while (token) {
              uint8_t v = (uint8_t)strtoul(token, NULL, 0);
              *((uint8_t*)a) = v;
              Serial.print("Wrote "); Serial.print(v, HEX); Serial.print(" to RAM[0x"); Serial.print(a, HEX); Serial.println("]");
              a++;
              token = strtok(NULL, " ");
            }
            free(cstr);
            return;
          }
        }
        // Multi-value EEPROM write: ew <address> <val1> <val2> ...
        if (cmd.startsWith("ew ")) {
          unsigned long addr;
          int n = sscanf(cmd.c_str(), "ew %lx", &addr);
          if (n == 1) {
            int idx = cmd.indexOf(' ');
            idx = cmd.indexOf(' ', idx+1);
            if (idx < 0) {
              Serial.println("Usage: ew <address> <val1> <val2> ...");
              return;
            }
            String vals = cmd.substring(idx+1);
            vals.trim();
            char *cstr = strdup(vals.c_str());
            char *token = strtok(cstr, " ");
            unsigned long a = addr;
            while (token) {
              uint8_t v = (uint8_t)strtoul(token, NULL, 0);
              EEPROM.write(a, v);
              Serial.print("EEPROM["); Serial.print(a); Serial.print("] = "); Serial.println(v, HEX);
              a++;
              token = strtok(NULL, " ");
            }
            free(cstr);
            return;
          }
        }
        char c = cmd.charAt(0);
        // 16/32-bit RAM/EEPROM read/write
        if (cmd.startsWith("r16")) {
          unsigned long addr, count;
          if (sscanf(cmd.c_str(), "r16 %lx %lu", &addr, &count) == 2) {
            printMemory16(addr, count, autoInc);
          } else {
            Serial.println("Usage: r16 <address> <count> [+]");
          }
        } else if (cmd.startsWith("w16")) {
          unsigned long addr, val;
          if (sscanf(cmd.c_str(), "w16 %lx %lu", &addr, &val) == 2) {
            writeMemory16(addr, (uint16_t)val);
            if (autoInc) addr += 2;
          } else {
            Serial.println("Usage: w16 <address> <value> [+]");
          }
        } else if (cmd.startsWith("r32")) {
          unsigned long addr, count;
          if (sscanf(cmd.c_str(), "r32 %lx %lu", &addr, &count) == 2) {
            printMemory32(addr, count, autoInc);
          } else {
            Serial.println("Usage: r32 <address> <count> [+]");
          }
        } else if (cmd.startsWith("w32")) {
          unsigned long addr, val;
          if (sscanf(cmd.c_str(), "w32 %lx %lu", &addr, &val) == 2) {
            writeMemory32(addr, (uint32_t)val);
            if (autoInc) addr += 4;
          } else {
            Serial.println("Usage: w32 <address> <value> [+]");
          }
        } else if (cmd.startsWith("e16")) {
          unsigned long addr, count;
          if (sscanf(cmd.c_str(), "e16 %lx %lu", &addr, &count) == 2) {
            printEEPROM16(addr, count, autoInc);
          } else {
            Serial.println("Usage: e16 <address> <count> [+]");
          }
        } else if (cmd.startsWith("ew16")) {
          unsigned long addr, val;
          if (sscanf(cmd.c_str(), "ew16 %lx %lu", &addr, &val) == 2) {
            EEPROM.write(addr, val & 0xFF);
            EEPROM.write(addr+1, (val >> 8) & 0xFF);
            if (autoInc) addr += 2;
            Serial.print("EEPROM["); Serial.print(addr); Serial.print("] = "); Serial.println(val, HEX);
          } else {
            Serial.println("Usage: ew16 <address> <value> [+]");
          }
        } else if (cmd.startsWith("e32")) {
          unsigned long addr, count;
          if (sscanf(cmd.c_str(), "e32 %lx %lu", &addr, &count) == 2) {
            printEEPROM32(addr, count, autoInc);
          } else {
            Serial.println("Usage: e32 <address> <count> [+]");
          }
        } else if (cmd.startsWith("ew32")) {
          unsigned long addr, val;
          if (sscanf(cmd.c_str(), "ew32 %lx %lu", &addr, &val) == 2) {
            for (int i = 0; i < 4; i++) EEPROM.write(addr+i, (val >> (8*i)) & 0xFF);
            if (autoInc) addr += 4;
            Serial.print("EEPROM["); Serial.print(addr); Serial.print("] = "); Serial.println(val, HEX);
          } else {
            Serial.println("Usage: ew32 <address> <value> [+]");
          }
        } else if (cmd.startsWith("rf")) {
          unsigned long addr, count;
          if (sscanf(cmd.c_str(), "rf %lx %lu", &addr, &count) == 2) {
            printMemoryFloat(addr, count, autoInc);
          } else {
            Serial.println("Usage: rf <address> <count> [+]");
          }
        } else if (cmd.startsWith("wf")) {
          unsigned long addr;
          float val;
          if (sscanf(cmd.c_str(), "wf %lx %f", &addr, &val) == 2) {
            writeMemoryFloat(addr, val);
            if (autoInc) addr += 4;
          } else {
            Serial.println("Usage: wf <address> <float> [+]");
          }
        } else if (cmd.startsWith("ef ")) {
          unsigned long addr, count;
          if (sscanf(cmd.c_str(), "ef %lx %lu", &addr, &count) == 2) {
            printEEPROMFloat(addr, count, autoInc);
          } else {
            Serial.println("Usage: ef <address> <count> [+]");
          }
        } else if (cmd.startsWith("ewf")) {
          unsigned long addr;
          float val;
          if (sscanf(cmd.c_str(), "ewf %lx %f", &addr, &val) == 2) {
            writeEEPROMFloat(addr, val);
            if (autoInc) addr += 4;
          } else {
            Serial.println("Usage: ewf <address> <float> [+]");
          }
        } else if (cmd.startsWith("script ")) {
          String subscript = cmd.substring(7);
          runScript(subscript);
        } else {
          char c = cmd.charAt(0);
          if (c == 'r' || c == 'w' || c == 'e') {
            if (c == 'r') {
              unsigned long addr, len;
              if (sscanf(cmd.c_str(), "r %lx %lu", &addr, &len) == 2) {
                printMemoryHexdump(addr, len);
              } else {
                Serial.println("Usage: r <address> <length>");
              }
            } else if (c == 'w') {
              unsigned long addr, val;
              if (sscanf(cmd.c_str(), "w %lx %lu", &addr, &val) == 2) {
                writeMemory(addr, (uint8_t)val);
              } else {
                Serial.println("Usage: w <address> <value>");
              }
            } else if (c == 'e') {
              if (cmd.startsWith("ew")) {
                unsigned long addr, val;
                if (sscanf(cmd.c_str(), "ew %lx %lu", &addr, &val) == 2) {
                  EEPROM.write(addr, (uint8_t)val);
                  Serial.print("EEPROM["); Serial.print(addr); Serial.print("] = "); Serial.println(val, HEX);
                } else {
                  Serial.println("Usage: ew <address> <value>");
                }
              } else {
                unsigned long addr, len;
                if (sscanf(cmd.c_str(), "e %lx %lu", &addr, &len) == 2) {
                  printEEPROMHexdump(addr, len);
                } else {
                  Serial.println("Usage: e <address> <length>");
                }
              }
            }
          }
        } // Ignore any other input
      } else {
        // If user just pressed enter on empty line, just print newline
        Serial.println();
      }
    } else if (ch == 8 || ch == 127) { // Handle backspace
      if (inputLine.length() > 0) {
        inputLine.remove(inputLine.length() - 1);
        Serial.print("\b \b"); // Erase last char on terminal
      }
    } else if (isPrintable(ch)) {
      inputLine += ch;
      Serial.print(ch); // Echo character
    }
  }
}

void printMemory(unsigned long addr, unsigned long len) {
  Serial.print("RAM[0x"); Serial.print(addr, HEX); Serial.print("]: ");
  for (unsigned long i = 0; i < len; i++) {
    uint8_t val = *((uint8_t*)addr + i);
    Serial.print(val, HEX); Serial.print(" ");
  }
  Serial.println();
}

void writeMemory(unsigned long addr, uint8_t val) {
  *((uint8_t*)addr) = val;
  Serial.print("Wrote "); Serial.print(val, HEX); Serial.print(" to RAM[0x"); Serial.print(addr, HEX); Serial.println("]");
}

void printEEPROM(unsigned long addr, unsigned long len) {
  Serial.print("EEPROM[0x"); Serial.print(addr, HEX); Serial.print("]: ");
  for (unsigned long i = 0; i < len; i++) {
    uint8_t val = EEPROM.read(addr + i);
    Serial.print(val, HEX); Serial.print(" ");
  }
  Serial.println();
}

// Improved hexdump for RAM
void printMemoryHexdump(unsigned long addr, unsigned long len) {
  for (unsigned long i = 0; i < len; i += 16) {
    Serial.print("0x"); Serial.print(addr + i, HEX); Serial.print(": ");
    char ascii[17];
    for (unsigned long j = 0; j < 16 && (i + j) < len; j++) {
      uint8_t val = *((uint8_t*)(addr + i + j));
      if (val < 16) Serial.print("0");
      Serial.print(val, HEX); Serial.print(" ");
      ascii[j] = isprint(val) ? val : '.';
    }
    for (unsigned long j = len - i; j < 16; j++) Serial.print("   ");
    ascii[(len - i) < 16 ? (len - i) : 16] = 0;
    Serial.print("| "); Serial.println(ascii);
  }
}
// Improved hexdump for EEPROM
void printEEPROMHexdump(unsigned long addr, unsigned long len) {
  for (unsigned long i = 0; i < len; i += 16) {
    Serial.print("0x"); Serial.print(addr + i, HEX); Serial.print(": ");
    char ascii[17];
    for (unsigned long j = 0; j < 16 && (i + j) < len; j++) {
      uint8_t val = EEPROM.read(addr + i + j);
      if (val < 16) Serial.print("0");
      Serial.print(val, HEX); Serial.print(" ");
      ascii[j] = isprint(val) ? val : '.';
    }
    for (unsigned long j = len - i; j < 16; j++) Serial.print("   ");
    ascii[(len - i) < 16 ? (len - i) : 16] = 0;
    Serial.print("| "); Serial.println(ascii);
  }
}

// Helper for output formatting
void printValue(uint32_t val, uint8_t width) {
  if (outputFormat == 0) { // hex
    Serial.print("0x");
    for (int i = width * 2 - 1; i >= 0; i--) {
      Serial.print((val >> (i * 4)) & 0xF, HEX);
    }
  } else if (outputFormat == 1) { // dec
    Serial.print(val, DEC);
  } else if (outputFormat == 2) { // ascii
    for (int i = 0; i < width; i++) {
      char c = (val >> (i * 8)) & 0xFF;
      Serial.print(isprint(c) ? c : '.');
    }
  }
}

void printMemory16(unsigned long addr, unsigned long count, bool autoInc) {
  for (unsigned long i = 0; i < count; i++) {
    uint16_t val = *((uint16_t*)(addr));
    Serial.print("RAM16["); printValue(addr, 2); Serial.print("] = "); printValue(val, 2); Serial.println();
    if (autoInc) addr += 2;
  }
}
void writeMemory16(unsigned long addr, uint16_t val) {
  *((uint16_t*)addr) = val;
  Serial.print("Wrote 16-bit "); Serial.print(val, HEX); Serial.print(" to RAM[0x"); Serial.print(addr, HEX); Serial.println("]");
}
void printMemory32(unsigned long addr, unsigned long count, bool autoInc) {
  for (unsigned long i = 0; i < count; i++) {
    uint32_t val = *((uint32_t*)(addr));
    Serial.print("RAM32["); printValue(addr, 4); Serial.print("] = "); printValue(val, 4); Serial.println();
    if (autoInc) addr += 4;
  }
}
void writeMemory32(unsigned long addr, uint32_t val) {
  *((uint32_t*)addr) = val;
  Serial.print("Wrote 32-bit "); Serial.print(val, HEX); Serial.print(" to RAM[0x"); Serial.print(addr, HEX); Serial.println("]");
}
void printEEPROM16(unsigned long addr, unsigned long count, bool autoInc) {
  for (unsigned long i = 0; i < count; i++) {
    uint16_t val = EEPROM.read(addr) | (EEPROM.read(addr+1) << 8);
    Serial.print("EEPROM16["); printValue(addr, 2); Serial.print("] = "); printValue(val, 2); Serial.println();
    if (autoInc) addr += 2;
  }
}
void printEEPROM32(unsigned long addr, unsigned long count, bool autoInc) {
  for (unsigned long i = 0; i < count; i++) {
    uint32_t val = EEPROM.read(addr) | (EEPROM.read(addr+1) << 8) | (EEPROM.read(addr+2) << 16) | (EEPROM.read(addr+3) << 24);
    Serial.print("EEPROM32["); printValue(addr, 4); Serial.print("] = "); printValue(val, 4); Serial.println();
    if (autoInc) addr += 4;
  }
}
// 8-bit search helpers
void searchRAM8(uint8_t value, unsigned long start, unsigned long len) {
  bool found = false;
  for (unsigned long i = start; i < start+len; i++) {
    if (*((uint8_t*)i) == value) {
      Serial.print("Found at RAM["); printValue(i, 1); Serial.println("]");
      found = true;
    }
  }
  if (!found) Serial.println("Value not found in RAM.");
}
void searchEEPROM8(uint8_t value, unsigned long start, unsigned long len) {
  bool found = false;
  for (unsigned long i = start; i < start+len; i++) {
    if (EEPROM.read(i) == value) {
      Serial.print("Found at EEPROM["); printValue(i, 1); Serial.println("]");
      found = true;
    }
  }
  if (!found) Serial.println("Value not found in EEPROM.");
}
// 16/32-bit search helpers
void searchRAM16(uint16_t value, unsigned long start, unsigned long len) {
  bool found = false;
  for (unsigned long i = start; i < start+len; i += 2) {
    if (*((uint16_t*)i) == value) {
      Serial.print("Found at RAM["); printValue(i, 2); Serial.println("]");
      found = true;
    }
  }
  if (!found) Serial.println("16-bit value not found in RAM.");
}
void searchEEPROM16(uint16_t value, unsigned long start, unsigned long len) {
  bool found = false;
  for (unsigned long i = start; i < start+len; i += 2) {
    uint16_t v = EEPROM.read(i) | (EEPROM.read(i+1) << 8);
    if (v == value) {
      Serial.print("Found at EEPROM["); printValue(i, 2); Serial.println("]");
      found = true;
    }
  }
  if (!found) Serial.println("16-bit value not found in EEPROM.");
}
void searchRAM32(uint32_t value, unsigned long start, unsigned long len) {
  bool found = false;
  for (unsigned long i = start; i < start+len; i += 4) {
    if (*((uint32_t*)i) == value) {
      Serial.print("Found at RAM["); printValue(i, 4); Serial.println("]");
      found = true;
    }
  }
  if (!found) Serial.println("32-bit value not found in RAM.");
}
void searchEEPROM32(uint32_t value, unsigned long start, unsigned long len) {
  bool found = false;
  for (unsigned long i = start; i < start+len; i += 4) {
    uint32_t v = EEPROM.read(i) | (EEPROM.read(i+1) << 8) | (EEPROM.read(i+2) << 16) | (EEPROM.read(i+3) << 24);
    if (v == value) {
      Serial.print("Found at EEPROM["); printValue(i, 4); Serial.println("]");
      found = true;
    }
  }
  if (!found) Serial.println("32-bit value not found in EEPROM.");
}
// Float helpers
void printMemoryFloat(unsigned long addr, unsigned long count, bool autoInc) {
  for (unsigned long i = 0; i < count; i++) {
    float val = *((float*)(addr));
    Serial.print("RAMF["); printValue(addr, 4); Serial.print("] = "); Serial.println(val, 6);
    if (autoInc) addr += 4;
  }
}
void writeMemoryFloat(unsigned long addr, float val) {
  *((float*)addr) = val;
  Serial.print("Wrote float "); Serial.print(val, 6); Serial.print(" to RAM["); printValue(addr, 4); Serial.println("]");
}
void printEEPROMFloat(unsigned long addr, unsigned long count, bool autoInc) {
  union { uint8_t b[4]; float f; } u;
  for (unsigned long i = 0; i < count; i++) {
    for (int j = 0; j < 4; j++) u.b[j] = EEPROM.read(addr + j);
    Serial.print("EEPROMF["); printValue(addr, 4); Serial.print("] = "); Serial.println(u.f, 6);
    if (autoInc) addr += 4;
  }
}
void writeEEPROMFloat(unsigned long addr, float val) {
  union { uint8_t b[4]; float f; } u;
  u.f = val;
  for (int j = 0; j < 4; j++) EEPROM.write(addr + j, u.b[j]);
  Serial.print("Wrote float "); Serial.print(val, 6); Serial.print(" to EEPROM["); printValue(addr, 4); Serial.println("]");
}
// Scripting support: run a sequence of commands separated by ';'
void runScript(String script) {
  int start = 0;
  while (start < script.length()) {
    int end = script.indexOf(';', start);
    if (end == -1) end = script.length();
    String line = script.substring(start, end);
    line.trim();
    if (line.length() > 0) {
      Serial.print("> "); Serial.println(line);
      // Simulate user input for each command by directly parsing the command string
      // This duplicates the main command parser logic for each script line
      bool autoInc = false;
      if (line.endsWith("+")) {
        autoInc = true;
        line = line.substring(0, line.length() - 1);
        line.trim();
      }
      // Copy of the main command parser logic, but using 'line' instead of 'cmd'
      // Output format command
      if (line.startsWith("format ")) {
        String fmt = line.substring(7);
        fmt.trim();
        if (fmt == "hex") outputFormat = 0;
        else if (fmt == "dec") outputFormat = 1;
        else if (fmt == "ascii") outputFormat = 2;
        else Serial.println("Unknown format. Use: hex, dec, ascii");
        Serial.print("Output format set to: "); Serial.println(fmt);
      } else if (line.startsWith("esearch ")) {
        unsigned long val;
        if (sscanf(line.c_str(), "esearch %lu", &val) == 1) {
          bool found = false;
          for (int i = 0; i < EEPROM.length(); i++) {
            if (EEPROM.read(i) == (uint8_t)val) {
              Serial.print("Found at EEPROM[0x"); Serial.print(i, HEX); Serial.println("]");
              found = true;
            }
          }
          if (!found) Serial.println("Value not found in EEPROM.");
        } else {
          Serial.println("Usage: esearch <value>");
        }
      } else if (line.startsWith("rsearch ")) {
        unsigned long val;
        if (sscanf(line.c_str(), "rsearch %lu", &val) == 1) {
          bool found = false;
          unsigned long base = 0x200;
          unsigned long max = base + 8192;
          for (unsigned long i = base; i < max; i++) {
            if (*((uint8_t*)i) == (uint8_t)val) {
              Serial.print("Found at RAM[0x"); Serial.print(i, HEX); Serial.println("]");
              found = true;
            }
          }
          if (!found) Serial.println("Value not found in RAM (first 8KB).");
        } else {
          Serial.println("Usage: rsearch <value>");
        }
      } else if (line.startsWith("fill ")) {
        unsigned long addr, len, val;
        if (sscanf(line.c_str(), "fill %lx %lu %lu", &addr, &len, &val) == 3) {
          for (unsigned long i = 0; i < len; i++) *((uint8_t*)(addr + i)) = (uint8_t)val;
          Serial.print("Filled RAM[0x"); Serial.print(addr, HEX); Serial.print("] for "); Serial.print(len); Serial.print(" bytes with 0x"); Serial.println(val, HEX);
        } else {
          Serial.println("Usage: fill <address> <length> <value>");
        }
      } else if (line.startsWith("efill ")) {
        unsigned long addr, len, val;
        if (sscanf(line.c_str(), "efill %lx %lu %lu", &addr, &len, &val) == 3) {
          if (addr >= (unsigned)EEPROM.length() || (addr+len) > (unsigned)EEPROM.length()) {
            Serial.println("EEPROM access out of range!");
          } else {
            for (unsigned long i = 0; i < len; i++) EEPROM.write(addr + i, (uint8_t)val);
            Serial.print("Filled EEPROM[0x"); Serial.print(addr, HEX); Serial.print("] for "); Serial.print(len); Serial.print(" bytes with 0x"); Serial.println(val, HEX);
          }
        } else {
          Serial.println("Usage: efill <address> <length> <value>");
        }
      } else if (line.startsWith("w ")) {
        unsigned long addr;
        int n = sscanf(line.c_str(), "w %lx", &addr);
        if (n == 1) {
          int idx = line.indexOf(' ');
          idx = line.indexOf(' ', idx+1);
          if (idx < 0) {
            Serial.println("Usage: w <address> <val1> <val2> ...");
          } else {
            String vals = line.substring(idx+1);
            vals.trim();
            char *cstr = strdup(vals.c_str());
            char *token = strtok(cstr, " ");
            unsigned long a = addr;
            while (token) {
              uint8_t v = (uint8_t)strtoul(token, NULL, 0);
              *((uint8_t*)a) = v;
              Serial.print("Wrote "); Serial.print(v, HEX); Serial.print(" to RAM[0x"); Serial.print(a, HEX); Serial.println("]");
              a++;
              token = strtok(NULL, " ");
            }
            free(cstr);
          }
        }
      } else if (line.startsWith("ew ")) {
        unsigned long addr;
        int n = sscanf(line.c_str(), "ew %lx", &addr);
        if (n == 1) {
          int idx = line.indexOf(' ');
          idx = line.indexOf(' ', idx+1);
          if (idx < 0) {
            Serial.println("Usage: ew <address> <val1> <val2> ...");
          } else {
            String vals = line.substring(idx+1);
            vals.trim();
            char *cstr = strdup(vals.c_str());
            char *token = strtok(cstr, " ");
            unsigned long a = addr;
            while (token) {
              uint8_t v = (uint8_t)strtoul(token, NULL, 0);
              EEPROM.write(a, v);
              Serial.print("EEPROM["); Serial.print(a); Serial.print("] = "); Serial.println(v, HEX);
              a++;
              token = strtok(NULL, " ");
            }
            free(cstr);
          }
        }
      } else if (line.startsWith("r16")) {
        unsigned long addr, count;
        if (sscanf(line.c_str(), "r16 %lx %lu", &addr, &count) == 2) {
          printMemory16(addr, count, autoInc);
        } else {
          Serial.println("Usage: r16 <address> <count> [+]");
        }
      } else if (line.startsWith("w16")) {
        unsigned long addr, val;
        if (sscanf(line.c_str(), "w16 %lx %lu", &addr, &val) == 2) {
          writeMemory16(addr, (uint16_t)val);
          if (autoInc) addr += 2;
        } else {
          Serial.println("Usage: w16 <address> <value> [+]");
        }
      } else if (line.startsWith("r32")) {
        unsigned long addr, count;
        if (sscanf(line.c_str(), "r32 %lx %lu", &addr, &count) == 2) {
          printMemory32(addr, count, autoInc);
        } else {
          Serial.println("Usage: r32 <address> <count> [+]");
        }
      } else if (line.startsWith("w32")) {
        unsigned long addr, val;
        if (sscanf(line.c_str(), "w32 %lx %lu", &addr, &val) == 2) {
          writeMemory32(addr, (uint32_t)val);
          if (autoInc) addr += 4;
        } else {
          Serial.println("Usage: w32 <address> <value> [+]");
        }
      } else if (line.startsWith("e16")) {
        unsigned long addr, count;
        if (sscanf(line.c_str(), "e16 %lx %lu", &addr, &count) == 2) {
          printEEPROM16(addr, count, autoInc);
        } else {
          Serial.println("Usage: e16 <address> <count> [+]");
        }
      } else if (line.startsWith("ew16")) {
        unsigned long addr, val;
        if (sscanf(line.c_str(), "ew16 %lx %lu", &addr, &val) == 2) {
          EEPROM.write(addr, val & 0xFF);
          EEPROM.write(addr+1, (val >> 8) & 0xFF);
          if (autoInc) addr += 2;
          Serial.print("EEPROM["); Serial.print(addr); Serial.print("] = "); Serial.println(val, HEX);
        } else {
          Serial.println("Usage: ew16 <address> <value> [+]");
        }
      } else if (line.startsWith("e32")) {
        unsigned long addr, count;
        if (sscanf(line.c_str(), "e32 %lx %lu", &addr, &count) == 2) {
          printEEPROM32(addr, count, autoInc);
        } else {
          Serial.println("Usage: e32 <address> <count> [+]");
        }
      } else if (line.startsWith("ew32")) {
        unsigned long addr, val;
        if (sscanf(line.c_str(), "ew32 %lx %lu", &addr, &val) == 2) {
          for (int i = 0; i < 4; i++) EEPROM.write(addr+i, (val >> (8*i)) & 0xFF);
          if (autoInc) addr += 4;
          Serial.print("EEPROM["); Serial.print(addr); Serial.print("] = "); Serial.println(val, HEX);
        } else {
          Serial.println("Usage: ew32 <address> <value> [+]");
        }
      } else if (line.startsWith("rf")) {
        unsigned long addr, count;
        if (sscanf(line.c_str(), "rf %lx %lu", &addr, &count) == 2) {
          printMemoryFloat(addr, count, autoInc);
        } else {
          Serial.println("Usage: rf <address> <count> [+]");
        }
      } else if (line.startsWith("wf")) {
        unsigned long addr;
        float val;
        if (sscanf(line.c_str(), "wf %lx %f", &addr, &val) == 2) {
          writeMemoryFloat(addr, val);
          if (autoInc) addr += 4;
        } else {
          Serial.println("Usage: wf <address> <float> [+]");
        }
      } else if (line.startsWith("ef ")) {
        unsigned long addr, count;
        if (sscanf(line.c_str(), "ef %lx %lu", &addr, &count) == 2) {
          printEEPROMFloat(addr, count, autoInc);
        } else {
          Serial.println("Usage: ef <address> <count> [+]");
        }
      } else if (line.startsWith("ewf")) {
        unsigned long addr;
        float val;
        if (sscanf(line.c_str(), "ewf %lx %f", &addr, &val) == 2) {
          writeEEPROMFloat(addr, val);
          if (autoInc) addr += 4;
        } else {
          Serial.println("Usage: ewf <address> <float> [+]");
        }
      } else if (line.startsWith("script ")) {
        String subscript = line.substring(7);
        runScript(subscript);
      } else {
        char c = line.charAt(0);
        if (c == 'r' || c == 'w' || c == 'e') {
          if (c == 'r') {
            unsigned long addr, len;
            if (sscanf(line.c_str(), "r %lx %lu", &addr, &len) == 2) {
              printMemoryHexdump(addr, len);
            } else {
              Serial.println("Usage: r <address> <length>");
            }
          } else if (c == 'w') {
            unsigned long addr, val;
            if (sscanf(line.c_str(), "w %lx %lu", &addr, &val) == 2) {
              writeMemory(addr, (uint8_t)val);
            } else {
              Serial.println("Usage: w <address> <value>");
            }
          } else if (c == 'e') {
            if (line.startsWith("ew")) {
              unsigned long addr, val;
              if (sscanf(line.c_str(), "ew %lx %lu", &addr, &val) == 2) {
                EEPROM.write(addr, (uint8_t)val);
                Serial.print("EEPROM["); Serial.print(addr); Serial.print("] = "); Serial.println(val, HEX);
              } else {
                Serial.println("Usage: ew <address> <value>");
              }
            } else {
              unsigned long addr, len;
              if (sscanf(line.c_str(), "e %lx %lu", &addr, &len) == 2) {
                printEEPROMHexdump(addr, len);
              } else {
                Serial.println("Usage: e <address> <length>");
              }
            }
          }
        }
      }
    }
    start = end + 1;
  }
}