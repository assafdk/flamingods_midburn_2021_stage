/* Example code for HC-SR501 PIR motion sensor with Arduino. More info: www.www.makerguides.com */
#include <Arduino.h>
#include "ledDriver.h"

//#define DEBUG
#define MOTION_TIMEOUT 10000

// Define connection pins:
#define pirPin1 3
#define pirPin2 4
#define pirPin3 5
#define pirPin4 6

#define ledPin 13

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Global variables:
bool val = LOW;
bool val1 = LOW;
bool val2 = LOW;
bool val3 = LOW;
bool val4 = LOW;

unsigned long now, motion_start_time;

bool motionState = false; // We start with no motion detected.

typedef enum {
  LED_IDLE,
  LED_SHOW,
  LED_EASTER,
  LED_FUN
} ledState_t;

ledState_t ledState;

// functions:
void motion_sensor();
ledState_t getSerialCommand();

// external functions
extern void led_setup();
extern void led_run();

// this holds a pointer to the current LED plan
// choose a plan from LED plan functions in "ledDriver.h"
func_ptr_t ledPlan;

void setup() {  
  motion_start_time = 0;
  // Configure the pins as input or output:
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin1, INPUT);
  pinMode(pirPin2, INPUT);
  pinMode(pirPin3, INPUT);
  pinMode(pirPin4, INPUT);
  
  Serial.begin(9600); // open the serial port at 9600 bps
  delay(300);
  
  DEBUG_PRINTLN("DEBUG MODE");
  // put your setup code here, to run once:
  ledState = LED_IDLE;
  led_setup();        // init led stuff
  ledPlan = back_flow; // choose first LED plan
}

void loop() { 
  now = millis();
  // poll motion sensor
  motion_sensor();
  if ((LED_IDLE == ledState) && (true == motionState)) {
    ledState = LED_SHOW;
    motion_start_time = now;
    DEBUG_PRINTLN("Start excitment");
  }

  // Turn off led show after MOTION_TIMEOUT millisec
  if ((LED_SHOW == ledState) && (now-motion_start_time > MOTION_TIMEOUT)) {
    ledState = LED_IDLE;
    motionState = false;
    DEBUG_PRINTLN("End excitment");
  }

  #ifdef DEBUG
  ledState = getSerialCommand();
  #endif
  
  // activate the right LED plan according to the motion detection.
  // LED plan options are:
  // sawtooth, flow, trail, runway, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm
  // confetti - great for NIGHT MODE when you don't invite people to go on stage
  
  switch (ledState) {
    case LED_IDLE:
      ledPlan = back_flow; //sawtooth; // flow;
      break;
    case LED_SHOW:
      ledPlan = rainbow;
      break;
//    case LED_EASTER:
//      ledPlan = confetti;
//      break;
//    case LED_FUN:
//      ledPlan = sinelon;
//      break;
//    default:
//      ledPlan = rainbowWithGlitter;
//      break;
  }
//  DEBUG_PRINT("ledState = ");
//  DEBUG_PRINTLN(ledState);
  
  // run current LED plan
  led_run(ledPlan);
}

#ifdef DEBUG
ledState_t getSerialCommand() {
  char incomingByte;
  // Seial COM event: recieve byte from PC to know when song is over
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    if ((incomingByte < '0') || (incomingByte > '3')) {
      return ledState;
    }
    return (incomingByte - '0');
  }
  return ledState;
}
#endif

void motion_sensor() {
  // Read out the pirPin and store as val:
//  val = digitalRead(pirPin1) || digitalRead(pirPin2) || digitalRead(pirPin3) || digitalRead(pirPin4);
  val1 = digitalRead(pirPin1);
  val2 = digitalRead(pirPin2);
  val3 = digitalRead(pirPin3);
  val4 = digitalRead(pirPin4);

//  if (val1) {DEBUG_PRINT("val1 = "); DEBUG_PRINTLN(val1);}
//  if (val2) {DEBUG_PRINT("val2 = "); DEBUG_PRINTLN(val2);}
//  if (val3) {DEBUG_PRINT("val3 = "); DEBUG_PRINTLN(val3);}
//  if (val4) {DEBUG_PRINT("val4 = "); DEBUG_PRINTLN(val4);}

//  DEBUG_PRINT("val1 = "); DEBUG_PRINTLN(val1);
//  DEBUG_PRINT("val2 = "); DEBUG_PRINTLN(val2);
//  DEBUG_PRINT("val3 = "); DEBUG_PRINTLN(val3);
//  DEBUG_PRINT("val4 = "); DEBUG_PRINTLN(val4);
  
  val = (val1 or val2 or val3 or val4) ? HIGH : LOW;
  // If motion is detected (pirPin = HIGH), do the following:
  if (val == HIGH) {
    digitalWrite(ledPin, HIGH); // Turn on the on-board LED.

    // Change the motion state to true (motion detected):
    if (motionState == false) {
//      Serial.println("Motion detected!");
      motionState = true;
    }
  }

  // If no motion is detected (pirPin = LOW), do the following:
  else {
    digitalWrite(ledPin, LOW); // Turn off the on-board LED.

    // Change the motion state to false (no motion):
    if (motionState == true) {
//      Serial.println("Motion ended!");
      motionState = false;
    }
  }
}
