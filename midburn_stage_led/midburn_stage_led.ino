//#define DEBUG
#define I2C

#include <FastLED.h>
#include "ledDriver.h"
#include <Wire.h>

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#ifdef I2C
  // I2C pins 
  //  UNO -- pullup 10k resistors needed
  //    SDA A4
  //    SCL A5
  //
  //  MEGA -- no need. there's built-in pullups
  //    SDA 20
  //    SCL 21
  
  // define slave address (need to match the address in the other Arduino)
  #define I2C_SLAVE_ADDR 9

  // define slave answer size
  #define I2C_ANSWER_SIZE 1
#else
  // Use direct connection with no protocol
  #define LED_CONTROL_PIN_2   5   // MSB
  #define LED_CONTROL_PIN_1   6   // LSB
#endif

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
  setupLedCom(); // I2C or direct
  
  Serial.begin(9600); // open the serial port at 9600 bps
  delay(1000);
  
  DEBUG_PRINTLN("DEBUG MODE");
  // put your setup code here, to run once:
  ledState = LED_IDLE;
  led_setup();        // init led stuff
  ledPlan = flow; //sawtooth; //flow;   // choose first LED plan
}

void loop() {

  #ifndef I2C
  // poll for current control pins state if not using I2C
  ledState = getLedState();
  #endif

  #ifdef DEBUG
  ledState = getSerialCommand();
  #endif
  
  // activate the right LED plan according to the control pins.
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

#ifndef I2C
// AUX functions
ledState_t getLedState() {
    ledState_t ledState;
    ledState_t lsb, msb;
    msb = digitalRead(LED_CONTROL_PIN_2);
    lsb = digitalRead(LED_CONTROL_PIN_1);
    ledState = msb*2+lsb;
    return;
  }
#endif

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

// function to run when data is recived from master
void i2cReveiveEvent() {
    ledState_t ls;
    // read while data received
    while (0 < Wire.available()) {
      ls = (ledState_t)Wire.read();
    }
    Serial.println (ls);
    ledState = ls;
    return;
}

//// function to run when data is recived from master
//void i2cReveiveEvent() {
//  // read while data received
//    while (0 < Wire.available()) {
//      byte xx = Wire.read();
//    }
//    Serial.println (xx);
//    return;
//}

// function to run when data is requested by master
void i2cRequestEvent() {
    Wire.write(ledState);    
    Serial.println (ledState);
    return;
}

void setupLedCom() {
  #ifdef I2C
    // I2C
    // init I2C communication as slave
    Wire.begin(I2C_SLAVE_ADDR);
  
    // function to run when data is requested by master
    Wire.onRequest(i2cRequestEvent);
  
    // function to run when data is recived from master
    Wire.onReceive(i2cReveiveEvent);
  #else
    pinMode(LED_CONTROL_PIN_1, INPUT_PULLUP);
    pinMode(LED_CONTROL_PIN_2, INPUT_PULLUP);
  #endif    
  return;
}
