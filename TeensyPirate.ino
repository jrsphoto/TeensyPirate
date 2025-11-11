/*
 * Bus Pirate Hardware Hacking Training Game
 * For Teensy 3.1
 * 
 * A Capture-The-Flag style hardware hacking game that teaches:
 * - I2C device enumeration and communication
 * - Reading and writing registers
 * - Protocol analysis
 * - Data extraction and encoding
 * - Multi-step challenges
 * 
 * GAME CONFIGURATION (Instructor/Designer Settings)
 * Change these values to create different game scenarios
 */

#include <i2c_t3.h>
#include <SPI.h>

// ============================================================================
// GAME CONFIGURATION - EDIT THESE TO CUSTOMIZE THE GAME
// ============================================================================

// I2C addresses for different "devices" in the game
#define I2C_ADDR_MAIN 0x42        // Main control device (always present)
#define I2C_ADDR_VAULT 0x55       // Hidden vault (Level 2+)
#define I2C_ADDR_SECRET 0x66      // Secret device (Level 3+)

// Game keys and passwords - CHANGE THESE!
#define KEY_LEVEL1 0xDEAD         // 16-bit key for level 1
#define KEY_LEVEL2 0xBEEF         // 16-bit key for level 2
#define KEY_LEVEL3 0xCAFE         // 16-bit key for level 3
#define MASTER_PASSWORD 0x1337    // Master unlock code

// Challenge parameters
#define REQUIRED_SEQUENCE_LENGTH 4    // Number of steps in sequence challenge
const uint8_t MAGIC_SEQUENCE[REQUIRED_SEQUENCE_LENGTH] = {0x12, 0x34, 0x56, 0x78};

// UART challenge password
const char UART_PASSWORD[] = "HACKTHEPLANET";  // Change this!

// GPIO challenge - how many pulses needed
#define GPIO_PULSE_COUNT_REQUIRED 10

// ============================================================================
// Pin Assignments
// ============================================================================

#define GPIO_INPUT_PIN 3          // Input for challenges
#define GPIO_OUTPUT_PIN 4         // Output for feedback
#define LED_PIN 13                // Onboard LED for status

// ============================================================================
// Register Maps for Different I2C "Devices"
// ============================================================================

// Main Device (0x42) - Always visible
#define REG_DEVICE_ID 0x00        // Returns device identifier
#define REG_GAME_STATUS 0x01      // Current game state
#define REG_LEVEL 0x02            // Current level (0-3)
#define REG_HINT 0x03             // Hint byte
#define REG_CHALLENGE_STATUS 0x04 // Bitmask of completed challenges
#define REG_KEY_INPUT_HIGH 0x10   // Write key here (high byte)
#define REG_KEY_INPUT_LOW 0x11    // Write key here (low byte)
#define REG_KEY_VERIFY 0x12       // Read to verify key (1=correct, 0=wrong)
#define REG_SEQUENCE_INPUT 0x20   // Write sequence bytes here
#define REG_SEQUENCE_CHECK 0x21   // Sequence progress (0-4)
#define REG_UNLOCK_CODE 0x30      // Revealed after challenges

// Vault Device (0x55) - Appears after Level 1
#define REG_VAULT_ID 0x00         // Vault identifier
#define REG_VAULT_STATUS 0x01     // Locked/Unlocked status
#define REG_VAULT_DATA 0x10       // Start of vault data (16 bytes)

// Secret Device (0x66) - Appears after Level 2
#define REG_SECRET_ID 0x00        // Secret device ID
#define REG_SECRET_FLAG 0x01      // Final flag location

// ============================================================================
// Game State
// ============================================================================

struct GameState {
  uint8_t level;                  // Current level (0-3)
  bool level1_complete;
  bool level2_complete;
  bool level3_complete;
  bool game_won;
  
  // Challenge tracking
  bool key_found;
  bool sequence_complete;
  bool uart_unlocked;
  bool gpio_challenge_done;
  
  // Sequence challenge state
  uint8_t sequence_position;
  uint8_t sequence_buffer[REQUIRED_SEQUENCE_LENGTH];
  
  // GPIO challenge state
  uint8_t gpio_pulse_count;
  bool last_gpio_state;
  
  // Key storage
  uint16_t current_key;
  
  // Vault data (revealed progressively)
  uint8_t vault_data[16];
  bool vault_unlocked;
  
  // Statistics
  uint32_t attempts;
  uint32_t start_time;
};

GameState game;

// I2C data buffers for each device
volatile uint8_t main_regs[64] = {0};
volatile uint8_t vault_regs[32] = {0};
volatile uint8_t secret_regs[16] = {0};
volatile uint8_t current_register = 0;
volatile uint8_t current_device = I2C_ADDR_MAIN;

// ============================================================================
// Setup
// ============================================================================

void setup() {
  // Initialize pins
  pinMode(GPIO_INPUT_PIN, INPUT_PULLUP);
  pinMode(GPIO_OUTPUT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(GPIO_OUTPUT_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize game state
  initializeGame();
  
  // Initialize I2C as slave on main address
  Wire.begin(I2C_SLAVE, I2C_ADDR_MAIN, I2C_PINS_18_19, I2C_PULLUP_EXT, 100000);
  Wire.onReceive(i2cReceiveEvent);
  Wire.onRequest(i2cRequestEvent);
  
  // Initialize Serial for debugging/monitoring
  Serial.begin(115200);
  delay(1500);
  
  Serial.println(F("========================================"));
  Serial.println(F("BUS PIRATE HARDWARE HACKING GAME"));
  Serial.println(F("========================================"));
  Serial.println();
  Serial.println(F("INSTRUCTOR MODE - Serial Monitor"));
  Serial.println(F("Students should use Bus Pirate only!"));
  Serial.println();
  printGameConfiguration();
  
  // Initialize Serial1 for game UART challenges
  Serial1.begin(115200);
  
  // Flash LED to indicate ready
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  updateGameLogic();
  updateI2CRegisters();
  checkGPIOChallenge();
  
  if (Serial1.available()) {
    handleUARTChallenge();
  }
  
  // LED indicates current level
  static unsigned long lastBlink = 0;
  unsigned long blinkRate = 1000 / (game.level + 1);
  if (millis() - lastBlink > blinkRate) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    lastBlink = millis();
  }
  
  delay(10);
}

// ============================================================================
// Game Initialization
// ============================================================================

void initializeGame() {
  game.level = 0;
  game.level1_complete = false;
  game.level2_complete = false;
  game.level3_complete = false;
  game.game_won = false;
  game.key_found = false;
  game.sequence_complete = false;
  game.uart_unlocked = false;
  game.gpio_challenge_done = false;
  game.sequence_position = 0;
  game.gpio_pulse_count = 0;
  game.last_gpio_state = HIGH;
  game.current_key = 0;
  game.vault_unlocked = false;
  game.attempts = 0;
  game.start_time = millis();
  
  // Initialize main device registers
  main_regs[REG_DEVICE_ID] = 0x42;  // Device ID
  main_regs[REG_GAME_STATUS] = 0x01; // Game active
  main_regs[REG_LEVEL] = 0;
  main_regs[REG_HINT] = 0x01;  // First hint
  main_regs[REG_CHALLENGE_STATUS] = 0x00;
  
  // Initialize vault with encrypted data
  vault_regs[REG_VAULT_ID] = 0x55;
  vault_regs[REG_VAULT_STATUS] = 0x00; // Locked
  
  // Fill vault with "encrypted" data (XOR with key)
  for (int i = 0; i < 16; i++) {
    game.vault_data[i] = i * 17; // Simple pattern
    vault_regs[REG_VAULT_DATA + i] = game.vault_data[i] ^ 0xAA; // XOR encryption
  }
  
  // Initialize secret device
  secret_regs[REG_SECRET_ID] = 0x66;
  secret_regs[REG_SECRET_FLAG] = 0x00; // Hidden until unlocked
}

// ============================================================================
// Game Logic
// ============================================================================

void updateGameLogic() {
  // Level 0 -> 1: Find the key
  if (game.level == 0 && game.key_found) {
    advanceToLevel1();
  }
  
  // Level 1 -> 2: Complete the sequence challenge
  if (game.level == 1 && game.sequence_complete) {
    advanceToLevel2();
  }
  
  // Level 2 -> 3: Complete UART and GPIO challenges
  if (game.level == 2 && game.uart_unlocked && game.gpio_challenge_done) {
    advanceToLevel3();
  }
  
  // Level 3: Unlock vault and find final flag
  if (game.level == 3 && game.vault_unlocked && !game.game_won) {
    winGame();
  }
}

void advanceToLevel1() {
  game.level = 1;
  game.level1_complete = true;
  main_regs[REG_LEVEL] = 1;
  main_regs[REG_HINT] = 0x02;  // New hint
  main_regs[REG_CHALLENGE_STATUS] |= 0x01;
  
  // Enable vault device on I2C
  // (In real implementation, would start responding to 0x55)
  
  Serial.println(F("\n*** LEVEL 1 UNLOCKED ***"));
  Serial.println(F("New device appeared at I2C address 0x55"));
  Serial.println(F("Challenge: Complete the magic sequence"));
}

void advanceToLevel2() {
  game.level = 2;
  game.level2_complete = true;
  main_regs[REG_LEVEL] = 2;
  main_regs[REG_HINT] = 0x03;  // New hint
  main_regs[REG_CHALLENGE_STATUS] |= 0x02;
  
  Serial.println(F("\n*** LEVEL 2 UNLOCKED ***"));
  Serial.println(F("Challenge: UART password and GPIO pulses"));
  Serial.println(F("Hint: Try the UART interface"));
}

void advanceToLevel3() {
  game.level = 3;
  game.level3_complete = true;
  main_regs[REG_LEVEL] = 3;
  main_regs[REG_HINT] = 0x04;  // Final hint
  main_regs[REG_CHALLENGE_STATUS] |= 0x04;
  
  // Reveal unlock code in register
  uint16_t unlock = KEY_LEVEL3;
  main_regs[REG_UNLOCK_CODE] = (unlock >> 8) & 0xFF;
  main_regs[REG_UNLOCK_CODE + 1] = unlock & 0xFF;
  
  Serial.println(F("\n*** LEVEL 3 UNLOCKED ***"));
  Serial.println(F("Final challenge: Unlock the vault"));
  Serial.println(F("A new device may have appeared..."));
}

void winGame() {
  game.game_won = true;
  main_regs[REG_GAME_STATUS] = 0xFF; // Victory!
  secret_regs[REG_SECRET_FLAG] = 0xF1; // Flag revealed
  
  uint32_t elapsed = (millis() - game.start_time) / 1000;
  
  Serial.println(F("\n"));
  Serial.println(F("╔═══════════════════════════════════╗"));
  Serial.println(F("║   🎉 GAME WON! CONGRATULATIONS! 🎉   ║"));
  Serial.println(F("╚═══════════════════════════════════╝"));
  Serial.println();
  Serial.print(F("Time: "));
  Serial.print(elapsed);
  Serial.println(F(" seconds"));
  Serial.print(F("Attempts: "));
  Serial.println(game.attempts);
  Serial.println();
  Serial.println(F("The student has mastered Bus Pirate hardware hacking!"));
  
  // Victory LED pattern
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(GPIO_OUTPUT_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(GPIO_OUTPUT_PIN, LOW);
    delay(100);
  }
}

// ============================================================================
// I2C Register Management
// ============================================================================

void updateI2CRegisters() {
  // Update dynamic registers
  main_regs[REG_GAME_STATUS] = game.game_won ? 0xFF : 0x01;
  main_regs[REG_LEVEL] = game.level;
  
  // Update challenge status bitmask
  uint8_t status = 0;
  if (game.key_found) status |= 0x01;
  if (game.sequence_complete) status |= 0x02;
  if (game.uart_unlocked) status |= 0x04;
  if (game.gpio_challenge_done) status |= 0x08;
  if (game.vault_unlocked) status |= 0x10;
  main_regs[REG_CHALLENGE_STATUS] = status;
  
  // Update sequence progress
  main_regs[REG_SEQUENCE_CHECK] = game.sequence_position;
  
  // Vault status
  vault_regs[REG_VAULT_STATUS] = game.vault_unlocked ? 0x01 : 0x00;
}

// ============================================================================
// I2C Handlers
// ============================================================================

void i2cReceiveEvent(size_t numBytes) {
  if (numBytes == 0) return;
  
  static bool register_set = false;
  
  // First byte is register address
  if (!register_set) {
    current_register = Wire.readByte();
    register_set = true;
    numBytes--;
  }
  
  // Process writes
  while (Wire.available()) {
    uint8_t value = Wire.readByte();
    handleRegisterWrite(current_register, value);
    current_register++;
  }
  
  if (!Wire.available()) {
    register_set = false;
  }
}

void i2cRequestEvent() {
  uint8_t data = handleRegisterRead(current_register);
  Wire.write(data);
  current_register++;
}

void handleRegisterWrite(uint8_t reg, uint8_t value) {
  game.attempts++;
  
  switch (reg) {
    case REG_KEY_INPUT_HIGH:
      game.current_key = (value << 8) | (game.current_key & 0xFF);
      main_regs[reg] = value;
      break;
      
    case REG_KEY_INPUT_LOW:
      game.current_key = (game.current_key & 0xFF00) | value;
      main_regs[reg] = value;
      checkKey();
      break;
      
    case REG_SEQUENCE_INPUT:
      handleSequenceInput(value);
      break;
      
    default:
      // Write to register buffer
      if (reg < 64) {
        main_regs[reg] = value;
      }
      break;
  }
}

uint8_t handleRegisterRead(uint8_t reg) {
  // Return from appropriate register bank
  if (reg < 64) {
    return main_regs[reg];
  }
  return 0x00;
}

// ============================================================================
// Challenge Implementations
// ============================================================================

void checkKey() {
  uint16_t expected_key = 0;
  
  switch (game.level) {
    case 0:
      expected_key = KEY_LEVEL1;
      break;
    case 1:
      expected_key = KEY_LEVEL2;
      break;
    case 2:
      expected_key = KEY_LEVEL3;
      break;
    case 3:
      expected_key = MASTER_PASSWORD;
      break;
  }
  
  if (game.current_key == expected_key) {
    main_regs[REG_KEY_VERIFY] = 0x01; // Success!
    
    if (game.level == 0) {
      game.key_found = true;
      Serial.println(F("Level 0: Correct key entered!"));
    } else if (game.level == 3 && !game.vault_unlocked) {
      game.vault_unlocked = true;
      // Decrypt vault data
      for (int i = 0; i < 16; i++) {
        vault_regs[REG_VAULT_DATA + i] = game.vault_data[i];
      }
      Serial.println(F("VAULT UNLOCKED!"));
    }
    
    digitalWrite(GPIO_OUTPUT_PIN, HIGH);
    delay(500);
    digitalWrite(GPIO_OUTPUT_PIN, LOW);
  } else {
    main_regs[REG_KEY_VERIFY] = 0x00; // Wrong
    Serial.print(F("Wrong key: 0x"));
    Serial.println(game.current_key, HEX);
  }
}

void handleSequenceInput(uint8_t value) {
  if (game.sequence_position < REQUIRED_SEQUENCE_LENGTH) {
    game.sequence_buffer[game.sequence_position] = value;
    
    // Check if this byte is correct
    if (value == MAGIC_SEQUENCE[game.sequence_position]) {
      game.sequence_position++;
      main_regs[REG_SEQUENCE_CHECK] = game.sequence_position;
      
      Serial.print(F("Sequence progress: "));
      Serial.print(game.sequence_position);
      Serial.print(F("/"));
      Serial.println(REQUIRED_SEQUENCE_LENGTH);
      
      // Check if complete
      if (game.sequence_position >= REQUIRED_SEQUENCE_LENGTH) {
        game.sequence_complete = true;
        Serial.println(F("SEQUENCE COMPLETE!"));
      }
    } else {
      // Wrong byte - reset
      game.sequence_position = 0;
      main_regs[REG_SEQUENCE_CHECK] = 0;
      Serial.println(F("Sequence reset - wrong byte"));
    }
  }
}

void handleUARTChallenge() {
  static char uart_buffer[32];
  static uint8_t uart_pos = 0;
  
  while (Serial1.available()) {
    char c = Serial1.read();
    
    if (c == '\n' || c == '\r') {
      uart_buffer[uart_pos] = '\0';
      
      // Check password
      if (strcmp(uart_buffer, UART_PASSWORD) == 0) {
        game.uart_unlocked = true;
        Serial1.println(F("UART CHALLENGE COMPLETE!"));
        Serial.println(F("UART password accepted"));
      } else {
        Serial1.println(F("ACCESS DENIED"));
        Serial.print(F("Wrong UART password: "));
        Serial.println(uart_buffer);
      }
      
      uart_pos = 0;
    } else if (uart_pos < 31) {
      uart_buffer[uart_pos++] = c;
    }
  }
}

void checkGPIOChallenge() {
  bool current_state = digitalRead(GPIO_INPUT_PIN);
  
  // Detect falling edge (pulse)
  if (game.last_gpio_state == HIGH && current_state == LOW) {
    game.gpio_pulse_count++;
    
    Serial.print(F("GPIO pulse detected: "));
    Serial.print(game.gpio_pulse_count);
    Serial.print(F("/"));
    Serial.println(GPIO_PULSE_COUNT_REQUIRED);
    
    // Visual feedback
    digitalWrite(GPIO_OUTPUT_PIN, HIGH);
    delay(50);
    digitalWrite(GPIO_OUTPUT_PIN, LOW);
    
    if (game.gpio_pulse_count >= GPIO_PULSE_COUNT_REQUIRED) {
      game.gpio_challenge_done = true;
      Serial.println(F("GPIO CHALLENGE COMPLETE!"));
    }
  }
  
  game.last_gpio_state = current_state;
}

// ============================================================================
// Instructor/Debug Functions
// ============================================================================

void printGameConfiguration() {
  Serial.println(F("Game Configuration:"));
  Serial.println(F("-------------------"));
  Serial.print(F("Level 1 Key: 0x"));
  Serial.println(KEY_LEVEL1, HEX);
  Serial.print(F("Level 2 Key: 0x"));
  Serial.println(KEY_LEVEL2, HEX);
  Serial.print(F("Level 3 Key: 0x"));
  Serial.println(KEY_LEVEL3, HEX);
  Serial.print(F("Master Password: 0x"));
  Serial.println(MASTER_PASSWORD, HEX);
  Serial.print(F("UART Password: "));
  Serial.println(UART_PASSWORD);
  Serial.print(F("GPIO Pulses Required: "));
  Serial.println(GPIO_PULSE_COUNT_REQUIRED);
  Serial.print(F("Magic Sequence: "));
  for (int i = 0; i < REQUIRED_SEQUENCE_LENGTH; i++) {
    Serial.print(F("0x"));
    Serial.print(MAGIC_SEQUENCE[i], HEX);
    if (i < REQUIRED_SEQUENCE_LENGTH - 1) Serial.print(F(", "));
  }
  Serial.println();
  Serial.println();
  Serial.println(F("I2C Addresses:"));
  Serial.print(F("  Main Device: 0x"));
  Serial.println(I2C_ADDR_MAIN, HEX);
  Serial.print(F("  Vault: 0x"));
  Serial.println(I2C_ADDR_VAULT, HEX);
  Serial.print(F("  Secret: 0x"));
  Serial.println(I2C_ADDR_SECRET, HEX);
  Serial.println();
  Serial.println(F("Game ready! Waiting for student..."));
  Serial.println(F("========================================"));
  Serial.println();
}
