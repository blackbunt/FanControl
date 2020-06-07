#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BAUD_RATE 115200

// Potentiometer Pins
#define POTI1 26
#define POTI2 27
#define POTI3 14
#define POTI4 12

// Fan PWM Pins
#define FAN1_PWM_PIN 2
#define FAN2_PWM_PIN 17
#define FAN3_PWM_PIN 32
#define FAN4_PWM_PIN 33
#define FAN5_PWM_PIN 25

// Touch Control Display
int touch_sensor_value=0;
#define THRESHOLD_UP_TOUCH 70
#define THRESHOLD_DOWN_TOUCH 7
#define NOT_TOUCHED 0
#define TOUCHED 1
int touch_sensor_staus = NOT_TOUCHED;
int touch_state = 0;
int last_touch_state = 0;
int DisplayState = 0;

// poti values
int pot1Value = 0;
int pot2Value = 0;
int pot3Value = 0;
int pot4Value = 0;

// setting PWM properties
const int freq = 25000;                 // freq in Hz
const int fan1 = 0;                     // pwm channel
const int fan2 = 1;
const int fan3 = 2;
const int fan4 = 3;
const int fan5 = 4;
const int resolution = 8;               // resolution in bit

// Variables for Tachometer Signal Reading

unsigned int fan_1_rpm = 0;
unsigned int fan_2_rpm = 0;
unsigned int fan_3_rpm = 0;
unsigned int fan_4_rpm = 0;
unsigned int fan_5_rpm = 0;

// Display Setup
#define SCREEN_WIDTH  128                                       // Display width in pixels
#define SCREEN_HEIGHT  64                                       // Display height in pixels
#define OLED_RESET      4 
#define I2C_ADDRESS_DISPLAY 0x3C                                // Display adress I2C Bus
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int Fan1_percent = 0;
int Fan2_percent = 0;
int Fan3_percent = 0;
int Fan4_percent = 0;

// Functions
int processToucherStatus(int value, int status);



void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Fan Control EPS32");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
    delay(2000);
  }
  
  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS_DISPLAY);
  display.display();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Booting...");
  display.display();
  delay(333);

  // configure LED PWM functionalitites
  ledcSetup(fan1, freq, resolution);
  ledcSetup(fan2, freq, resolution);
  ledcSetup(fan3, freq, resolution);
  ledcSetup(fan4, freq, resolution);
  ledcSetup(fan5, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(FAN1_PWM_PIN, 0);
  ledcAttachPin(FAN2_PWM_PIN, 0);
  ledcAttachPin(FAN3_PWM_PIN, 1);
  ledcAttachPin(FAN4_PWM_PIN, 2);
  ledcAttachPin(FAN5_PWM_PIN, 3);
}

void loop()
{

touch_sensor_value = touchRead(T0);
if (touch_sensor_value <= 60) {
  Serial.println(touch_sensor_value);
}

int touch_state = processToucherStatus(touch_sensor_value, touch_sensor_staus);

// potXValue 0 - 4095
pot1Value = analogRead(POTI1);
pot2Value = analogRead(POTI2);
pot3Value = analogRead(POTI3);
pot4Value = analogRead(POTI4);

// map poti val 2 pwm val
// fan 1 and 2 are the two intake (HDD Bays)
fan_1_rpm = map(pot1Value, 0, 4095, 0, 255);
//fan_2_rpm = map(pot1Value, 0, 4095, 0, 255);
fan_3_rpm = map(pot2Value, 0, 4095, 0, 255);
fan_4_rpm = map(pot3Value, 0, 4095, 0, 255);
fan_5_rpm = map(pot4Value, 0, 4095, 0, 255);

Fan1_percent = map(pot1Value, 0, 4095, 0, 100);
Fan2_percent = map(pot2Value, 0, 4095, 0, 100);
Fan3_percent = map(pot3Value, 0, 4095, 0, 100);
Fan4_percent = map(pot4Value, 0, 4095, 0, 100);

//Serial.println(fan_1_rpm);
//Serial.println(Fan1_percent);

// Display Control with TouchButton
if (touch_state != last_touch_state) {
  //Serial.println("Changed");
  last_touch_state = touch_state;
  
  if (touch_state == TOUCHED && DisplayState == 0) {
    //turn Display on
    DisplayState = 1;
    display.ssd1306_command(SSD1306_DISPLAYON);
    Serial.println("Display powered on.");

  }
  else if (touch_state == TOUCHED && DisplayState == 1) {
    //turn Display off
    DisplayState = 0;
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    Serial.println("Display powered off.");
  }
  
}


display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);

// static Text
display.setCursor(10, 0);
display.print("Fan-Control");

display.setCursor(10, 16);
display.print("CPU: ");
display.setCursor(48, 16);
display.print(Fan1_percent);

display.setCursor(10, 24);
display.print("HDD o: ");
display.setCursor(48, 24);
display.print(Fan2_percent);

display.setCursor(10, 32);
display.print("HDD u: ");
display.setCursor(48, 32);
display.print(Fan3_percent);

display.setCursor(10, 40);
display.print("SSD: ");
display.setCursor(48, 40);
display.print(Fan4_percent);


display.display();


// Set Fan PWM
ledcWrite(0, fan_1_rpm);

ledcWrite(1, fan_3_rpm);
ledcWrite(2, fan_4_rpm);
ledcWrite(3, fan_5_rpm);

delay(100);

}

// hint: status -> touch_sensor_value
// and: value -> touchRead(T0)
int processToucherStatus(int value, int status) {
  if (status == NOT_TOUCHED && value <= THRESHOLD_DOWN_TOUCH)
  {
    status = TOUCHED;
  }
  if (status == TOUCHED && value >= THRESHOLD_UP_TOUCH)
  {
    status = NOT_TOUCHED;
  }

  return status;
}
