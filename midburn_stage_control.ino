//#define DEBUG

#include <FastLED.h>
#include "pushButtonDriver.h"
#include "ledDriver.h"

extern void led_setup();
extern void led_run();

extern unsigned long now; // this is the time the button is pushed

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

typedef enum {
  IDLE_STATE,
  SHOW_STATE,
  EASTER_STATE   
} state_t;
state_t cur_state, prev_state;


// ---------------------- Stage control ----------------------
// -- constants --
#define BUTTON_DELAY_MS  50      // push button debouncibg delay in millisec

#define SHOW_COLOR_LIGHTS_1_TIME  500
#define SHOW_COLOR_LIGHTS_2_TIME  1000
#define SHOW_WHITE_LIGHTS_TIME    1500
#define SHOW_SONG_PLAY_TIME       1500
#define SHOW_BUBBLE_TIME          2000
#define SHOW_SMOKE_TIME           2000

#define SHOW_SMOKE_DURATION       4000
#define SHOW_SMOKE_REPEAT_TIME    40000

//#define SHOW_BUBBLE_DURATION    10000
#define SHOW_TIMEOUT_MS         (60000*6) // 6 minutes

#define EASTER_ENABLE_TIME        (SHOW_SONG_PLAY_TIME)
#define EASTER_SMOKE_DURATION     4000    
#define EASTER_FLICKERS_DURATION  15000 
#define EASTER_TIMEOUT_MS         (EASTER_SMOKE_DURATION + EASTER_FLICKERS_DURATION)

//// pins definition MEGA
//#define WHITE_LIGHTS    41
//#define COLOR_LIGHTS    43
//#define BUBBLE_MACHINE  45
//#define SMOKE_MACHINE   47
//#define FLICKERS        39

// pins definition UNO
#define WHITE_LIGHTS    8
#define COLOR_LIGHTS_1  9
#define COLOR_LIGHTS_2  7
#define BUBBLE_MACHINE  10
#define SMOKE_MACHINE   11
#define FLICKERS        12

#define SERIAL_PLAY_SONG  'P' // Play
#define SERIAL_STOP_SONG  'S' // Stop
//#define SERIAL_CLAP_FX    'C' // Clapping effect
//#define SERIAL_DRUMS_FX   'D' // Drums effect
#define SERIAL_VOCAL_WOW_FX     'V' // Vocal wow effect for easter
#define SERIAL_ZOTI_WOW_FX     'Z' // Vocal wow effect for easter

#define ON  HIGH
#define OFF LOW
// ---------------

// -- functions declaration --
// ---------------------------
state_t get_new_state();
event_t getEvent();

// states
void idle_setup();
void idle_state();
void show_setup();
void show_state();
void easter_setup();
void easter_state();
void easter_exit();

// transition
state_t get_new_state(state_t prev_state, event_t event); // state machine function
void transition_output(state_t prev_state, state_t cur_state, event_t event); // transitions between states

// AUX functions
void turnAllRelaysOff();
void relayToggle(int pin_number,int desiredOutput);
// ---------------------------


// ------- globals ----------
unsigned long showTime;         // counting ms into each show
unsigned long showStartTime;    // capturing the show start moment
unsigned long lastSmokeTime;    // relative to (counting from) showStartTime
unsigned long smokeStartTime;   // capturing the absolute time when smoke is turned on
unsigned long lastBubbleTime;   // relative to (counting from) showStartTime
unsigned long easterStartTime;  // absolute time (not relative)
unsigned long lastEventCheckTime; // capturing the absolute time when event was checked

bool smokeMachineON = false;
bool bubbleMachineON = false;
bool whiteLightsON = false;
bool colorLights_1_ON = false;
bool colorLights_2_ON = false;
bool songPlaying = false;
bool flickersON = false;

bool easterShortFlag = false;
bool easterLongFlag = false;
bool easterContFlag = false;
bool timeToCheckEvent = true;

event_t btnEvent;

// this holds a pointer to the current LED plan
// choose a plan from LED plan functions in "ledDriver.h"
func_ptr_t ledPlan;
// --------------------------

// ------------------------ Main ------------------------
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // open the serial port at 9600 bps
  DEBUG_PRINTLN("DEBUG MODE");

  // input pin
  pinMode(PUSH_BTN_PIN, INPUT_PULLUP);

  // relay output pins
  pinMode(WHITE_LIGHTS, OUTPUT);
  pinMode(COLOR_LIGHTS_1, OUTPUT);
  pinMode(COLOR_LIGHTS_2, OUTPUT);
  pinMode(BUBBLE_MACHINE, OUTPUT);
  pinMode(SMOKE_MACHINE, OUTPUT);
  pinMode(FLICKERS, OUTPUT);

//  // led output pins (interface to the other Arduino that controls the LEDs)
//  pinMode(ARDUINO_PIN_1, OUTPUT);
//  pinMode(LED_PIN_2, OUTPUT);
  

  // set last event as now
  lastEventCheckTime = millis();

  // setup LEDs
  //led_setup();        // init led stuff
  ledPlan = runway;  // choose first LED plan
}

event_t event = NO_EVENT;
void loop() {
  now = millis();
  
  // event handling
  timeToCheckEvent = ((lastEventCheckTime + BUTTON_DELAY_MS) < now);
  if (true == timeToCheckEvent) {
    lastEventCheckTime = now;
    event = getEvent();
    if (NO_EVENT != event) {
      DEBUG_PRINT("event: ");
      DEBUG_PRINTLN(event);
      
      cur_state = get_new_state(prev_state, event); // determine the next state based on prev_state & the input
      
      DEBUG_PRINT("prev_state: ");
      DEBUG_PRINTLN(prev_state);
      DEBUG_PRINT("cur_state: ");
      DEBUG_PRINTLN(cur_state);
      
      transition_output(prev_state, cur_state, event);  // mealy machine output on transition
      prev_state = cur_state;
    }
  }
  
  // LED handling
  //led_run(ledPlan);
  
  // state handling
  switch (cur_state)
  {
    case IDLE_STATE:
      idle_state();
      break;
    case SHOW_STATE:
      show_state();
      break;
    case EASTER_STATE:
      easter_state();
      break;
  }
}
// ------------------------------------------------------

// ----------- state functions -----------
// -- IDLE STATE --
void idle_setup() {
  ledPlan = runway;
  DEBUG_PRINTLN("idle_setup()");
  DEBUG_PRINTLN("IDLE SETUP - STOP SONG");
  turnAllRelaysOff();
  return;
}

void idle_state()
{
  // random LED plans that are directed towards into the stage
  return;
}
// ----------------

// -- SHOW STATE --
bool showSetupFlag = false;
void show_setup() {
  ledPlan = confetti;
  DEBUG_PRINTLN("show_setup()");
  showSetupFlag = true;
  showTime = 0;
  showStartTime = millis();
  return;
}

void show_state()
{
  showTime = now-showStartTime;
   
  if (true == showSetupFlag) {
    // Color 1
    if ((false == colorLights_1_ON) && (showTime > SHOW_COLOR_LIGHTS_1_TIME)) {
        DEBUG_PRINTLN("SHOW - color lights 1 ON");
        relayToggle(COLOR_LIGHTS_1,ON);}
    // Color 2
    if ((false == colorLights_2_ON) && (showTime > SHOW_COLOR_LIGHTS_2_TIME)) {
        DEBUG_PRINTLN("SHOW - color lights 2 ON");
        relayToggle(COLOR_LIGHTS_2,ON);}    
    // White lights
    if ((false == whiteLightsON) && (showTime > SHOW_WHITE_LIGHTS_TIME)) {
        DEBUG_PRINTLN("SHOW - white lights ON");
        relayToggle(WHITE_LIGHTS,ON);}
    // Play song
    if ((false == songPlaying) && (showTime > SHOW_SONG_PLAY_TIME)) {
        DEBUG_PRINTLN("SHOW - play song");
        Serial.print(SERIAL_PLAY_SONG);
        songPlaying = true;}
    // Bubbles
    if ((false == bubbleMachineON) && (showTime > SHOW_BUBBLE_TIME)) {
        DEBUG_PRINTLN("SHOW - bubbules ON");
        relayToggle(BUBBLE_MACHINE,ON);}
    
    // Smoke
    if ((false == smokeMachineON) && (showTime > SHOW_SMOKE_TIME)) {
        DEBUG_PRINTLN("SHOW - smoke ON");
        relayToggle(SMOKE_MACHINE,ON);
        lastSmokeTime = showTime;}

    // exit setup when everything is ON
    if (whiteLightsON && colorLights_1_ON && colorLights_2_ON && songPlaying && smokeMachineON && bubbleMachineON) {
        DEBUG_PRINTLN("end show setup");
        showSetupFlag = false; }
  }

  // smoke machine control
  // smoke ON
  if ((showTime - lastSmokeTime > SHOW_SMOKE_REPEAT_TIME) && (false == smokeMachineON)) {
      DEBUG_PRINTLN("SHOW - smoke ON");
      relayToggle(SMOKE_MACHINE,ON);
      lastSmokeTime = showTime; }

  // smoke machine control
  if (true == smokeMachineON && (showTime-lastSmokeTime > SHOW_SMOKE_DURATION)) {
      DEBUG_PRINTLN("SHOW - smoke OFF");
      relayToggle(SMOKE_MACHINE,OFF);
  }
  
  return;
}
// ----------------

// -- EASTER STATE --
void easter_setup() {
  DEBUG_PRINTLN("easter_setup()");
  easterStartTime = now;
  relayToggle(WHITE_LIGHTS,OFF);
  relayToggle(COLOR_LIGHTS_1,OFF);
  relayToggle(COLOR_LIGHTS_2,OFF);
  return;
}

void easter_state()
{
  // add continues tap mode:
  // if someone taps non-stop then do something every X taps

  // SHORT TAP
  if ((true == easterShortFlag)) {
     DEBUG_PRINTLN("easter SHORT_TAP");
     easterShortFlag = false;
     ledPlan = rainbow;
     Serial.print(SERIAL_VOCAL_WOW_FX);
     relayToggle(SMOKE_MACHINE,ON);  // turn smoke on for a few seconds
     }
  
  // SHORT TAP end - turn smoke off after a few seconds
  if ((now - smokeStartTime > EASTER_SMOKE_DURATION) && (true == smokeMachineON)) {
      relayToggle(SMOKE_MACHINE,OFF);}
         
  // LONG TAP
  if ((true == easterLongFlag) && (false == flickersON)) {
     DEBUG_PRINTLN("easter LONG_TAP");
     easterLongFlag = false;
     ledPlan = juggle;
     Serial.print(SERIAL_ZOTI_WOW_FX);
     relayToggle(FLICKERS,ON); }

  // CONT TAP
  if (true == easterContFlag) {
     DEBUG_PRINTLN("easter CONT_TAP");
     easterContFlag = false;
     ledPlan = bpm;
     Serial.print(SERIAL_ZOTI_WOW_FX);
     }

  return;
}

void easter_exit() {
  DEBUG_PRINTLN("easter_exit()");
  relayToggle(SMOKE_MACHINE,OFF);
  relayToggle(FLICKERS,OFF);
  return;
}
// ------------------

// ----------- state machine -----------
state_t get_new_state(state_t prev_state, event_t event)
{
  if (SONG_END == event) {
    songPlaying = false;
    return IDLE_STATE;
  }
  
  if (LONG_PRESS == event) {
    Serial.print(SERIAL_STOP_SONG);
    songPlaying = false;
    return IDLE_STATE;
  }
  
  if ((IDLE_STATE != prev_state) && (SHOW_TIMEOUT_EVENT == event)) {
    songPlaying = false;
          return IDLE_STATE; }
          
  switch (prev_state) {
    case IDLE_STATE:
        if (SINGLE_CLICK == event) {
          return SHOW_STATE; }
        break;
    case SHOW_STATE:
        if ((DOUBLE_CLICK == event) || (TRIPPLE_CLICK == event)) {
          return SHOW_STATE; }
        if ((SHORT_TAP == event) && (showTime > EASTER_ENABLE_TIME)) {
          return EASTER_STATE; }
        break;
    case EASTER_STATE:
        if (EASTER_TIMEOUT_EVENT == event) {
          return SHOW_STATE; }
        break;
    default:
        return prev_state;
  }
}
// -------------------------------------

// ----------- transition functions -----------
// transitions between states
void transition_output(state_t prev_state, state_t cur_state, event_t event)
{
  if ((SONG_END == event) || (LONG_PRESS == event)) {
    idle_setup();
    return; }
   
  switch (prev_state) {
    case IDLE_STATE:
      if (SINGLE_CLICK == event) {
          // do we want to flash LEDs three times every time a song starts? (put it in show_setup())
          show_setup(); }
      break;
      
    case SHOW_STATE:
      if ((DOUBLE_CLICK == event) || (TRIPPLE_CLICK == event)) {
          Serial.print(SERIAL_PLAY_SONG);
          songPlaying = true; 
          }
      if (SHORT_TAP == event) {
          easterShortFlag = true;
          easter_setup(); }
      break;
      
    case EASTER_STATE:
      if (LONG_TAP == event) {
          easterLongFlag = true;
          }
      if (CONT_TAP == event) {
          easterContFlag = true;
          }
      if (cur_state != EASTER_STATE) {
          easter_exit();
          } 
      break;        
  }
  return;
}
// --------------------------------------------

// ----------- event translation -----------
event_t getEvent()
{
  char incomingByte;
  
  // Button event:
  btnEvent = btnPushSense();
  if (NO_EVENT != btnEvent) {
    DEBUG_PRINT("BUTTON EVENT: ");
    DEBUG_PRINTLN(btnEvent);
    return btnEvent;
  }

  // Timeout event (EASTER): check if we're too long in EASTER_STATE
  if ((EASTER_STATE == cur_state) && (now-easterStartTime > EASTER_TIMEOUT_MS)) {
    return EASTER_TIMEOUT_EVENT;
  }

  // Timeout event (SHOW): check if we're too long in SHOW_STATE
  if ((SHOW_STATE == cur_state) && (now-showStartTime > SHOW_TIMEOUT_MS)) {
    return SHOW_TIMEOUT_EVENT;
  }

  // Seial COM event: recieve byte from PC to know when song is over
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    switch (incomingByte) {
      case 'F':
        DEBUG_PRINT("PC just sent me this: ");
        DEBUG_PRINTLN(incomingByte);
        return SONG_END;
      default:
        DEBUG_PRINT("Hey I recieved this weird byte on serial: ");
        DEBUG_PRINTLN(incomingByte);
        break;
    }
  }

  // None of the above events occured:
  return NO_EVENT;
}

// ----------- Relay functions -----------
void turnAllRelaysOff() {
  relayToggle(SMOKE_MACHINE,OFF);
  relayToggle(BUBBLE_MACHINE,OFF);
  relayToggle(FLICKERS,OFF);
  relayToggle(WHITE_LIGHTS,OFF);
  relayToggle(COLOR_LIGHTS_1,OFF);
  relayToggle(COLOR_LIGHTS_2,OFF);
}



void relayToggle(int pin_number,int desiredOutput) {
  digitalWrite(pin_number, desiredOutput);
  switch (pin_number) {
    case SMOKE_MACHINE:
      smokeStartTime = now;
      smokeMachineON = (HIGH == desiredOutput);
      DEBUG_PRINT("RELAY - SMOKE_MACHINE: ");
      DEBUG_PRINTLN(smokeMachineON);
      break;
    case BUBBLE_MACHINE:
      bubbleMachineON = (HIGH == desiredOutput);
      DEBUG_PRINT("RELAY - BUBBLE_MACHINE: ");
      DEBUG_PRINTLN(bubbleMachineON);
      break;
    case WHITE_LIGHTS:
      whiteLightsON = (HIGH == desiredOutput);
      DEBUG_PRINT("RELAY - WHITE_LIGHTS: ");
      DEBUG_PRINTLN(whiteLightsON);
      break;
    case COLOR_LIGHTS_1:
      colorLights_1_ON = (HIGH == desiredOutput);
      DEBUG_PRINT("RELAY - COLOR_LIGHTS_1: ");
      DEBUG_PRINTLN(colorLights_1_ON);
      break;
    case COLOR_LIGHTS_2:
      colorLights_2_ON = (HIGH == desiredOutput);
      DEBUG_PRINT("RELAY - COLOR_LIGHTS_2: ");
      DEBUG_PRINTLN(colorLights_2_ON);
      break;
    case FLICKERS:
      flickersON = (HIGH == desiredOutput);
      DEBUG_PRINT("RELAY - FLICKERS: ");
      DEBUG_PRINTLN(flickersON);
      break;
    default:
      DEBUG_PRINTLN("undefined pin");
      break;
  }
  return;
  
}
