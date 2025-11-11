# Bus Pirate 5 Hardware Hacking Game - Setup Checklist

## Hardware Checklist

- ☐ Bus Pirate 5
- ☐ Teensy 3.1 (or compatible ARM board)
- ☐ USB cable for Teensy
- ☐ USB cable for Bus Pirate
- ☐ 3-4 jumper wires (female-to-female recommended)
- ☐ Computer with Arduino IDE + Teensyduino

## Software Setup

- ☐ Install Arduino IDE (1.8.x or 2.x)
- ☐ Install Teensyduino from <https://www.pjrc.com/teensy/td_download.html>
- ☐ Verify i2c_t3 library is available (included with Teensyduino)
- ☐ Upload buspirate_ctf_game.ino to Teensy 3.1
- ☐ Verify sketch compiles without errors

## Wiring Setup

Connect Bus Pirate 5 to Teensy 3.1:

Bus Pirate 5 Teensy 3.1  
────────────────────────────────  
SDA (I2C Data) → Pin 18  
SCL (I2C Clock) → Pin 19  
GND (Ground) → GND

**Power Options:** - Option A: Teensy powered via USB (recommended for testing) - Option B: Teensy powered by Bus Pirate (check current limits)

## Bus Pirate 5 Test

- **Connect Bus Pirate to computer**
- **Open terminal** (PuTTY, screen, minicom, etc.)
- **Find correct port** (COM port on Windows, /dev/ttyUSB\* on Linux)
- **Set baud rate:** 115200
- **Test connection:**

- HiZ> i
- Should show version info

## Teensy Test

- **Connect Teensy via USB**
- **Open Arduino Serial Monitor** (Tools → Serial Monitor)
- **Set baud to 115200**
- **Look for startup message:**

- \========================================  
    BUS PIRATE HARDWARE HACKING GAME  
    \========================================

## First I2C Test

- **On Bus Pirate, enter I2C mode:**

- HiZ> m  
    Mode> 5  
    I2C>

- **Enable pull-ups:**

- I2C> P

- **Check voltage:**

- I2C> v
- Should show 3.3V

- **Try reading from device:**

- I2C> scan
- Should show device at 0x42
- Or manually:
- I2C> \[0x85 r\]
- Should get ACK and data (not NACK)

- **Read device ID:**

- I2C> \[0x84 0x00\]  
    I2C> \[0x85 r\]
- Should return 0x42

## If Something Doesn't Work

### No device found on I2C bus

- Check wiring (SDA=18, SCL=19, GND)
- Verify pull-ups enabled: P
- Check voltage: v (should be ~3.3V)
- Try manual read: \[0x85 r\]
- Press reset button on Teensy
- Check Serial Monitor for Teensy startup message

### Can't compile sketch

- Verify Teensyduino is installed
- Check Board selected: Tools → Board → Teensy 3.1
- Verify i2c_t3 library is available
- Try "Verify" button first before uploading

### Bus Pirate not responding

- Check COM port is correct
- Verify baud rate is 115200
- Try unplugging and reconnecting
- Reset Bus Pirate with ~ command

### Wrong data from I2C reads

- Double-check wiring (especially SDA/SCL not swapped)
- Verify common ground connection
- Check for loose connections
- Measure voltage with multimeter if available

## Ready to Play?

Once you can successfully: - See Teensy startup message in Serial Monitor - Enter I2C mode on Bus Pirate - Read 0x42 from \[0x42 0x00 r\]

You're ready! Give students the STUDENT_GUIDE.md and let them hack!

## Instructor Mode

Keep Arduino Serial Monitor open while students use Bus Pirate. You'll see: - All student attempts (right and wrong) - Challenge completions - Level progression - Victory announcement

**Don't show students the Serial Monitor** - it reveals all answers!

## Game Reset

To restart the game: - Press reset button on Teensy - Or re-upload the sketch - Game state resets to Level 0

## Customization (Optional)

Before uploading, edit these values in buspirate_ctf_game.ino:

// Change keys (line ~25)  
# define KEY_LEVEL1 0xDEAD  
# define KEY_LEVEL2 0xBEEF  
# define KEY_LEVEL3 0xCAFE  
# define MASTER_PASSWORD 0x1337  
<br/>// Change passwords (line ~35)  
const char UART_PASSWORD\[\] = "HACKTHEPLANET";  
<br/>// Change sequences (line ~32)  
const uint8_t MAGIC_SEQUENCE\[4\] = {0x12, 0x34, 0x56, 0x78};  
<br/>// Adjust difficulty (line ~38)  
# define GPIO_PULSE_COUNT_REQUIRED 10

Save changes and re-upload to Teensy.

## Quick Troubleshooting Reference

| Problem | Quick Fix |
| --- | --- |
| No I2C device | Check wiring, enable pull-ups (P) |
| Wrong data | Verify SDA/SCL not swapped |
| Can't compile | Install Teensyduino |
| BP not responding | Check COM port & baud (115200) |
| Teensy silent | Open Serial Monitor at 115200 |
| Game stuck | Press Teensy reset button |

## Success Criteria

**Minimum working setup:** - Teensy shows startup message ✓ - Bus Pirate enters I2C mode ✓ - Can read 0x42 from device ID: \[0x84 0x00\] then \[0x85 r\] ✓

**You're ready to go!** 🎉

For detailed gameplay instructions, see STUDENT_GUIDE.md For teaching tips, see INSTRUCTOR_MANUAL.md For answers, see SOLUTIONS.md (instructors only!)
