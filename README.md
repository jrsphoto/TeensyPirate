# TeensyPirate

A Capture-The-Flag style hardware hacking training game for Bus Pirate 5 and Teensy 3.1.

## Overview

TeensyPirate is an educational tool that teaches hardware reverse engineering and protocol analysis through progressive challenges. Students use a Bus Pirate 5 to discover hidden keys, complete byte sequences, solve multi-protocol puzzles, and unlock virtual vaults by analyzing I2C, UART, and GPIO communications.

## Features

- **Progressive Difficulty**: 4 levels from basic I2C reads to multi-protocol challenges
- **Real Hardware Skills**: I2C device enumeration, register manipulation, protocol analysis
- **Customizable**: Easy-to-change keys, passwords, and difficulty settings
- **Educational**: Designed for classroom training, CTF competitions, and workshops
- **Realistic**: Simulates real hardware hacking scenarios with multiple protocols

## Game Levels

**Level 0:** Discover and submit a hidden 16-bit key via I2C  
**Level 1:** Complete a magic byte sequence in correct order  
**Level 2:** Multi-protocol challenge (UART password + GPIO pulses)  
**Level 3:** Unlock the vault and extract the final flag  

New I2C devices appear on the bus as students progress (vault at 0x55, secret at 0x66).

## Hardware Requirements

- **Bus Pirate 5** - Hardware hacking tool
- **Teensy 3.1** - Target device running the game
- **Wiring**: 3 connections (SDA, SCL, GND)
- **Optional**: Breadboard, jumper wires

## Software Requirements

- Arduino IDE (1.8.x or 2.x)
- Teensyduino add-on
- i2c_t3 library (included with Teensyduino)

## Quick Start

1. **Upload the code:**
   - Open `buspirate_ctf_game.ino` in Arduino IDE
   - Select Board: Teensy 3.1
   - Upload to Teensy

2. **Connect hardware:**
   ```
   Bus Pirate 5    →    Teensy 3.1
   ───────────────────────────────
   SDA             →    Pin 18
   SCL             →    Pin 19
   GND             →    GND
   ```

3. **Start hacking:**
   - See [STUDENT_GUIDE.md](STUDENT_GUIDE.md) for gameplay instructions
   - See [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md) for detailed setup

## Documentation

- **[STUDENT_GUIDE.md](STUDENT_GUIDE.md)** - Complete challenge guide
- **[SOLUTIONS.md](SOLUTIONS.md)** - Complete walkthrough
- **[SETUP_CHECKLIST.md](SETUP_CHECKLIST.md)** - Step-by-step setup verification

## Customization

Edit the top of `buspirate_ctf_game.ino` to customize:

```cpp
// Change keys
#define KEY_LEVEL1 0xDEAD
#define KEY_LEVEL2 0xBEEF

// Change passwords
const char UART_PASSWORD[] = "HACKTHEPLANET";

// Adjust difficulty
#define GPIO_PULSE_COUNT_REQUIRED 10
```

## Bus Pirate 5 Quick Commands

```
HiZ> m              # Mode menu
Mode> 5             # I2C mode
I2C> scan           # Find devices
I2C> [0x84 0x00]    # Set register (write address 0x84)
I2C> [0x85 r]       # Read data (read address 0x85)
```

**Note:** Bus Pirate 5 uses 8-bit I2C addresses. Device at 7-bit address 0x42 = Write 0x84, Read 0x85.

## Learning Objectives

Students will learn:
- I2C device enumeration and communication
- Reading and writing hardware registers
- Protocol analysis and state machines
- Multi-protocol coordination
- Systematic hardware debugging
- Real-world hardware hacking techniques

## Use Cases

- **Classroom Training** - Structured hardware security course
- **CTF Competitions** - Hardware hacking category
- **Makerspaces** - Self-guided learning stations
- **Workshops** - Hands-on Bus Pirate training
- **Security Training** - Embedded systems security

## Instructor Features

- Real-time monitoring via Serial Monitor (115200 baud)
- View all student attempts and progress
- Easily create custom scenarios
- Adjustable difficulty levels
- Complete answer key provided

## Troubleshooting

**Device not found?**
- Check connections (SDA=18, SCL=19, GND)
- Enable pull-ups: `P` command in I2C mode
- Run `scan` to verify device at 0x42

**Commands not working?**
- Use 8-bit addresses: 0x84 (write), 0x85 (read)
- See [I2C_ADDRESS_REFERENCE.md](I2C_ADDRESS_REFERENCE.md)

**Need help?**
- Check [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md)
- See [STUDENT_GUIDE.md](STUDENT_GUIDE.md) for command examples

## Contributing

Contributions welcome! Feel free to:
- Submit bug reports and feature requests
- Improve documentation
- Create new challenge levels
- Share custom scenarios

## License

MIT License - See [LICENSE](LICENSE) file for details.

## Acknowledgments

Created as an educational tool for hardware hacking and Bus Pirate training. Designed to teach real-world embedded systems security skills in a safe, controlled environment.

---

**⚠️ Educational Use Only:** This tool is for authorized educational and training purposes. Always practice ethical hardware hacking with proper authorization.

## Quick Links

- [Bus Pirate 5 Documentation](https://docs.buspirate.com/)
- [Teensy 3.1 Info](https://www.pjrc.com/teensy/teensy31.html)
- [i2c_t3 Library](https://github.com/nox771/i2c_t3)

**Happy Hacking! 🏴‍☠️**
