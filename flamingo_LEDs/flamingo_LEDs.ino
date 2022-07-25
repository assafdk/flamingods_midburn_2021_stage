/* Example code for HC-SR501 PIR motion sensor with Arduino. More info: www.www.makerguides.com */
#include <Arduino.h>
#include "ledDriver.h"

//#define DEBUG
#define jumperPin 8
#define ledPin 13

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Global variables:

unsigned long now, party_start_time;

bool jumperState = HIGH;  // HIGH - Jumper is disconnected. LOW - Jumper is connected

typedef enum {
  LED_IDLE,
  LED_SHOW,
  LED_EASTER,
  LED_FUN
} ledState_t;

ledState_t ledState;

// functions:
ledState_t getSerialCommand();

// external functions
extern void led_setup();
extern void led_run();

// this holds a pointer to the current LED plan
// choose a plan from LED plan functions in "ledDriver.h"
func_ptr_t ledPlan;

void setup() {  
  party_start_time = 0;
  // Configure the pins as input or output:
  pinMode(ledPin, OUTPUT);
  pinMode(jumperPin, INPUT_PULLUP);
  
  Serial.begin(9600); // open the serial port at 9600 bps
  delay(300);
  
  DEBUG_PRINTLN("DEBUG MODE");
  // put your setup code here, to run once:
  ledState = LED_IDLE;
  led_setup();        // init led stuff
  ledPlan = bpm;  // choose first LED plan
}

void loop() { 
  
  now = millis();
  
  // poll jumper
  jumper_polling();
  
//  
//  #ifdef DEBUG
//    ledState = getSerialCommand();
//  #endif
//  
//
//  if ((LED_IDLE == ledState) && (true == motionState)) {
//    ledState = LED_SHOW;
//    motion_start_time = now;
//    DEBUG_PRINTLN("Start excitment");
//  }
//
//  // Turn off led show after MOTION_TIMEOUT millisec
//  if ((LED_SHOW == ledState) && (now-motion_start_time > MOTION_TIMEOUT)) {
//    ledState = LED_IDLE;
//    motionState = false;
//    DEBUG_PRINTLN("End excitment");
//  }
  
  // activate the right LED plan according to jumper state.
  // LED plan options are:
  // sawtooth, flow, trail, runway, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm
  // confetti - great for NIGHT MODE when you don't invite people to go on stage
  
  switch (ledState) {
    case LED_IDLE:
      // jumper is disconnected
      EVERY_N_SECONDS( 1 ) { DEBUG_PRINTLN("LED_IDLE"); }
//      ledPlan = rainbow; //sawtooth; // flow;
<<<<<<< HEAD
      ledPlan = bpm;
=======
      ledPlan = all_pink;
>>>>>>> 3d9cec259824c4588052b90225f9a5f384651ebf
      // run current LED plan
      led_run(ledPlan);
      break;
    case LED_FUN:
      // jumper is connected
      EVERY_N_SECONDS( 1 ) { DEBUG_PRINTLN("LED_FUN"); }
      //ledPlan = flickering_rainbow;;
      //led_run(juggle);
      led_multiplan();
      break;
  }
  EVERY_N_SECONDS( 1 ) { DEBUG_PRINT("ledState = "); DEBUG_PRINTLN(ledState);}
}

void jumper_polling() {
  // Read out the jumperPin
  jumperState = digitalRead(jumperPin);

  if (LOW == jumperState) {
    // Party mode
    // Circulate between the different LED plans
    ledState = LED_FUN;
  }
  if (HIGH == jumperState) {
    // idle mode
    // static rainbow
    ledState = LED_IDLE;
  }
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
