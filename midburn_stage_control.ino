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

#define SHOW_WHITE_LIGHTS_TIME 2000
#define SHOW_COLOR_LIGHTS_TIME 4000
#define SHOW_SONG_PLAY_TIME    5000
#define SHOW_SMOKE_TIME        10000
#define SHOW_SMOKE_DURATION    4000
#define SHOW_BUBBLE_REPEAT_TIME 40000
#define SHOW_BUBBLE_DURATION    10000
#define SHOW_TIMEOUT_MS         (60000*6) // 6 minutes

#define EASTER_ENABLE_TIME        (SHOW_SONG_PLAY_TIME)
#define EASTER_SMOKE_DURATION     4000    
#define EASTER_FLICKERS_DURATION  15000 
#define EASTER_TIMEOUT_MS         (EASTER_SMOKE_DURATION + EASTER_FLICKERS_DURATION)

// pins definition
#define WHITE_LIGHTS    2
#define COLOR_LIGHTS    3
#define BUBBLE_MACHINE  4
#define SMOKE_MACHINE   5
#define FLICKERS        6





#define SERIAL_PLAY_SONG  'P' // Play
#define SERIAL_STOP_SONG  'S' // Stop
#define SERIAL_CLAP_FX    'C' // Clapping effect
#define SERIAL_DRUMS_FX   'D' // Drums effect
#define SERIAL_WOW_FX     'E' // Vocal wow effect for easter

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
unsigned long showStartTime;    // capturing the show start
unsigned long lastSmokeTime;
unsigned long lastBubbleTime;
unsigned long easterStartTime;
unsigned long lastEventCheckTime;

bool smokeMachineON = false;
bool bubbleMachineON = false;
bool whiteLightsON = false;
bool colorLightsON = false;
bool songPlaying = false;
bool flickersON = false;

bool easterLongFlag = false;
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
  DEBUG_PRINTLN("Started...");

  // input pin
  pinMode(PUSH_BTN_PIN, INPUT_PULLUP);

  // relay output pins
  pinMode(WHITE_LIGHTS, OUTPUT);
  pinMode(COLOR_LIGHTS, OUTPUT);
  pinMode(BUBBLE_MACHINE, OUTPUT);
  pinMode(SMOKE_MACHINE, OUTPUT);
  pinMode(FLICKERS, OUTPUT);

  // set last event as now
  lastEventCheckTime = millis();

  // setup LEDs
  led_setup();        // init led stuff
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
  led_run(ledPlan);
  
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
  //flashLEDs();
  //Serial.print(SERIAL_STOP_SONG);
  DEBUG_PRINTLN("SHOW SETUP - STOP SONG");
  showSetupFlag = true;
  showTime = 0;
  showStartTime = millis();
  return;
}

void show_state()
{
  //DEBUG_PRINTLN("show_state()");
  showTime = now-showStartTime;
  // LEDplan = random...
  // run LED plan
  // 
  // LEDs control
   
  
  if (true == showSetupFlag) {
    if ((false == whiteLightsON) && (showTime > SHOW_WHITE_LIGHTS_TIME)) {
        DEBUG_PRINTLN("SHOW - white lights ON");
        relayToggle(WHITE_LIGHTS,ON);}
    if ((false == colorLightsON) && (showTime > SHOW_COLOR_LIGHTS_TIME)) {
        DEBUG_PRINTLN("SHOW - color lights ON");
        relayToggle(COLOR_LIGHTS,ON);}
    if ((false == songPlaying) && (showTime > SHOW_SONG_PLAY_TIME)) {
        DEBUG_PRINTLN("SHOW - play song");
        Serial.print(SERIAL_PLAY_SONG);
        songPlaying = true;}
    if ((false == smokeMachineON) && (showTime > SHOW_SMOKE_TIME)) {
        DEBUG_PRINTLN("SHOW - smoke ON");
        relayToggle(SMOKE_MACHINE,ON);
        lastSmokeTime = showTime;}
    
    // exit setup when everything is ON
    if (whiteLightsON && colorLightsON && songPlaying && smokeMachineON) {
        DEBUG_PRINTLN("end show setup");
        showSetupFlag = false; }
  }

  // bubble machine control
  // bubbles ON
  if ((showTime - lastBubbleTime > SHOW_BUBBLE_REPEAT_TIME) && (false == bubbleMachineON)) {
      DEBUG_PRINTLN("SHOW - bubbules ON");
      relayToggle(BUBBLE_MACHINE,ON);
      lastBubbleTime = showTime; }

  // bubbles OFF
  if (true == bubbleMachineON && (showTime-lastBubbleTime > SHOW_BUBBLE_DURATION)) {
      DEBUG_PRINTLN("SHOW - bubbles OFF");
      relayToggle(BUBBLE_MACHINE,OFF);
  }

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
  ledPlan = rainbow;
  DEBUG_PRINTLN("easter_setup()");
  easterStartTime = now;
  relayToggle(WHITE_LIGHTS,OFF);
  relayToggle(COLOR_LIGHTS,OFF);
  // SHORT TAP
  relayToggle(SMOKE_MACHINE,ON);  // turn smoke on for 4 sec
  return;
}

void easter_state()
{
  // turn smoke off after 4 sec
  if ((now - easterStartTime > EASTER_SMOKE_DURATION) && (true == smokeMachineON)) {
      relayToggle(SMOKE_MACHINE,OFF); }
      
  // LONG TAP
  if ((true == easterLongFlag) && (false == flickersON)) {
     relayToggle(FLICKERS,ON); }
     
  return;
}

void easter_exit() {
  DEBUG_PRINTLN("easter_exit()");
  relayToggle(SMOKE_MACHINE,OFF);
  relayToggle(FLICKERS,OFF);
  easterLongFlag = false;
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
          Serial.print(SERIAL_STOP_SONG);
          songPlaying = false;
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
          show_setup(); }
      if (SHORT_TAP == event) {
          easter_setup(); }
      break;
      
    case EASTER_STATE:
      if (LONG_TAP == event) {
          easterLongFlag = true; }
      if (cur_state != EASTER_STATE) {
          easter_exit(); }
      break;        
  }
  return;
}
// --------------------------------------------

// ----------- event translation -----------
event_t getEvent()
{
  char incomingByte;
  
  // check if button event
  btnEvent = btnPushSense();
  if (NO_EVENT != btnEvent) {
    return btnEvent;
  }

  // check if we're too long in EASTER_STATE
  if ((EASTER_STATE == cur_state) && (now-easterStartTime > EASTER_TIMEOUT_MS)) {
    return EASTER_TIMEOUT_EVENT;
  }

  // check if we're too long in SHOW_STATE
  if ((SHOW_STATE == cur_state) && (now-showStartTime > SHOW_TIMEOUT_MS)) {
    return SHOW_TIMEOUT_EVENT;
  }

  // recieve byte from PC to know when song is over
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
  return NO_EVENT;
}

// ----------- Relay functions -----------
void turnAllRelaysOff() {
  relayToggle(SMOKE_MACHINE,OFF);
  relayToggle(BUBBLE_MACHINE,OFF);
  relayToggle(FLICKERS,OFF);
  relayToggle(WHITE_LIGHTS,OFF);
  relayToggle(COLOR_LIGHTS,OFF);
}



void relayToggle(int pin_number,int desiredOutput) {
  digitalWrite(pin_number, desiredOutput);
  switch (pin_number) {
    case SMOKE_MACHINE:
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
    case COLOR_LIGHTS:
      colorLightsON = (HIGH == desiredOutput);
      DEBUG_PRINT("RELAY - COLOR_LIGHTS: ");
      DEBUG_PRINTLN(colorLightsON);
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
