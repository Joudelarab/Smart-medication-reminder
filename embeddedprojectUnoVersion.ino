#include <DS1302.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "scheduler.h"

// Pin definitions
#define RTC_RST_PIN 8
#define RTC_IO_PIN 10
#define RTC_SCLK_PIN 9

#define MODE_BUTTON_PIN 3
#define INCREMENT_BUTTON_PIN 4
#define CONFIRM_BUTTON_PIN 5
#define BACK_BUTTON_PIN 6

#define LED1_PIN 10
#define LED2_PIN 11
#define LED3_PIN 12
#define LED4_PIN 13

#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#define BUZZER_PIN 2
#define SET_BUTTON_PIN A0 // Define pin for set button


// RTC and LCD initialization
DS1302 rtc(RTC_RST_PIN, RTC_IO_PIN, RTC_SCLK_PIN);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);


// Storage for scheduled times
int ledHours[4] = {6, 12, 18, 22};  // Default times for slots
int ledMinutes[4] = {0, 0, 0, 0};   // Default times for slots

int selectedSlot = 0;       // Currently selected slot (0 to 3)
bool isConfigMode = false;  // Flag for configuration mode
bool isConfigMinutes = false; // Toggle between hour and minute configuration
bool isSettingDateTime = false; // Flag for date and time configuration mode

int hour ;   // Global declaration
int minute; // Global declaration
int second;
int year ;
int month;
int day ;
bool initialized = false; // Global initialization flag

String serialInput = ""; // Store serial input



void setup() {
  Serial.begin(9600);//Initialize Serial communication
  lcd.init();//Initialize LCD
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  Serial.println("Initializing...");
  delay(2000);

  //Pin modes
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INCREMENT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CONFIRM_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SET_BUTTON_PIN, INPUT_PULLUP);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

    //set initial date and time
  char monthStr[4]; // Array to store the month abbreviation 
  sscanf(__TIME__,"%d:%d:%d",&hour,&minute,&second);// take the current time from laptop
  rtc.setDate(day, month, year);
  sscanf(__DATE__,"%s %d %d",monthStr,&day,&year);//take the current date from laptop
  rtc.setTime(hour, minute, second);
if (strcmp(monthStr, "Jan") == 0) month = 1;
else if (strcmp(monthStr, "Feb") == 0) month = 2;
else if (strcmp(monthStr, "Mar") == 0) month = 3;
else if (strcmp(monthStr, "Apr") == 0) month = 4;
else if (strcmp(monthStr, "May") == 0) month = 5;
else if (strcmp(monthStr, "Jun") == 0) month = 6;
else if (strcmp(monthStr, "Jul") == 0) month = 7;
else if (strcmp(monthStr, "Aug") == 0) month = 8;
else if (strcmp(monthStr, "Sep") == 0) month = 9;
else if (strcmp(monthStr, "Oct") == 0) month = 10;
else if (strcmp(monthStr, "Nov") == 0) month = 11;
else if (strcmp(monthStr, "Dec") == 0) month = 12;

  //clear lcd an display ready message to know that we've done initializion
  lcd.clear();
  lcd.print("Ready");
  Serial.println("Ready");
  delay(2000);

  // Display menu after initialization
  displayMenu();

  //initilaize scheduler
  scheduler_init(millis);

  //create task handle for scheduler
  s_task_handle_t normalTaskHandle;
  s_task_handle_t configModeTaskHandle;
  s_task_handle_t menuTaskHandle;
  s_task_handle_t dateTimeConfigTaskHandle;

  //create tasks
 s_task_create(true, S_TASK_NORMAL_PRIORITY, 500, taskDateTimeConfig, &dateTimeConfigTaskHandle, NULL); 
  s_task_create(true, S_TASK_NORMAL_PRIORITY, 1000, taskNormalOperation, &normalTaskHandle, NULL);
  s_task_create(true, S_TASK_NORMAL_PRIORITY, 1000, taskEnterConfigMode, &configModeTaskHandle, NULL);
  s_task_create(true, S_TASK_NORMAL_PRIORITY, 500, taskMenuOperation, &menuTaskHandle, NULL); // Add task for menu
}

void loop() {
  scheduler();
}

// Display the next slot based on current time and the current time
void displayNextSlotAndCurrentTime() {
  Time now = rtc.getTime();
  int nextSlot = getNextSlot(now.hour, now.min);

  lcd.setCursor(0, 0);
  lcd.print("Next: ");
  lcd.print(nextSlot + 1);
  lcd.print(" ");
  lcd.print(ledHours[nextSlot] < 10 ? "0" : "");
  lcd.print(ledHours[nextSlot]);
  lcd.print(":");
  lcd.print(ledMinutes[nextSlot] < 10 ? "0" : "");
  lcd.print(ledMinutes[nextSlot]);
  lcd.print("  ");

  lcd.setCursor(0, 1);
  lcd.print("Now: ");
  lcd.print(now.hour < 10 ? "0" : "");
  lcd.print(now.hour);
  lcd.print(":");
  lcd.print(now.min < 10 ? "0" : "");
  lcd.print(now.min);
  lcd.print(":");
  lcd.print(now.sec < 10 ? "0" : "");
  lcd.print(now.sec);

  delay(500); // Refresh every half second
}

// Get the next slot index based on current time
int getNextSlot(int currentHour, int currentMinute) {
  for (int i = 0; i < 4; i++) {
    if ((ledHours[i] > currentHour) || 
        (ledHours[i] == currentHour && ledMinutes[i] > currentMinute)) {
      return i;
    }
  }
  return 0; // Return the first slot if all times have passed
}

// Handle normal operation (activate LEDs when time matches)
void handleNormalOperation() {
  Time now = rtc.getTime();
  bool ledActivated = false;
  for (int i = 0; i < 4; i++) {
    if (now.hour == ledHours[i] && now.min == ledMinutes[i]) {
      activateLED(i);
      ledActivated = true;
    }
  }
  if (ledActivated) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(5000); // LED ON duration (5 seconds)
    digitalWrite(BUZZER_PIN, LOW);
  } 
}

// Activate the corresponding LED and 
void activateLED(int slot) {
  int ledPin;

  switch (slot) {
    case 0: 
      ledPin = LED1_PIN; 
      break;
    case 1: 
      ledPin = LED2_PIN;
      break;
    case 2: 
      ledPin = LED3_PIN; 
      break;
    case 3: 
      ledPin = LED4_PIN; 
      break;
  }

  digitalWrite(ledPin, !digitalRead(ledPin);// we change in this code for optimiztion
  delay(5000); // LED ON duration (5 seconds)
}

// Task for normal operation
void taskNormalOperation(s_task_handle_t, s_task_msg_t**, void*) {
  if (!isConfigMode && !isSettingDateTime) {
    displayNextSlotAndCurrentTime();
    handleNormalOperation();

    if (digitalRead(SET_BUTTON_PIN) == LOW) { // Use SET_BUTTON_PIN to enter date/time setting mode
      isSettingDateTime = true;
      lcd.clear();
      lcd.print("Set Date/Time");
      delay(1000);
    }
  }
}


// Task to enter configuration mode
void taskEnterConfigMode(s_task_handle_t, s_task_msg_t**, void*) {
  if (isConfigMode) {
    if (isSettingDateTime) {
      handleDateTimeConfig(); // Handle date and time configuration mode
    } else {
      handleConfigMode(); // Handle slot configuration mode
    }
  }
}

// Task for menu operation
void taskMenuOperation(s_task_handle_t, s_task_msg_t**, void*) {
  handleSerialInput();
}

// Handle configuration mode
void handleConfigMode() {
  // Initialize hour and minute only once when entering config mode
  if (!initialized) {
    hour = ledHours[selectedSlot];
    minute = ledMinutes[selectedSlot];
    initialized = true;
  }

  lcd.setCursor(0, 0);
  lcd.print("Slot ");
  lcd.print(selectedSlot + 1);
  lcd.setCursor(0, 1);

  if (!isConfigMinutes) {
    lcd.print("Hour: ");
    lcd.print(hour < 10 ? "0" : "");
    lcd.print(hour);
    lcd.print("   ");
  } else {
    lcd.print("Minute: ");
    lcd.print(minute < 10 ? "0" : "");
    lcd.print(minute);
    lcd.print("   ");
  }
//handle button for configuration mode| define each butoon functionality when enter configuration mode
  handleButtonPress(MODE_BUTTON_PIN, changeSlot);
  handleButtonPress(INCREMENT_BUTTON_PIN, incrementValue);
  handleButtonPress(CONFIRM_BUTTON_PIN, confirmValue);
  handleButtonPress(BACK_BUTTON_PIN, exitConfigMode);
}

// Handle button presses
void handleButtonPress(int buttonPin, void (*action)()) {
  static unsigned long lastPressTime = 0;
  unsigned long currentTime = millis();

  if (digitalRead(buttonPin) == LOW && (currentTime - lastPressTime > 200)) { // Debounce
    lastPressTime = currentTime;
    action(); // Execute the assigned action
  }
}

// Change the slot
void changeSlot() {
  selectedSlot = (selectedSlot + 1) % 4;
  lcd.clear();
  lcd.print("Slot ");
  lcd.print(selectedSlot + 1);
  delay(1000);

  // Reinitialize hour and minute for the new slot
  hour = ledHours[selectedSlot];
  minute = ledMinutes[selectedSlot];
}

// Increment the value
void incrementValue() {
  if (!isConfigMinutes) { // Configuring hours
    hour = (hour + 1) % 24; // Increment hour by 1, wrap around after 23
  } else { // Configuring minutes
    minute = (minute + 10) % 60; // Increment minute by 10, wrap around after 59
  }

  lcd.setCursor(0, 1);
  if (!isConfigMinutes) {
    lcd.print("Hour: ");
    lcd.print(hour < 10 ? "0" : "");
    lcd.print(hour);
    lcd.print("   ");
  } else {
    lcd.print("Minute: ");
    lcd.print(minute < 10 ? "0" : "");
    lcd.print(minute);
    lcd.print("   ");
  }
}

// Confirm the value
void confirmValue() {
  if (!isConfigMinutes) {
    isConfigMinutes = true; // Move to configuring minutes
    Serial.println("Switched to minute configuration.");
  } else {
    // Save the hour and minute for the selected slot
    ledHours[selectedSlot] = hour;
    ledMinutes[selectedSlot] = minute;
    Serial.println("Slot " + String(selectedSlot + 1) + " saved: " + String(hour) + ":" + String(minute));
    lcd.clear();
    lcd.print("Saved Slot ");
    lcd.print(selectedSlot + 1);
    delay(2000);
    isConfigMode = false; // Exit configuration mode
    initialized = false; // Reset initialization flag
  }
}

// Exit configuration mode
void exitConfigMode() {
  lcd.clear();
  lcd.print("Exiting Config");
  delay(1000);
  isConfigMode = false;
}

// Handle serial input
void handleSerialInput() {
  while (Serial.available()) {
    char input = Serial.read();
    Serial.print("Received: "); // Debugging line
    Serial.println(input); // Debugging line
    if (input == '\n' || input == '\r') { // Check for end of input
      processSerialCommand(serialInput);
      serialInput = ""; // Clear input
    } else {
      serialInput += input; // Append character to input string
    }
  }
}

// Process serial commands
void processSerialCommand(String command) {
  command.trim(); // Trim any whitespace
  char cmd = command.charAt(0); // Get the first character of the command

  switch (cmd) {
    case '1':
    case '2':
    case '3':
    case '4': {
      selectedSlot = cmd - '1'; // Convert '1'-'4' to slot index 0-3
      Serial.print("Enter hour (00-23): ");
      
      while (Serial.available() == 0); // Wait for input
      String hourStr = Serial.readStringUntil('\n');
      int hour = hourStr.toInt();
      Serial.println(hour); // Display the entered hour

      Serial.print("Enter minute (00-59): ");
      while (Serial.available() == 0); // Wait for input
      String minuteStr = Serial.readStringUntil('\n');
      int minute = minuteStr.toInt();
      Serial.println(minute); // Display the entered minute

      if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
        setSlot(selectedSlot, hour, minute);
        Serial.println("Slot " + String(selectedSlot + 1) + " saved: " + String(hour) + ":" + String(minute));
      } else {
        Serial.println("Invalid time format. Please enter a valid time (00:00 - 23:59).");
      }
      break;
    }
    case '5':
      setDateTimeSerial();
      break;
    case 'E':
    case 'e':
      Serial.println("Exiting without saving.");
      break;
    default:
      Serial.println("Invalid choice. Please enter a valid option.");
      break;
  }

  displayMenu(); // Redisplay the menu after processing the command
}

// Function to display the menu
void displayMenu() {
  Serial.println("\n===== Menu =====");
  Serial.println("1. Configure Slot 1");
  Serial.println("2. Configure Slot 2");
  Serial.println("3. Configure Slot 3");
  Serial.println("4. Configure Slot 4");
  Serial.println("5. Set Date and Time");
  Serial.println("E. Exit");
  Serial.println("=================");
  Serial.print("Enter your choice: ");
}

// Set slot time
void setSlot(int slot, int hour, int minute) {
  ledHours[slot] = hour;
  ledMinutes[slot] = minute;
  lcd.clear();
  lcd.print("Slot ");
  lcd.print(slot + 1);
  lcd.print(" set to ");
  lcd.print(hour < 10 ? "0" : "");
  lcd.print(hour);
  lcd.print(":");
  lcd.print(minute < 10 ? "0" : "");
  lcd.print(minute);
  lcd.print("  ");
  delay(1000); // Display for a second
  Serial.println("Slot " + String(slot + 1) + " set to " + String(hour) + ":" + String(minute));
}

// Function to set date and time via serial
void setDateTimeSerial() {
  Serial.print("Enter year (YYYY): ");
  while (Serial.available() == 0); // Wait for input
  String yearStr = Serial.readStringUntil('\n');
  int year = yearStr.toInt();
  Serial.println(year);

  Serial.print("Enter month (1-12): ");
  while (Serial.available() == 0); // Wait for input
  String monthStr = Serial.readStringUntil('\n');
  int month = monthStr.toInt();
  Serial.println(month);

  Serial.print("Enter day (1-31): ");
  while (Serial.available() == 0); // Wait for input
  String dayStr = Serial.readStringUntil('\n');
  int day = dayStr.toInt();
  Serial.println(day);

  Serial.print("Enter hour (0-23): ");
  while (Serial.available() == 0); // Wait for input
  String hourStr = Serial.readStringUntil('\n');
  int hour = hourStr.toInt();
  Serial.println(hour);

  Serial.print("Enter minute (0-59): ");
  while (Serial.available() == 0); // Wait for input
  String minuteStr = Serial.readStringUntil('\n');
  int minute = minuteStr.toInt();
  Serial.println(minute);

  Serial.print("Enter second (0-59): ");
  while (Serial.available() == 0); // Wait for input
  String secondStr = Serial.readStringUntil('\n');
  int second = secondStr.toInt();
  Serial.println(second);

  // Set the RTC with the new date and time
  rtc.setDOW(0); // Ignore the day of the week for simplicity
  rtc.setTime(hour, minute, second);
  rtc.setDate(day, month, year);

  Serial.println("Date and time set successfully.");
}

void handleDateTimeConfig() {
  static int settingStep = 0; // Track the current step (0: year, 1: month, ..., 5: second)

  lcd.setCursor(0, 0);
  lcd.print("Set Date/Time");

  switch (settingStep) {
    case 0: // Year
    lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Year: ");
      lcd.print(year);
      handleButtonPress(INCREMENT_BUTTON_PIN, incrementYear);
      handleButtonPress(BACK_BUTTON_PIN, decrementYear);
      break;
    case 1: // Month
    lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Month: ");
      lcd.print(month);
      handleButtonPress(INCREMENT_BUTTON_PIN, incrementMonth);
      handleButtonPress(BACK_BUTTON_PIN, decrementMonth);
      break;
    case 2: // Day
    lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Day: ");
      lcd.print(day);
      handleButtonPress(INCREMENT_BUTTON_PIN, incrementDay);
      handleButtonPress(BACK_BUTTON_PIN, decrementDay);
      break;
    case 3: // Hour
    lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Hour: ");
      lcd.print(hour);
      handleButtonPress(INCREMENT_BUTTON_PIN, incrementHour);
      handleButtonPress(BACK_BUTTON_PIN, decrementHour);
      break;
    case 4: // Minute
    lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Minute: ");
      lcd.print(minute);
      handleButtonPress(INCREMENT_BUTTON_PIN, incrementMinute);
      handleButtonPress(BACK_BUTTON_PIN, decrementMinute);
      break;
    case 5: // Second
    lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Second: ");
      lcd.print(second);
      handleButtonPress(INCREMENT_BUTTON_PIN, incrementSecond);
      handleButtonPress(BACK_BUTTON_PIN, decrementSecond);
      break;
  }

  // Move to the next setting step
  handleButtonPress(CONFIRM_BUTTON_PIN, []() {
    settingStep = (settingStep + 1) % 6;
    if (settingStep == 0) {
      // Save the date and time after all steps are completed
      //rtc.setDOW(0); // Use 0 for Sunday for simplicity
      rtc.setTime(hour, minute, second);
      rtc.setDate(day, month, year);
      lcd.clear();
      lcd.print("Date/Time Set");
      delay(2000);
      isSettingDateTime = false; // Exit date/time configuration mode
    }
  });
}

// Functions to handle increment and decrement actions for date and time setting
void incrementYear() { year++;}
void decrementYear() { year--; }

void incrementMonth() { month = (month % 12) + 1; }
void decrementMonth() { month = (month - 1 == 0 ? 12 : month - 1); }

void incrementDay() { day = (day % 31) + 1; }
void decrementDay() { day = (day - 1 == 0 ? 31 : day - 1); }

void incrementHour() { hour = (hour + 1) % 24; }
void decrementHour() { hour = (hour - 1 == -1 ? 23 : hour - 1); }

void incrementMinute() { minute = (minute + 1) % 60; }
void decrementMinute() { minute = (minute - 1 == -1 ? 59 : minute - 1); }

void incrementSecond() { second = (second + 1) % 60; }
void decrementSecond() { second = (second - 1 == -1 ? 59 : second - 1); }

// Task to handle date/time configuration mode
void taskDateTimeConfig(s_task_handle_t, s_task_msg_t**, void*) {
  if (isSettingDateTime) {
    handleDateTimeConfig(); // Handle date and time configuration mode
  }
}
