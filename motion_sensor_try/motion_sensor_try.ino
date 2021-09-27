/* Example code for HC-SR501 PIR motion sensor with Arduino. More info: www.www.makerguides.com */
//#define DEBUG
#define DELAY 25

#include <FastLED.h>
#include "ledDriver.h"

// Define connection pins:
#define pirPin 2
#define ledPin 13

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Global variables:
int val = 0;
bool motionState = false; // We start with no motion detected.

typedef enum {
  LED_IDLE,
  LED_SHOW,
  LED_EASTER,
  LED_FUN
} ledState_t;

ledState_t ledState;

// external functions
extern void led_setup();
extern void led_run();

// this holds a pointer to the current LED plan
// choose a plan from LED plan functions in "ledDriver.h"
func_ptr_t ledPlan;

void setup() {  
  // Configure the pins as input or output:
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  
  Serial.begin(9600); // open the serial port at 9600 bps
  delay(300);
  
  DEBUG_PRINTLN("DEBUG MODE");
  // put your setup code here, to run once:
  ledState = LED_IDLE;
  led_setup();        // init led stuff
  ledPlan = flow; // choose first LED plan
}

void loop() { 
  // poll motion sensor
  motion_sensor();
  if (motionState) {
    ledState = LED_SHOW;
  } else {
    ledState = LED_IDLE;
  }
  delay(DELAY);
  
  #ifdef DEBUG
  ledState = getSerialCommand();
  #endif
  
  // activate the right LED plan according to the motion detection.
  // LED plan options are:
  // sawtooth, flow, trail, runway, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm
  // confetti - great for NIGHT MODE when you don't invite people to go on stage
  
  switch (ledState) {
    case LED_IDLE:
      ledPlan = flow; //sawtooth; // flow;
      break;
    case LED_SHOW:
      ledPlan = rainbow;
      break;
    case LED_EASTER:
      ledPlan = confetti;
      break;
    case LED_FUN:
      ledPlan = sinelon;
      break;
    default:
      ledPlan = rainbowWithGlitter;
      break;
  }
  DEBUG_PRINT("ledState = ");
  DEBUG_PRINTLN(ledState);
  
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
  val = digitalRead(pirPin);

  // If motion is detected (pirPin = HIGH), do the following:
  if (val == HIGH) {
    digitalWrite(ledPin, HIGH); // Turn on the on-board LED.

    // Change the motion state to true (motion detected):
    if (motionState == false) {
      Serial.println("Motion detected!");
      motionState = true;
    }
  }

  // If no motion is detected (pirPin = LOW), do the following:
  else {
    digitalWrite(ledPin, LOW); // Turn off the on-board LED.

    // Change the motion state to false (no motion):
    if (motionState == true) {
      Serial.println("Motion ended!");
      motionState = false;
    }
  }
}
