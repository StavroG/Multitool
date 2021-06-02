/*
 * Author: Stavro Gorou 
 * GitHub: https://github.com/StavroG
 * Date (MM/DD/YY): 06/01/21
 * Materials: Arduino Nano, HC-SRO4 sensor, MPU6050 sensor, laser diode, 128x32 OLED display, 10k Ohm resistor, 4pin push button
 * Description: A Multitool that can check the distance of an object using an ultrasonic sound sensor, and can also check how level a surface is using an accelerometer/gyroscope sensor. 
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>

const int OLED_RESET_PIN = 4; //Reset pin number for the OLED display
const int SCREEN_ADDRESS = 0x3C;  //Address for the screen. 0x3c for 128x32 and 0x3d for 128x64
const int BUTTON_PIN = 6; //Pin that button is connected to 
const int LASER_PIN = 10; //Pin that laser is connected to
const int ECHO_PIN = 2; //ECHO pin from HC-SR04 sensor
const int TRIG_PIN = 3; //TRIG pin from HC-SR04 sensor

boolean oldState = false; //Last state from button
boolean newState = false; //New state from button
boolean isRangeTool = false;  //Is range tool selected

long time = 0;  //Start at 0 to check how long last button was clicked
long debounce = 100;  //Time between every possible button click. If button press is flickering increase debounce.

Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET_PIN);
MPU6050 mpu;

void setup() {  //Only called once when new sketch starts
  Serial.begin(9600); // Serial Communication is starting with 9600 of baudrate speed
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {  //Prints an error message if OLED display is not found
    Serial.println("SSD1306 allocation failed");
  }
  if (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {  //Prints an error message if mpu6050 is not found
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
  }
  Serial.end(); //Ends serial communication
  
  display.display();  //Default screen for the OLED display
  display.setTextColor(SSD1306_WHITE);  //Sets the text color for display
  
  pinMode(TRIG_PIN, OUTPUT); // Sets the TRIG Pin as an OUTPUT
  pinMode(ECHO_PIN, INPUT); // Sets the ECHO Pin as an INPUT
  pinMode(BUTTON_PIN, INPUT); //Sets the BUTTON pin as an INPUT
  pinMode(LASER_PIN, OUTPUT); //Sets LASER pin as OUTPUT
}

void loop() { //Loops forever
  newState = digitalRead(BUTTON_PIN); //Reads input from the button

  if ((millis() - time) > debounce) { //Checks if last time button was pressed is greater than the debounce time
    if (newState) { //Toggle is turned on
      isRangeTool = (oldState == true); //Sets isRangedTool by checking if toggle was on or off before
      oldState = !oldState; //Flips the toggle of the old state
      time = millis();  //Resets the counter for debouncing
    }
  
    if (isRangeTool) {  //If the toggle is set to range tool turn on range tool
      rangeTool();
    }
    else {  //If toggle is not set to range tool then turn on leveler tool
      levelerTool();
    }
  }
}

void rangeTool() {
  digitalWrite(LASER_PIN, true); //Turns on the laser
  
  // Clears the trigPin condition
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculating the distance in cm
  int distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

  //Displays the distance on the OLED display
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Distance: ");
  display.print(distance * 0.393701 + 1);  //Convert the distance from cm to in and add 1 in offset
  display.println(" in");
  display.display();
}

void levelerTool() {
  digitalWrite(LASER_PIN, false);  //Turns off the laser
  
  Vector level = mpu.readNormalizeAccel();  //Vector of the normalized accelorometer values
  
  // Calculate Pitch & Roll
  int pitch = -(atan2(level.XAxis, sqrt(level.YAxis*level.YAxis + level.ZAxis*level.ZAxis))*180.0)/M_PI;
  int roll = (atan2(level.YAxis, level.ZAxis)*180.0)/M_PI;

  //Displays the pitch and roll on the OLED display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("Pitch: ");
  display.println(pitch);
  display.print("Roll: ");
  display.println(roll);
  display.display();
}
