# Reflex Game Documentation

## Overview
The Reflex Game is an embedded application designed for the LPC17xx microcontroller platform. It tests and measures the user's reaction time by displaying directional commands on an OLED screen and recording how quickly the user responds by moving a joystick in the indicated direction.

## Hardware Requirements
- LPC17xx development board (e.g., LPCXpresso board)
- OLED display
- Joystick
- EEPROM (for storing high scores)
- Speaker/Buzzer (connected to PIN 26 on PORT 0)

## Features
1. **Reaction Time Testing**: Measures how quickly users can respond to visual directional cues
2. **Score Tracking**: Records and displays the user's average reaction time
3. **High Score System**: Stores the best score in EEPROM with checksum validation
4. **Musical Feedback**: Plays a victory tune at the end of each game session
5. **Simple UI**: Clear instructions displayed on OLED screen

## Game Flow
1. **Start Screen**: User is presented with options to start the game or view the best score
2. **Gameplay**: A random direction (UP, DOWN, LEFT, RIGHT) is shown on the display
3. **Reaction**: User must move the joystick in the indicated direction as quickly as possible
4. **Feedback**: After each move, the system displays the reaction time
5. **Game Completion**: After 5 rounds, the average reaction time is calculated and displayed
6. **High Score**: If a new best score is achieved, it's saved to EEPROM and announced

## Technical Implementation

### Initialization Functions
- `init_timer()`: Configures the timer with microsecond precision
- `init_i2c()`: Sets up I2C communication for EEPROM and other peripherals
- `init_adc()`: Configures the Analog-to-Digital Converter
- `init_ssp()`: Initializes the SSP (Synchronous Serial Port) interface
- `joystick_init()`: Prepares the joystick for input reading
- `oled_init()`: Configures the OLED display
- `eeprom_init()`: Initializes the EEPROM storage

### Game Logic
The game operates as a state machine with five states:
1. `GAME_WAIT_START`: Initial state where the game displays instructions
2. `GAME_WAIT_START_MOVE`: Waits for user input to start game or view best score
3. `GAME_SHOW_DIR`: Displays a random direction and starts the reaction timer
4. `GAME_WAIT_MOVE`: Waits for the user to move the joystick in the correct direction
5. `GAME_SHOW_RESULT`: Shows game results after 5 rounds are completed

### Persistent Storage
- Best scores are stored in EEPROM with XOR checksum validation
- Functions `readBestScore()` and `saveBestScore()` handle the EEPROM operations
- If checksum validation fails, a default high score of 1000.0 seconds is used

### Audio System
- Uses simple GPIO toggling to generate square wave tones
- `playNote()`: Generates a tone of specified frequency and duration
- `playSong()`: Interprets a string-based musical notation to play a victory tune
- Musical notation format: Note (A-G, a-g), Duration (0-9), Pause (+,.,_)

### Helper Functions
- `dirToString()`: Converts directional constants to human-readable strings
- `generateRandomDirection()`: Produces a random direction for each round
- `calculateAverageScore()`: Computes the average reaction time from multiple rounds
- `calculateChecksum()`: Creates an XOR checksum for data validation

## Usage Instructions

1. Power on the device and wait for the start screen
2. Move the joystick RIGHT to begin a new game
3. Move the joystick LEFT to view the current best score
4. During gameplay, quickly move the joystick in the direction shown on screen
5. After 5 rounds, your average reaction time will be displayed
6. If you achieve a new best score, it will be saved automatically

## Performance Tips
- For optimal reaction times, hold the device in a comfortable position
- Focus on the screen and be prepared to move the joystick quickly
- Practice regularly to improve your reaction time

## Technical Notes
- Reaction times are measured in seconds with millisecond precision
- The default "no best score" value is set to 1000.0 seconds
- The game plays 5 rounds per session
- If EEPROM data is corrupted (checksum validation fails), the system will reset the best score
