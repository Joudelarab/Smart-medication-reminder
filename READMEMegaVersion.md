# LED Control System

## Description
This project uses an Arduino to control LEDs based on scheduled times using a DS1302 RTC (Real-Time Clock) module and a LiquidCrystal I2C LCD. The system supports configuration through buttons and serial input.
The objective is to design and implement a digital drug dispenser system that tracks and displays the current time, allows the users to set and adjust the cureent though push buttons and serial monitor,
allows the user to shedule multiple medication timings 4 timingd for 4 drug containers through push buttons and serial monitor and alerts the user with a buzzer and led when it's time for medication.
## Hardware Requirements
- Arduino board 
- DS1302 RTC module
- LiquidCrystal I2C LCD
- LEDs and resistors
- Buttons for user input
- Buzzer
- Jumper wires
- Breadboard
- Servo motor

## Pin Connections
- DS1302 RTC
  - RST_PIN: 6
  - IO_PIN: 8
  - SCLK_PIN: 7

- Buttons
  - MODE_BUTTON_PIN: 37
  - INCREMENT_BUTTON_PIN: 35
  - CONFIRM_BUTTON_PIN: 33
  - BACK_BUTTON_PIN: 31
  - SET_BUTTON_PIN: 30

- LEDs
  - LED1_PIN: 45
  - LED2_PIN: 44
  - LED3_PIN: 43
  - LED4_PIN: 42
  
- Sevos
  - SERVO1_PIN: 22
  - SERVO2_PIN: 23
  - SERVO3_PIN: 24
  - SERVO4_PIN: 25
  

- LCD
  - LCD_ADDR: 0x27
  - LCD_COLS: 16
  - LCD_ROWS: 2

- Buzzer
  - BUZZER_PIN: 2

## Software Requirements
- Arduino IDE
- DS1302 RTC library
- LiquidCrystal I2C library
- Scheduler library
- Sevo librairy

## Setup
1. Connect the hardware components as described in the pin connections section.
2. Open the Arduino IDE and install the necessary libraries.(LiquidCrystalI2C & DS1302 & Servo)
3. Upload the provided code to your Arduino board. (Don't forget to open all header and source file provided with the .ino file)
4. Open the serial monitor for serial input configuration.

## Configuration
### Button Configuration
- Enter configuration mode using the MODE button:
  Use the increment,decrement to change hour, the back to return to the initial window without saving, the comfirm button will serve to svae hour--minute--next slot... in this order 

- Navigate and set hour and minute values using the INCREMENT and CONFIRM buttons:
  Press set button to enter this mode, use the back button to decrement, the comfirm butoon to change: year--month--hour--hour--minute in this order


### Serial Input Configuration
- Use the serial monitor to enter slot configurations and set the date/time.
- Follow the on-screen prompts to input values.

## Functionality
- The system will activate LED and open servo 90 degree at scheduled times.
- Users can configure the scheduled times and the date/time using buttons or serial input.
- The LCD displays the next scheduled time and the current time.

## License
This project was dine by Joud El Arab and Kevin Aoun. All rights reserved

## Acknowledgments
- This project is part of the CTE 558- Embeded systems course at Faculty of engineering at Sagesse University (Polytech-Beirut) under the instructions of Dr. Clara Zaiter 

