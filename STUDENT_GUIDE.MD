# Bus Pirate Hardware Hacking Challenge

## Student Guide

Welcome to the Bus Pirate Hardware Hacking Challenge! Your mission is to progress through multiple levels by analyzing hardware, discovering hidden information, and exploiting communication protocols.

## Your Tools

- **Bus Pirate 5** - Your primary tool for hardware analysis
- **Teensy 3.1 Target** - The device you're trying to hack
- **Your Skills** - Protocol knowledge, persistence, and creativity

## Getting Started

### Initial Setup

- **Connect to the target device via I2C:**

- HiZ> m  
    Mode> 5  
    I2C>
- This enters I2C mode (default settings are usually fine).

- **Scan for devices:**

- I2C> scan
- For more detail:
- I2C> scan -v
- You should find at least one device. What address is it?
- **Note:** You may see multiple I2C devices (like sensors on the board). The game device shows as **0x42** in the scan. To communicate with it, use write address **0x84** and read address **0x85**.
- **Fallback:** If scan doesn't work, try manually: \[0x85 r\]

- **Reconnaissance:**
  - Try reading from different registers
  - Look for patterns
  - Some registers might give you hints!

## Game Structure

The game has multiple levels. Each level requires you to: 1. Discover what challenge you need to solve 2. Find the key, password, or sequence needed 3. Submit it correctly to advance

### Important I2C Registers (Address 0x42)

| Register | Purpose | Hint |
| --- | --- | --- |
| 0x00 | Device ID | Start here |
| 0x01 | Game Status | Check if game is active |
| 0x02 | Current Level | What level are you on? |
| 0x03 | Hint Byte | Might give you a clue |
| 0x04 | Challenge Status | Bitmask of completed challenges |
| 0x10-0x11 | Key Input | Submit keys here (16-bit) |
| 0x12 | Key Verify | Did your key work? |
| 0x20 | Sequence Input | For sequence challenges |
| 0x21 | Sequence Progress | How many bytes correct? |

## Level Challenges

### Level 0: Initial Access

**Goal:** Find and submit the correct key to gain access.

**Hints:** - Keys are 16-bit values (2 bytes) - Write the high byte to register 0x10 - Write the low byte to register 0x11 - Read register 0x12 to check if correct - The key might be hidden in plain sight… or encoded somehow

**Questions to explore:** - What data can you read from the device? - Are there any patterns in the register values? - What happens when you write different values?

### Level 1: The Vault Appears

**Goal:** Complete the magic sequence challenge.

**Hints:** - A new device might appear on the I2C bus - Sequences must be entered in the correct order - Check register 0x21 for progress - Wrong byte = sequence resets! - The sequence is 4 bytes long

**Bus Pirate Tips:** - Use scan (or scan -v for details) to find devices - Try reading from different addresses - Look for clues in register values

### Level 2: Multi-Protocol Challenge

**Goal:** Complete TWO challenges - UART and GPIO.

**UART Challenge:** - Connect to the UART interface (Serial1: pins 0 and 1) - Find the correct password - Passwords are ASCII strings - You might need to try common phrases…

**GPIO Challenge:** - Send a specific number of pulses to pin 3 - Use Bus Pirate's HiZ/GPIO mode - Count your pulses carefully!

**Hints:** - Both challenges must be completed - Check challenge status register (0x04) to see progress - Each bit represents a different challenge

### Level 3: The Final Vault

**Goal:** Unlock the vault and retrieve the flag.

**Hints:** - You'll need a master password - A new device might be available - The vault contains encrypted data - Once unlocked, the data is revealed - Victory is close!

## Bus Pirate 5 Command Reference

### Changing Modes

From the HiZ> prompt:

HiZ> m # Enter mode selection  
Mode> 5 # Select I2C (5)  
Mode> 3 # Select UART (3)  
Mode> 1 # Select HiZ/GPIO (1)

**Mode Numbers:** - 1 = HiZ (GPIO mode) - 2 = 1-Wire - 3 = UART - 4 = HD-UART - 5 = I2C - 6 = SPI - 7 = 2-Wire - 8 = 3-Wire - 9 = DIO - 10 = LED - 11 = Infrared - 12 = JTAG

### I2C Mode Commands

**Scan for devices:**

I2C> scan # Scan I2C bus for devices  
I2C> scan -v # Verbose scan with more details

If scan doesn't work, manually check:

I2C> \[0x85 r\] # Try reading from game device (read address)

**Read a single register:**

I2C> \[0x84 0x00\] # Set register pointer to 0x00 (write)  
I2C> \[0x85 r\] # Read one byte (read)

**Read multiple bytes:**

I2C> \[0x84 0x10\] # Set pointer to 0x10  
I2C> \[0x85 r:2\] # Read 2 bytes

**Write to a register:**

I2C> \[0x84 0x10 0xDE 0xAD\] # Write 0xDEAD to registers 0x10-0x11

**Understanding I2C Addresses:** - The device is at 7-bit address **0x42** - For I2C operations, use 8-bit addresses: - **0x84** = Write address (0x42 << 1) - **0x85** = Read address (0x42 << 1 | 1)

**Check voltages:**

I2C> v # Display voltage readings

**Enable/disable pull-ups:**

I2C> P # Capital P - enable pull-ups  
I2C> p # Lowercase p - disable pull-ups

or

I2C> W # Enable power/pull-ups (use carefully!)

### UART Mode Commands

**Enter UART mode:**

HiZ> m  
Mode> 3 # Select UART  
UART>

**Send text:**

UART> "Hello" # Send ASCII text with quotes

**Send hex bytes:**

UART> 0x48 0x65 # Send hex values

**Return to mode selection:**

UART> m # Back to mode menu

### HiZ/GPIO Mode Commands

**Enter HiZ mode:**

HiZ> m  
Mode> 1 # Select HiZ  
HiZ>

**Set individual pins HIGH/LOW:**

HiZ> W # Set all AUX pins HIGH  
HiZ> w # Set all AUX pins LOW

**Read pin states:**

HiZ> v # Read voltages and pin states

### General Commands (Work in most modes)

? # Show available commands  
v # Display voltages  
~ # Reset Bus Pirate  
i # Show version/info

## Troubleshooting

**Can't find device:** - Check connections (SDA=18, SCL=19, GND) - Enable pull-ups: P (capital P) - Try scanning: scan - Or manually probe: \[0x85 r\] - Check voltage: v

**Key not working:** - Make sure you're writing HIGH byte first (0x10), then LOW byte (0x11) - Check byte order (big-endian vs little-endian) - Read 0x12 immediately after writing

**Sequence keeps resetting:** - You must enter ALL bytes correctly in order - Check register 0x21 for progress - One wrong byte = start over

**UART not responding:** - Verify you're in UART mode (mode 3) - Check TX/RX connections (don't swap!) - Try sending with quotes: "password" - Verify baud rate in UART setup menu

**GPIO not working:** - Make sure you're in HiZ mode (mode 1) - Check physical connections - Try v to verify pin states

## Strategy Tips

- **Read Everything First**
  - Scan all registers 0x00-0x3F
  - Look for patterns, ASCII text, or obvious values
  - Document what you find
- **Try Common Values**
  - 0xDEAD, 0xBEEF, 0xCAFE - common test values
  - 0x1234, 0x5678 - sequential patterns
  - Powers of 2, common hex values
- **Monitor Changes**
  - Read the same register multiple times
  - Does it change? Is it a counter?
  - Dynamic values might encode information
- **Think Like a Hardware Hacker**
  - Devices often hide in plain sight
  - Debug ports might be enabled
  - Manufacturers use predictable patterns
- **Check Status Often**
  - Register 0x04 shows challenge progress
  - Register 0x02 shows current level
  - LED blink rate changes with level

## Scoring

Your performance is measured by: - **Time:** How fast can you complete all levels? - **Attempts:** Fewer wrong attempts = better score - **Style:** Did you use clever techniques?

## Victory Condition

When you complete the final challenge: - Register 0x01 will show 0xFF (game won!) - The LED will flash rapidly - A final flag will be revealed - Your time and attempt count will be displayed

## Bus Pirate 5 Tips

- Use m to change modes anytime
- Press ? in any mode to see available commands
- The (1) macro in I2C mode scans the bus
- Capital letters often mean "enable" (P, W)
- Lowercase letters often mean "disable" (p, w)
- Use v often to check voltage levels

## Good Luck

Remember: Persistence and methodical analysis win the day. When stuck, go back to basics - read, analyze, experiment!

_Note: This is an educational tool. Real hardware hacking requires authorization!_
