#define DEBUG
#include "pushButtonDriver.h"

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// ---------------------- Stage control ----------------------
// -- constants --
#define BUTTON_DELAY_MS  50      // push button debouncibg delay in millisec

//// pins definition MEGA
//#define WHITE_LIGHTS        31
//#define COLOR_LIGHTS_1      33
//#define COLOR_LIGHTS_2      35
//#define BUBBLE_MACHINE      37
//#define SMOKE_MACHINE       39
//#define FLICKERS            41
//#define WHITE_LIGHTS_BACKUP        43
//#define COLOR_LIGHTS_1_BACKUP      45
//#define COLOR_LIGHTS_2_BACKUP      47
//#define FLICKERS_BACKUP            49

// pins definition UNO
#define FIRST_PIN     2
#define LAST_PIN      11

#define WHITE_LIGHTS        8
#define COLOR_LIGHTS_1      9
#define COLOR_LIGHTS_2      7
#define BUBBLE_MACHINE      10
#define SMOKE_MACHINE       5 // new
#define FLICKERS            6
//#define LED_CONTROL_PIN_1   2
//#define LED_CONTROL_PIN_2   3

#define WHITE_LIGHTS_BACKUP        2
#define COLOR_LIGHTS_1_BACKUP      3
#define COLOR_LIGHTS_2_BACKUP      4
#define FLICKERS_BACKUP            11

#define ON  HIGH
#define OFF LOW
// ---------------

// -- functions declaration --
// ---------------------------


// AUX functions
void turnAllRelaysOff();
void relayToggle(int pin_number,int desiredOutput);
// ---------------------------


// ------- globals ----------
unsigned long this_now;
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

// --------------------------

// ------------------------ Main ------------------------
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // open the serial port at 9600 bps
  DEBUG_PRINTLN("Pin Discovery !!!!");
  DEBUG_PRINTLN("Every time you push the button the next pin is turned on");
  DEBUG_PRINT("FIRST_PIN = ");
  DEBUG_PRINTLN(FIRST_PIN);
  DEBUG_PRINT("LAST_PIN = ");
  DEBUG_PRINTLN(LAST_PIN);
  
  // input pin
  pinMode(PUSH_BTN_PIN, INPUT_PULLUP);

  // relay output pins
  pinMode(WHITE_LIGHTS, OUTPUT);
  pinMode(COLOR_LIGHTS_1, OUTPUT);
  pinMode(COLOR_LIGHTS_2, OUTPUT);
  pinMode(BUBBLE_MACHINE, OUTPUT);
  pinMode(SMOKE_MACHINE, OUTPUT);
  pinMode(FLICKERS, OUTPUT);
  
  pinMode(WHITE_LIGHTS_BACKUP, OUTPUT);
  pinMode(COLOR_LIGHTS_1_BACKUP, OUTPUT);
  pinMode(COLOR_LIGHTS_2_BACKUP, OUTPUT);
  pinMode(FLICKERS_BACKUP, OUTPUT);

  turnAllRelaysOff();
  
  // initialize variables
  btnEvent = NO_EVENT;
    
  // set last event as now
  lastEventCheckTime = millis();
}

event_t event = NO_EVENT;

int pin_num = 2;
void loop() {
  this_now = millis();
  
  // event handling
  timeToCheckEvent = ((lastEventCheckTime + BUTTON_DELAY_MS) < this_now);
  if (true == timeToCheckEvent) {
    lastEventCheckTime = this_now;
    event = btnPushSense();
    if (NO_EVENT != event) {
      turnAllRelaysOff();
      DEBUG_PRINT("event: ");
      DEBUG_PRINTLN(event);

      DEBUG_PRINT("Turning ON PIN: ");
      DEBUG_PRINTLN(pin_num);
      relayToggle(pin_num,ON);
      pin_num++;
      if (pin_num > LAST_PIN) {
        pin_num = FIRST_PIN;
      }
    }
  }
}
// -----------------------------------------------------

// ----------- Relay functions -----------
void turnAllRelaysOff() {
  relayToggle(SMOKE_MACHINE,OFF);
  relayToggle(BUBBLE_MACHINE,OFF);
  relayToggle(FLICKERS,OFF); relayToggle(FLICKERS_BACKUP,OFF);
  relayToggle(WHITE_LIGHTS,OFF); relayToggle(WHITE_LIGHTS_BACKUP,OFF);
  relayToggle(COLOR_LIGHTS_1,OFF); relayToggle(COLOR_LIGHTS_1_BACKUP,OFF);
  relayToggle(COLOR_LIGHTS_2,OFF); relayToggle(COLOR_LIGHTS_2_BACKUP,OFF);
}



void relayToggle(int pin_number,int desiredOutput) {
  digitalWrite(pin_number, desiredOutput);
  
  switch (pin_number) {
    case SMOKE_MACHINE:
      smokeStartTime = this_now;
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
