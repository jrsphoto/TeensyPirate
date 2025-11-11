# Bus Pirate Hardware Hacking Game - Solutions

## Default Configuration Solutions

These solutions assume the default/medium difficulty settings in the code.

## Level 0: Initial Key Discovery

### Objective

Find and submit the 16-bit key: **0xDEAD**

### Solution Steps

- **Initial reconnaissance:**

HiZ> m # Enter mode menu  
Mode> 5 # Select I2C mode  
I2C> scan # Scan for devices

For more detail:

I2C> scan -v # Verbose output

Expected: Device found at 0x42 (hex 0x42, decimal 66)

If scan doesn't work, try manually:

I2C> \[0x85 r\] # Try reading from 0x85 (read address)

- **Read device information:**

\[0x84 0x00\] # Set pointer to register 0x00  
\[0x85 r\] # Read device ID

Returns: 0x42

\[0x84 0x02\] # Set pointer to register 0x02  
\[0x85 r\] # Read current level

Returns: 0x00 (Level 0)

- **Scan all registers for clues:**

\[0x84 0x00\] # Set pointer to start  
\[0x85 r:16\] # Read first 16 registers

- **Submit the key (0xDEAD):**

\[0x84 0x10 0xDE\] # Write high byte to reg 0x10  
\[0x84 0x11 0xAD\] # Write low byte to reg 0x11

- **Verify success:**

\[0x84 0x12\] # Set pointer to verification register  
\[0x85 r\] # Read result

Returns: 0x01 (Success!)

- **Check new level:**

\[0x84 0x02\]  
\[0x85 r\]

Returns: 0x01 (Advanced to Level 1)

**Teaching Points:** - Systematic register scanning - Understanding 16-bit value submission - Reading feedback registers

## Level 1: Magic Sequence

### Objective

Enter the magic sequence: **0x12, 0x34, 0x56, 0x78**

### Solution Steps

- **Check current challenge:**

\[0x84 0x21\] # Set pointer to sequence progress  
\[0x85 r\] # Read progress

Returns: 0x00 (No progress yet)

- **Enter sequence bytes one at a time:**

\[0x84 0x20 0x12\] # First byte  
\[0x84 0x21\] # Point to progress register  
\[0x85 r\] # Check progress

Returns: 0x01 (1 byte correct)

\[0x84 0x20 0x34\] # Second byte  
\[0x84 0x21\]  
\[0x85 r\]

Returns: 0x02

\[0x84 0x20 0x56\] # Third byte  
\[0x84 0x21\]  
\[0x85 r\]

Returns: 0x03

\[0x84 0x20 0x78\] # Fourth byte  
\[0x84 0x21\]  
\[0x85 r\]

Returns: 0x04 (Complete!)

- **Verify level advancement:**

\[0x84 0x02\]  
\[0x85 r\]

Returns: 0x02 (Level 2)

**Common Mistakes:** - Entering wrong byte → sequence resets to 0 - Not checking progress after each byte - Trying to enter all bytes at once

**Teaching Points:** - State machine concept - Error recovery - Incremental verification

## Level 2: Multi-Protocol Challenge

### Part A: UART Password

### Objective

Send password via UART: **"HACKTHEPLANET"**

### Solution Steps

- **Switch to UART mode:**

I2C> m # Exit to mode menu  
Mode> 3 # Select UART mode  
UART> # Now in UART mode

(Follow baud rate setup prompts if asked - select 115200)

- **Connect to UART:**

- Disconnect I2C wires (or keep GND)
- Connect Bus Pirate TX → Teensy Pin 0 (RX1)
- Connect Bus Pirate RX → Teensy Pin 1 (TX1)
- Ensure GND connected

- **Send password:**

"HACKTHEPLANET"

Expected response: "UART CHALLENGE COMPLETE!"

### Part B: GPIO Pulses

### Objective

Send 10 pulses to GPIO pin 3

### Solution Steps

- **Switch to GPIO mode:**

UART> m # Exit to mode menu  
Mode> 1 # Select HiZ (GPIO) mode  
HiZ> # Now in HiZ mode

- **Connect:**

- Bus Pirate AUX/IO pin → Teensy Pin 3
- Keep GND connected

- **Send 10 pulses:**

W # Set HIGH  
w # Set LOW (pulse 1)  
W  
w # Pulse 2  
W  
w # Pulse 3  
... (repeat 10 times total)

- **Verify completion via I2C:**

HiZ> m # Mode menu  
Mode> 5 # Back to I2C mode  
I2C> \[0x84 0x04\] # Point to challenge status  
I2C> \[0x85 r\] # Read status

- Bit 0x0C should be set (both challenges complete)

**Teaching Points:** - Multi-protocol coordination - Physical connection management - ASCII vs binary communication

## Level 3: Vault Unlocking

### Objective

Submit master password (0x1337) and extract vault data

### Solution Steps

- **Ensure in I2C mode:**

HiZ> m # Mode menu (if not already in I2C)  
Mode> 5 # Select I2C mode  
I2C>

- **Read unlock code location:**

\[0x84 0x30\] # Point to registers 0x30-0x31  
\[0x85 r:2\] # Read 2 bytes

This gives you a hint about the structure

- **Submit master password (0x1337):**

\[0x84 0x10 0x13\] # High byte  
\[0x84 0x11 0x37\] # Low byte

- **Verify vault unlocked:**

\[0x84 0x12\]  
\[0x85 r\]

Returns: 0x01

- **Scan for new devices:**

I2C> scan # Scan bus  
I2C> scan -v # Verbose - shows all details

Should now find devices at: - 0x42 (main device) - 0x55 (vault - now unlocked!) - 0x66 (secret device - may appear)

Or manually verify:

I2C> \[0xAA r\] # Try vault write address (0x55 << 1)  
I2C> \[0xCC r\] # Try secret write address (0x66 << 1)

- **Read vault data (0x55 / 0xAA-0xAB):**

\[0xAA 0x00\] # Point to vault ID  
\[0xAB r\] # Read vault ID  
\[0xAA 0x01\] # Point to status  
\[0xAB r\] # Status (should be 0x01 = unlocked)  
\[0xAA 0x10\] # Point to vault data  
\[0xAB r:16\] # Read vault data

- **Read final flag from secret device (0x66 / 0xCC-0xCD):**

\[0xCC 0x00\] # Point to device ID  
\[0xCD r\] # Read device ID  
\[0xCC 0x01\] # Point to flag  
\[0xCD r\] # Final flag (0xF1)

- **Verify game won:**

\[0x84 0x01\]  
\[0x85 r\]

Returns: 0xFF (Victory!)

**Teaching Points:** - Master keys vs. level keys - Progressive device reveal - Data extraction after unlock - Multi-address I2C bus

## Complete Speedrun Script (Bus Pirate 5)

For experienced users or demonstration purposes:

\# Start from HiZ> prompt  
<br/>\# Level 0  
m  
5 # I2C mode  
scan # Find device  
\[0x84 0x10 0xDE 0x11 0xAD\]  
<br/>\# Level 1  
\[0x84 0x20 0x12\]  
\[0x84 0x20 0x34\]  
\[0x84 0x20 0x56\]  
\[0x84 0x20 0x78\]  
<br/>\# Level 2 - UART  
m  
3 # UART mode  
"HACKTHEPLANET"  
<br/>\# Level 2 - GPIO  
m  
1 # HiZ mode  
W w W w W w W w W w W w W w W w W w W w  
<br/>\# Level 3  
m  
5 # Back to I2C  
\[0x84 0x10 0x13 0x11 0x37\]  
scan # Find new devices  
\[0xAA 0x10\]  
\[0xAB r:16\]  
\[0xCC 0x01\]  
\[0xCD r\]

Estimated speedrun time: 3-7 minutes for expert

## Alternative Discovery Methods

### Method 1: Brute Force Keys

Write a script to try common hex values: - 0x0000 through 0xFFFF (time consuming!) - Common values: 0x1234, 0xDEAD, 0xBEEF, 0xCAFE, 0xBABE, etc.

### Method 2: Register Analysis

Create a map of all readable registers and look for: - ASCII text that might be hints - Patterns in hex values - Registers that change over time - Related values across registers

### Method 3: Traffic Analysis

Monitor what registers the device accesses internally: - Some devices leak info through timing - Watch for auto-incrementing values - Observe LED blink patterns

## Hints for Stuck Students

### Can't find Level 0 key

- "Try common hex words: DEAD, BEEF, CAFE…"
- "The key uses letters A-F in hex"
- "Think about what a hacker might use as a test value"

### Sequence keeps resetting

- "You need to enter bytes one at a time"
- "Check progress after EACH byte"
- "The sequence follows a pattern - look for it"

### UART password unknown

- "Think about famous hacker movie quotes"
- "It's all uppercase"
- "Related to hackers taking over the world…"

### GPIO pulses not working

- "You need falling edges, not just LOW"
- "Try: HIGH, LOW, HIGH, LOW…"
- "Count carefully - register 0x04 tracks your progress"

### Can't find master password

- "It's a common 'leet speak' number"
- "Written as 1337 in decimal"
- "Think elite hackers…"

## Debugging Tips

If the game isn't progressing correctly:

- **Check register 0x04 (Challenge Status):**
  - Bit 0 (0x01): Key found
  - Bit 1 (0x02): Sequence complete
  - Bit 2 (0x04): UART unlocked
  - Bit 3 (0x08): GPIO challenge done
  - Bit 4 (0x10): Vault unlocked
- **Monitor Serial output** (instructor only):
  - Shows all attempts and progress
  - Displays wrong keys/passwords entered
  - Confirms challenge completions
- **Reset if needed:**
  - Press Teensy reset button
  - Re-upload sketch
  - Check wiring connections

## Bus Pirate 5 Command Quick Reference

| Task | BP5 Command | Notes |
| --- | --- | --- |
| Mode menu | m   | From any prompt |
| Select I2C | 5   | From mode menu |
| Select UART | 3   | From mode menu |
| Select HiZ/GPIO | 1   | From mode menu |
| Scan I2C bus | scan | In I2C mode |
| Manual probe | \[0x85 r\] | Try reading from device |
| Enable pull-ups | P   | Capital P |
| Pins HIGH | W   | Capital W |
| Pins LOW | w   | Lowercase w |
| Check voltage | v   | Any mode |
| Help | ?   | Any mode |

## Creating New Scenarios

To create a fresh game with different solutions:

- Change the keys in the code:

# define KEY_LEVEL1 0xABCD // Your new key  
# define KEY_LEVEL2 0x9876  
# define KEY_LEVEL3 0x4567  
# define MASTER_PASSWORD 0x1111

- Update sequences:

const uint8_t MAGIC_SEQUENCE\[\] = {0xAA, 0xBB, 0xCC, 0xDD};

- Change passwords:

const char UART_PASSWORD\[\] = "NEWPASSWORD";

- Adjust difficulty:

# define GPIO_PULSE_COUNT_REQUIRED 20 // More pulses = harder

- Update this solution guide accordingly!

**Remember:** The goal is education, not frustration. Provide hints appropriately!
