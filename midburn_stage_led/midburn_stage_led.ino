//#define DEBUG

#include <FastLED.h>
#include "ledDriver.h"

extern void led_setup();
extern void led_run();

// this holds a pointer to the current LED plan
// choose a plan from LED plan functions in "ledDriver.h"
func_ptr_t ledPlan;

void setup() {
  // put your setup code here, to run once:

  led_setup();        // init led stuff
  ledPlan = runway;  // choose first LED plan
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // LED handling
  led_run(ledPlan);

  // random LED plans that are directed towards into the stage
  //ledPlan = runway;

  //ledPlan = confetti;
}
