//#define DEBUG

#include <FastLED.h>
#include "ledDriver.h"

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#define LED_CONTROL_PIN_2   5   // MSB
#define LED_CONTROL_PIN_1   6   // LSB

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
  pinMode(LED_CONTROL_PIN_1, INPUT_PULLUP);
  pinMode(LED_CONTROL_PIN_2, INPUT_PULLUP);
  
  Serial.begin(9600); // open the serial port at 9600 bps
  delay(1000);
  DEBUG_PRINTLN("DEBUG MODE");
  // put your setup code here, to run once:
  ledState = LED_IDLE;
  led_setup();        // init led stuff
  ledPlan = runway;   // choose first LED plan
}

void loop() {

  // get current control pins state
  ledState = getLedState();

  //ledState = getSerialCommand();
  
  // activate the right LED plan according to the control pins.
  // LED plan options are:
  // runway, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm
  switch (ledState) {
    case LED_IDLE:
      ledPlan = runway;
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

// AUX functions
ledState_t getLedState() {
    ledState_t ledState;
    ledState_t lsb, msb;
    msb = digitalRead(LED_CONTROL_PIN_2);
    lsb = digitalRead(LED_CONTROL_PIN_1);
    ledState = msb*2+lsb;
    return;
  }

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
