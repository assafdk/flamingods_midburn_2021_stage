#define DEBUG
#define I2C
#include <Arduino.h>
#include <FastLED.h>
#include "pushButtonDriver.h"
#include <Wire.h>
#include "LoRa.h"

extern unsigned long now; // this is the time the button is pushed

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// if using I2C
#ifdef I2C
#define I2C_SLAVE_ADDR 9
#endif

typedef enum {
  IDLE_STATE,
  SHOW_STATE,
  EASTER_STATE   
} state_t;
state_t cur_state, prev_state;

typedef enum {
  LED_IDLE,
  LED_SHOW,
  LED_EASTER,
  LED_FUN
} ledState_t;

#define LED_PLANS_COUNT 4

// ---------------------- Stage control ----------------------
// -- constants --
#define BUTTON_DELAY_MS  50      // push button debouncibg delay in millisec

#ifdef DEBUG
  #define dbg_delay                 5000
  #define SHOW_COLOR_LIGHTS_1_TIME  dbg_delay
  #define SHOW_COLOR_LIGHTS_2_TIME  (2*dbg_delay)
  #define SHOW_WHITE_LIGHTS_TIME    (3*dbg_delay)
  #define SHOW_SONG_PLAY_TIME       (3*dbg_delay)
  #define SHOW_BUBBLE_TIME          (4*dbg_delay)
  #define SHOW_SMOKE_TIME           (4*dbg_delay)
  
  #define SHOW_SMOKE_DURATION       4000
  #define SHOW_SMOKE_REPEAT_TIME    40000
  
  //#define SHOW_BUBBLE_DURATION    10000
  #define SHOW_TIMEOUT_MS           (60000*1) // 1 minute timeout in debug
  
  #define EASTER_ENABLE_TIME        (SHOW_SONG_PLAY_TIME)
  #define EASTER_SMOKE_DURATION     4000    
  #define EASTER_FLICKERS_DURATION  15000 
  #define EASTER_TIMEOUT_MS         (EASTER_SMOKE_DURATION + EASTER_FLICKERS_DURATION)
#else
  #define SHOW_COLOR_LIGHTS_1_TIME  500
  #define SHOW_COLOR_LIGHTS_2_TIME  1000
  #define SHOW_WHITE_LIGHTS_TIME    1500
  #define SHOW_SONG_PLAY_TIME       1500
  #define SHOW_BUBBLE_TIME          2000
  #define SHOW_SMOKE_TIME           2000
  
  #define SHOW_SMOKE_DURATION       4000
  #define SHOW_SMOKE_REPEAT_TIME    40000
  
  //#define SHOW_BUBBLE_DURATION    10000
  #define SHOW_TIMEOUT_MS           (60000*6) // 6 minutes show timeout
  
  #define EASTER_ENABLE_TIME        (SHOW_SONG_PLAY_TIME)
  #define EASTER_SMOKE_DURATION     4000    
  #define EASTER_FLICKERS_DURATION  15000 
  #define EASTER_TIMEOUT_MS         (EASTER_SMOKE_DURATION + EASTER_FLICKERS_DURATION)
#endif

#define ON  HIGH
#define OFF LOW

// --- pins definition MEGA ---
#define WHITE_LIGHTS        2
#define COLOR_LIGHTS_1      3
#define COLOR_LIGHTS_2      4
#define BUBBLE_MACHINE      6
#define SMOKE_MACHINE       5
#define FLICKERS            7
#define WHITE_LIGHTS_BACKUP        8
#define COLOR_LIGHTS_1_BACKUP      9
#define COLOR_LIGHTS_2_BACKUP      10
#define FLICKERS_BACKUP            11
#define OTHER_PIN_1            12
#define OTHER_PIN_2            13
#define BluetoothSerial Serial1   // TX1 / RX1

// LoRa pins
#define LORA_SS_PIN     53
#define LORA_RESET_PIN  49
#define LORA_DIO0_PIN   48
// Default LoRa SPI pins!! Can't change them... stated for documentation only:
// #define MISO  50
// #define MOSI  51
// #define SCK   52

// // --- pins definition UNO ---
// #define WHITE_LIGHTS        8
// #define COLOR_LIGHTS_1      9
// #define COLOR_LIGHTS_2      7
// #define BUBBLE_MACHINE      10
// #define SMOKE_MACHINE       5
// #define FLICKERS            6
// //#define LED_CONTROL_PIN_1   2
// //#define LED_CONTROL_PIN_2   3
// #define WHITE_LIGHTS_BACKUP        2
// #define COLOR_LIGHTS_1_BACKUP      3
// #define COLOR_LIGHTS_2_BACKUP      4
// #define FLICKERS_BACKUP            11
// // -------- Bluetooth UNO --------
// #include <SoftwareSerial.h>
// #define BLUETOOTH_SERIAL_RX   A5
// #define BLUETOOTH_SERIAL_TX   A4
// SoftwareSerial BluetoothSerial(BLUETOOTH_SERIAL_RX, BLUETOOTH_SERIAL_TX); // RX | TX 

#define SERIAL_PLAY_SONG  'P' // Play
#define SERIAL_STOP_SONG  'S' // Stop
//#define SERIAL_CLAP_FX    'C' // Clapping effect
//#define SERIAL_DRUMS_FX   'D' // Drums effect
#define SERIAL_VOCAL_WOW_FX     'V' // Vocal wow effect for easter
#define SERIAL_ZOTI_WOW_FX     'Z' // Vocal wow effect for easter

#define PC_BAUD_RATE 9600

// -------- COMMUNICATION --------
// -------- Bluetooth --------
#define BT_MAX_MESSAGE_LEN 14
#define BT_MSG_START_CHAR '{'
#define BT_MSG_SEPERATION_CHAR ','
#define BT_MSG_END_CHAR '}'
#define APPLIANCE_REPORT_DELAY 3000   // send appliances status to tablet over bluetooth every APPLIANCE_REPORT_DELAY ms
#define BT_BAUD_RATE 9600

#define FLAMINGO_MSG_LEN  7   // msg format to flamingo is TPRGBYW: 0101100
unsigned long lastAppliancesReportTime; // capturing the last time appliances status was reported over bluetooth
static bool btMsgInProgress = false;
static bool bt_msg_ready = false;
static char bt_message[BT_MAX_MESSAGE_LEN] = {0};
static int bt_indx=0;
static char bt_inByte;

// Bluetooth functions
void recv_BT_msg();
event_t parse_BT_msg();
void report_appliances_status();
void init_bt_buffer();

// -------- LoRa --------
#define LORA_BUFF_SIZE  16      // no reason why I chose 16. has to be >7
#define LORA_MSG_IDLE   "10"  // {TC} - Type=1 Command=0 (0=Idle)
#define LORA_MSG_SHOW   "11"  // {TC} - Type=1 Command=0 (1=Show)
#define LORA_MSG_EASTER "12"  // {TC} - Type=1 Command=0 (2=Easter)
#define LORA_MSG_LEN    2

#define LORA_SEND_IDLE_INTERVAL 60  // send IDLE to flamingo every 1 minute
#define LORA_SEND_SHOW_INTERVAL 10  // send SHOW to flamingo every 10 seconds
#define LORA_SEND_EASTER_INTERVAL 1  // send SHOW to flamingo every 1 seconds

char LoRa_buff[LORA_BUFF_SIZE] = {0};
char LoRa_incoming_buff[LORA_BUFF_SIZE] = {0};
void LoRa_send(char* buff,int len);
void LoRa_read(char* buff,int len);
bool LoRa_RecvFlag = false;
void init_lora_incoming_buff();

// --------------------------------

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
void ledControl(ledState_t ledState);
void ledControl_I2C(ledState_t ledState);
void setupLedCom();

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

// --------------------------

// ------------------------ Main ------------------------
void setup() {
  // put your setup code here, to run once:
  Serial.begin(PC_BAUD_RATE); // open the serial port at 9600 bps
  delay(500);
  BluetoothSerial.begin(BT_BAUD_RATE);
  delay(500);
  init_bt_buffer();  
  LoRa.setPins(LORA_SS_PIN,LORA_RESET_PIN,LORA_DIO0_PIN);
  LoRa.begin(433E6);
  delay(500);
  DEBUG_PRINTLN("DEBUG MODE");
  
  setupLedCom(); // I2C or direct
  
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
  pinMode(OTHER_PIN_1, OUTPUT);
  pinMode(OTHER_PIN_2, OUTPUT);

  turnAllRelaysOff();
  
  // initialize variables
  prev_state = IDLE_STATE;
  cur_state = IDLE_STATE;
  btnEvent = NO_EVENT;
    
  // set last event as now
  lastEventCheckTime = millis();

  // setup LEDs
  ledControl(LED_IDLE);

  // print constants
  DEBUG_PRINTLN("Printing constants:");
  DEBUG_PRINT("dbg_delay = ");
  DEBUG_PRINTLN(dbg_delay);
  DEBUG_PRINT("SHOW_COLOR_LIGHTS_1_TIME = ");
  DEBUG_PRINTLN(SHOW_COLOR_LIGHTS_1_TIME);
  DEBUG_PRINT("SHOW_COLOR_LIGHTS_2_TIME = ");
  DEBUG_PRINTLN(SHOW_COLOR_LIGHTS_2_TIME);
  DEBUG_PRINT("SHOW_WHITE_LIGHTS_TIME = ");    
  DEBUG_PRINTLN(SHOW_WHITE_LIGHTS_TIME);    
  DEBUG_PRINT("SHOW_SONG_PLAY_TIME = ");       
  DEBUG_PRINTLN(SHOW_SONG_PLAY_TIME);       
  DEBUG_PRINT("SHOW_BUBBLE_TIME = ");          
  DEBUG_PRINTLN(SHOW_BUBBLE_TIME);          
  DEBUG_PRINT("SHOW_SMOKE_TIME = ");          
  DEBUG_PRINTLN(SHOW_SMOKE_TIME);          
  
  DEBUG_PRINT("SHOW_SMOKE_DURATION = ");       
  DEBUG_PRINTLN(SHOW_SMOKE_DURATION);       
  DEBUG_PRINT("SHOW_SMOKE_REPEAT_TIME = ");    
  DEBUG_PRINTLN(SHOW_SMOKE_REPEAT_TIME);    
  
  //DEBUG_PRINT("SHOW_BUBBLE_DURATION    
  //DEBUG_PRINTLN(SHOW_BUBBLE_DURATION    
  DEBUG_PRINT("SHOW_TIMEOUT_MS = ");           
  DEBUG_PRINTLN(SHOW_TIMEOUT_MS);           
  
  DEBUG_PRINT("EASTER_ENABLE_TIME = ");       
  DEBUG_PRINTLN(EASTER_ENABLE_TIME);       
  DEBUG_PRINT("EASTER_SMOKE_DURATION = ");         
  DEBUG_PRINTLN(EASTER_SMOKE_DURATION);         
  DEBUG_PRINT("EASTER_FLICKERS_DURATION = ");
  DEBUG_PRINTLN(EASTER_FLICKERS_DURATION);  
  DEBUG_PRINT("EASTER_TIMEOUT_MS = ");        
  DEBUG_PRINTLN(EASTER_TIMEOUT_MS);
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
  DEBUG_PRINTLN("idle_setup()");
  ledControl(LED_IDLE);
  LoRa_send(LORA_MSG_IDLE, strlen(LORA_MSG_IDLE));
  DEBUG_PRINTLN("IDLE SETUP - STOP SONG");
  turnAllRelaysOff();
  return;
}

void idle_state()
{
  EVERY_N_SECONDS( LORA_SEND_IDLE_INTERVAL ) { LoRa_send(LORA_MSG_IDLE, strlen(LORA_MSG_IDLE)); }
  
  return;
}
// ----------------

// -- SHOW STATE --
bool showSetupFlag = false;
void show_setup() {
  ledControl(LED_SHOW);
  LoRa_send(LORA_MSG_SHOW, strlen(LORA_MSG_SHOW));
  // LoRa_send(LORA_MSG_SHOW, 4);
  DEBUG_PRINTLN("show_setup()");
  showSetupFlag = true;
  showTime = 0;
  showStartTime = now;
  return;
}

void show_state()
{
  showTime = now-showStartTime;
   
  if (true == showSetupFlag) {
    // Color 1
    if ((false == colorLights_1_ON) && (showTime > SHOW_COLOR_LIGHTS_1_TIME)) {
        DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
        DEBUG_PRINTLN(showTime);
        DEBUG_PRINTLN("SHOW - color lights 1 ON");
        relayToggle(COLOR_LIGHTS_1,ON); relayToggle(COLOR_LIGHTS_1_BACKUP,ON);}
    // Color 2
    if ((false == colorLights_2_ON) && (showTime > SHOW_COLOR_LIGHTS_2_TIME)) {
        DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
        DEBUG_PRINTLN(showTime);
        DEBUG_PRINTLN("SHOW - color lights 2 ON");
        relayToggle(COLOR_LIGHTS_2,ON); relayToggle(COLOR_LIGHTS_2_BACKUP,ON);
        relayToggle(FLICKERS,ON); relayToggle(FLICKERS_BACKUP,ON);}    
    // White lights
    if ((false == whiteLightsON) && (showTime > SHOW_WHITE_LIGHTS_TIME)) {
        DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
        DEBUG_PRINTLN(showTime);
        DEBUG_PRINTLN("SHOW - white lights ON");
        relayToggle(WHITE_LIGHTS,ON); relayToggle(WHITE_LIGHTS_BACKUP,ON);
        relayToggle(OTHER_PIN_1,ON); relayToggle(OTHER_PIN_2,ON);
        }
    // Play song
    if ((false == songPlaying) && (showTime > SHOW_SONG_PLAY_TIME)) {
        DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
        DEBUG_PRINTLN(showTime);
        DEBUG_PRINTLN("SHOW - play song");
        Serial.print(SERIAL_PLAY_SONG);
        songPlaying = true;}
    // Bubbles
    if ((false == bubbleMachineON) && (showTime > SHOW_BUBBLE_TIME)) {
        DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
        DEBUG_PRINTLN(showTime);
        DEBUG_PRINTLN("SHOW - bubbules ON");
        relayToggle(BUBBLE_MACHINE,ON);}
    
    // Smoke
    if ((false == smokeMachineON) && (showTime > SHOW_SMOKE_TIME)) {
        DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
        DEBUG_PRINTLN(showTime);
        DEBUG_PRINTLN("SHOW - smoke ON");
        relayToggle(SMOKE_MACHINE,ON);
        lastSmokeTime = showTime;}

    // exit setup when everything is ON
    if (whiteLightsON && colorLights_1_ON && colorLights_2_ON && songPlaying && smokeMachineON && bubbleMachineON) {
        DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
        DEBUG_PRINTLN(showTime);
        DEBUG_PRINTLN("end show setup");
        showSetupFlag = false; }
  }

  // smoke machine control
  // smoke ON
  if ((showTime - lastSmokeTime > SHOW_SMOKE_REPEAT_TIME) && (false == smokeMachineON) && (false == showSetupFlag)) {
      DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
      DEBUG_PRINTLN(showTime);
      DEBUG_PRINTLN("SHOW - smoke ON");
      relayToggle(SMOKE_MACHINE,ON);
      lastSmokeTime = showTime; }

  // smoke OFF
  if (true == smokeMachineON && (showTime-lastSmokeTime > SHOW_SMOKE_DURATION)) {
      DEBUG_PRINTLN("------------------------"); DEBUG_PRINT("showTime = ");
      DEBUG_PRINTLN(showTime);
      DEBUG_PRINTLN("SHOW - smoke OFF");
      relayToggle(SMOKE_MACHINE,OFF);
  }
  
  EVERY_N_SECONDS( LORA_SEND_SHOW_INTERVAL ) { LoRa_send(LORA_MSG_SHOW, strlen(LORA_MSG_SHOW)); }
  return;
}
// ----------------

// -- EASTER STATE --
void easter_setup() {
  DEBUG_PRINTLN("easter_setup()");
  LoRa_send(LORA_MSG_EASTER, strlen(LORA_MSG_EASTER));
  easterStartTime = now;
  relayToggle(WHITE_LIGHTS,OFF); relayToggle(WHITE_LIGHTS_BACKUP,OFF);
  relayToggle(OTHER_PIN_1,OFF); relayToggle(OTHER_PIN_2,OFF);
  relayToggle(COLOR_LIGHTS_1,OFF); relayToggle(COLOR_LIGHTS_1_BACKUP,OFF);
  relayToggle(COLOR_LIGHTS_2,OFF); relayToggle(COLOR_LIGHTS_2_BACKUP,OFF);
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
     ledControl(LED_EASTER);
     Serial.print(SERIAL_VOCAL_WOW_FX);
     relayToggle(SMOKE_MACHINE,ON);  // turn smoke on for a few seconds
     }
  
  // SHORT TAP end: turn smoke off after a few seconds
  if ((now - smokeStartTime > EASTER_SMOKE_DURATION) && (true == smokeMachineON)) {
      relayToggle(SMOKE_MACHINE,OFF);}
         
  // LONG TAP
  if (true == easterLongFlag) {
     DEBUG_PRINTLN("easter LONG_TAP");
     ledControl(LED_FUN);
     easterLongFlag = false;
     Serial.print(SERIAL_ZOTI_WOW_FX);
     }

  // CONT TAP
  if (true == easterContFlag) {
     DEBUG_PRINTLN("easter CONT_TAP");
     easterContFlag = false;
     Serial.print(SERIAL_ZOTI_WOW_FX);
     }

  EVERY_N_SECONDS( LORA_SEND_EASTER_INTERVAL ) { LoRa_send(LORA_MSG_EASTER, strlen(LORA_MSG_EASTER)); }
  return;
}

void easter_exit() {
  DEBUG_PRINTLN("easter_exit()");
  relayToggle(SMOKE_MACHINE,OFF);
  return;
}

void show_lights_ON() {
  DEBUG_PRINTLN("show_lights_ON()");
  ledControl(LED_SHOW);
  relayToggle(WHITE_LIGHTS,ON); relayToggle(WHITE_LIGHTS_BACKUP,ON);
  relayToggle(OTHER_PIN_1,ON); relayToggle(OTHER_PIN_2,ON);
  relayToggle(COLOR_LIGHTS_1,ON); relayToggle(COLOR_LIGHTS_1_BACKUP,ON);
  relayToggle(COLOR_LIGHTS_2,ON); relayToggle(COLOR_LIGHTS_2_BACKUP,ON);
  
  return;
}
// ------------------

// ----------- state machine -----------
state_t get_new_state(state_t prev_state, event_t event)
{
  if (SONG_END == event) {
    return IDLE_STATE;
  }
  
  if (LONG_PRESS == event) {
    return IDLE_STATE;
  }
  
  if ((IDLE_STATE != prev_state) && (SHOW_TIMEOUT_EVENT == event)) {
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
        if ((DOUBLE_CLICK == event) || (TRIPPLE_CLICK == event)) {
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
  if (SONG_END == event) {
    songPlaying = false;
    idle_setup();
    return; }

  if (LONG_PRESS == event) {
    songPlaying = false;
    Serial.print(SERIAL_STOP_SONG);
    idle_setup();
    return; }

  if ((IDLE_STATE != prev_state) && (SHOW_TIMEOUT_EVENT == event)) {
    songPlaying = false;
    idle_setup();
    return; }
   
  switch (prev_state) {
    case IDLE_STATE:
      if (SHOW_STATE == cur_state) {    // SINGLE_CLICK == event) {
          show_setup(); }
      break;
      
    case SHOW_STATE:
      if ((DOUBLE_CLICK == event) || (TRIPPLE_CLICK == event)) {
          Serial.print(SERIAL_PLAY_SONG);
          songPlaying = true; 
          }
      if (EASTER_STATE == cur_state) {    // (SHORT_TAP == event) {
          easterShortFlag = true;
          easter_setup(); }
      break;
      
    case EASTER_STATE:
      if ((DOUBLE_CLICK == event) || (TRIPPLE_CLICK == event)) {
          Serial.print(SERIAL_PLAY_SONG);
          songPlaying = true; 
          }
      if (LONG_TAP == event) {
          easterLongFlag = true;
          }
      if (CONT_TAP == event) {
          easterContFlag = true;
          }
      if (cur_state == SHOW_STATE) {
          show_lights_ON();
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
event_t getEvent() {
  char incomingByte;
  event_t lora_buzzer_event;  
  
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
  if ((IDLE_STATE != cur_state) && (now-showStartTime > SHOW_TIMEOUT_MS)) {
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
  
  // Bluetooth event: recieve byte from Tablet app through HC-05 Bluetooth module 
  // send relay positions to tablet app
  if (lastAppliancesReportTime + APPLIANCE_REPORT_DELAY < now) {
    lastAppliancesReportTime = now;
    report_appliances_status();
  }
  // Check for new BT message
  recv_BT_msg();
  if (bt_msg_ready) {
    bt_msg_ready = false;
    DEBUG_PRINTLN("parsing BT message");
    btnEvent = parse_BT_msg();
  } 
  if (NO_EVENT != btnEvent) {
    DEBUG_PRINT("Bluetooth BUTTON EVENT: ");
    DEBUG_PRINTLN(btnEvent);
    return btnEvent;
  }
  
  // Lora receive buzzer event
  if (LoRa.available()) {
    LoRa_read(LoRa_incoming_buff);
    lora_buzzer_event = parse_LoRa_msg(LoRa_incoming_buff);
    init_lora_incoming_buff();
    return lora_buzzer_event;      
      }   
  // None of the above events occured:
  return NO_EVENT;
}

// ----------- Relay functions -----------
void turnAllRelaysOff() {
  relayToggle(SMOKE_MACHINE,OFF);
  relayToggle(BUBBLE_MACHINE,OFF);
  relayToggle(FLICKERS,OFF); relayToggle(FLICKERS_BACKUP,OFF);
  relayToggle(WHITE_LIGHTS,OFF); relayToggle(WHITE_LIGHTS_BACKUP,OFF);
  relayToggle(COLOR_LIGHTS_1,OFF); relayToggle(COLOR_LIGHTS_1_BACKUP,OFF);
  relayToggle(COLOR_LIGHTS_2,OFF); relayToggle(COLOR_LIGHTS_2_BACKUP,OFF);
  relayToggle(OTHER_PIN_1,OFF); relayToggle(OTHER_PIN_2,OFF);
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
      DEBUG_PRINTLN("undefined pin - probably a BACKUP relay");
      break;
  }
  return;
  
}

// ----------- LED functions -----------

void ledControl_I2C(ledState_t ledState) {
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(ledState);
    Wire.endTransmission();
    return;
  }

void ledControl(ledState_t ledState) {
    #ifdef I2C
      ledControl_I2C(ledState);
      DEBUG_PRINT("I2C --> ");
      DEBUG_PRINTLN(ledState);
    #else  
      digitalWrite(LED_CONTROL_PIN_2, ledState / 2);
      digitalWrite(LED_CONTROL_PIN_1, ledState % 2);
    #endif
    return;
  }

void setupLedCom() { // I2C or direct
  #ifdef I2C
    //start I2C
    Wire.begin();
  #else
    // led output pins (interface to the other Arduino that controls the LEDs)
    pinMode(LED_CONTROL_PIN_1, OUTPUT);
    pinMode(LED_CONTROL_PIN_2, OUTPUT);
  #endif
  return;
}

// ----------- Bluetooth functions -----------

void DEBUG_recv_BT_msg(){
  char c;
  //from bluetooth to Terminal. 
  if (BluetoothSerial.available()) { 
    c=BluetoothSerial.read();
    Serial.write(c);}
  //from termial to bluetooth 
  if (Serial.available()){ 
    c=Serial.read();
    BluetoothSerial.write(c);
  }
}

void recv_BT_msg() {
  // Check if there's a new Bluetooth message
  while (BluetoothSerial.available()>0 && bt_msg_ready == false) {
    bt_inByte = BluetoothSerial.read();
    DEBUG_PRINT("bt_inByte: ");
    DEBUG_PRINTLN(bt_inByte);
   
    if (btMsgInProgress == true)
        {      // found some!!
            if (bt_inByte != BT_MSG_END_CHAR)
            {   // msg body  
                bt_message[bt_indx++] = bt_inByte;  // 1st array position=data
                if (bt_indx >= BT_MAX_MESSAGE_LEN)            // if index>= number of chars 
                    bt_indx = BT_MAX_MESSAGE_LEN - 1;         // index -1
            }
            else 
            {                          // end marker found
                // msg end
                DEBUG_PRINTLN("BT msg end DETECTED");
                DEBUG_PRINTLN("Done receiving BT message");
                bt_message[bt_indx] = '\0'; // terminate the string  
                btMsgInProgress = false;
                bt_indx = 0;                  // reset index
                bt_msg_ready = true;           // new data received flag
                DEBUG_PRINT("BT message: ");
                DEBUG_PRINTLN(bt_message);
            }
        } 
        else if (bt_inByte == BT_MSG_START_CHAR) {       // signal start of new data
          // msg start
          DEBUG_PRINTLN("BT msg start DETECTED");
          btMsgInProgress = true; }
    }
}


event_t parse_BT_msg() {   // Parse incoming BT message
    char appliance_type;
    int appliance_number, appliance_cmd;
    bt_indx = 0;
    int dimmer_value, led_plan_input;
    event_t bt_event;
    
    // BT cmd structure should be "{key}={value}" i.e. {BB=P},{A0=1},{DM=76}
     appliance_type = bt_message[0];      // A
     appliance_number = bt_message[1]-'0';    // 0
     if (bt_message[2] != '=')
     { // {A31=0} appliance numer has 2 digits
       appliance_number = appliance_number*10 + (bt_message[2]-'0');
       appliance_cmd = bt_message[4]-'0';
     }else{
       appliance_cmd = bt_message[3]-'0';       // 1 (bt_message[2] is '=')
     }
     switch(appliance_type){
          case 'B': // i.e. B=P : Button=Play
              DEBUG_PRINT("appliance_type");
              DEBUG_PRINT(": ");
              DEBUG_PRINTLN(appliance_type);
              DEBUG_PRINT("Button event");
              DEBUG_PRINT(": ");
              DEBUG_PRINTLN(appliance_cmd);
              
              // create button event based on appliance_cmd
              if (SINGLE_CLICK == appliance_cmd)
                return SINGLE_CLICK;
              if (DOUBLE_CLICK == appliance_cmd)
                return DOUBLE_CLICK;
              if (LONG_PRESS == appliance_cmd)
                return LONG_PRESS;
              if (SHORT_TAP == appliance_cmd)
                return SHORT_TAP;
              
              DEBUG_PRINTLN("Unknown button event");              
              break;
          case 'A': // i.e. A0=1 : Turn on pin 0
              DEBUG_PRINT("appliance_type");
              DEBUG_PRINT(": ");
              DEBUG_PRINTLN(appliance_type);
              DEBUG_PRINT("appliance_number");
              DEBUG_PRINT(": ");
              DEBUG_PRINTLN(appliance_number);
              DEBUG_PRINT("appliance_cmd");
              DEBUG_PRINT(": ");
              DEBUG_PRINTLN(appliance_cmd);
              
              // toggle appliance_number to state appliance_command
              relayToggle(appliance_number,appliance_cmd);
              
              DEBUG_PRINT("Turn ");
              DEBUG_PRINT(appliance_number);
              DEBUG_PRINT(" : ");
              DEBUG_PRINTLN(appliance_cmd);
              break;
           case 'D': // i.e. DM=33, set dimmer to 33%
              DEBUG_PRINT("appliance_type");
              DEBUG_PRINT(": ");
              DEBUG_PRINTLN(appliance_type);

              // set dimmer to 33%
              dimmer_value = atoi(&bt_message[3]);
              
              DEBUG_PRINT("Set dimmer to: ");
              DEBUG_PRINTLN(dimmer_value);
              break;
           case 'L': // i.e. LD=3 : set LEDs to plan 3
              DEBUG_PRINT("appliance_type");
              DEBUG_PRINT(": ");
              DEBUG_PRINTLN(appliance_type);
              
              // set LEDs to plan 3
              led_plan_input = atoi(&bt_message[3]);
              if ((led_plan_input < 0) or (led_plan_input >= LED_PLANS_COUNT))
                break;
              ledControl(led_plan_input);
              
              DEBUG_PRINT("set LEDs plan: ");
              DEBUG_PRINTLN(led_plan_input);
              break;
            case 'F': // i.e. FC=0010011 (Flamingo Color = TPRGBYW): Send 0010011 to Flamingo using LoRa
                      // this allows simulating the panels from the tablet app
              if ('C' == bt_message[2]) // FC=0010011 (FC=TPRGBYW) change flamingo color
                strncpy(LoRa_buff,&bt_message[3],FLAMINGO_MSG_LEN);
              if ('E' == bt_message[2]) // FE=30 (FE=TPE) (Flamingo Enable = Type_msg Panel_num Enable/Disable). TPE=230 disable panel 3, TPE=231 enable panel 3.
              {
                LoRa_buff[0] = '2';
                LoRa_buff[1] = bt_message[3];
                LoRa_buff[2] = bt_message[4];
                LoRa_buff[3] = 0;
                LoRa_send(LoRa_buff,strlen(LoRa_buff));
              }
              break;
           default:
              DEBUG_PRINT("appliance_type");
              DEBUG_PRINT(": ");
              DEBUG_PRINTLN(appliance_type);
              DEBUG_PRINT("Hey I recieved this weird message on bluetooth: ");
              DEBUG_PRINTLN(bt_message);
              break;     
        }
     return NO_EVENT;
}

void report_appliances_status() {
  static int prevApplianceState;
  int currApplianceState;
  // check if anything changed from last report to BT
  currApplianceState = 
    smokeMachineON +
    bubbleMachineON +
    whiteLightsON +
    colorLights_1_ON +
    colorLights_2_ON +
    flickersON +
    songPlaying;
  /// TODO: Maybe add backup pins flags??

  if (prevApplianceState != currApplianceState)
  {
    prevApplianceState = currApplianceState;
    DEBUG_PRINTLN("Sending appliances status to tablet");
    BluetoothSerial.print(BT_MSG_START_CHAR);
    BluetoothSerial.print("State=");
    BluetoothSerial.print(cur_state);
    BluetoothSerial.print(BT_MSG_SEPERATION_CHAR);
    BluetoothSerial.print("S=");
    BluetoothSerial.print(smokeMachineON);
    BluetoothSerial.print(BT_MSG_SEPERATION_CHAR);
    BluetoothSerial.print("B=");
    BluetoothSerial.print(bubbleMachineON);
    BluetoothSerial.print(BT_MSG_SEPERATION_CHAR);
    BluetoothSerial.print("W=");
    BluetoothSerial.print(whiteLightsON);
    BluetoothSerial.print(BT_MSG_SEPERATION_CHAR);
    BluetoothSerial.print("C1=");
    BluetoothSerial.print(colorLights_1_ON);
    BluetoothSerial.print(BT_MSG_SEPERATION_CHAR);
    BluetoothSerial.print("C2=");
    BluetoothSerial.print(colorLights_2_ON);
    BluetoothSerial.print(BT_MSG_SEPERATION_CHAR);
    BluetoothSerial.print("F=");
    BluetoothSerial.print(flickersON);
    BluetoothSerial.print(BT_MSG_SEPERATION_CHAR);
    BluetoothSerial.print("P=");
    BluetoothSerial.print(songPlaying);
    BluetoothSerial.print(BT_MSG_END_CHAR);
    BluetoothSerial.print("\r\n");
  }
  return;

//  S=smokeMachineON
//  B=bubbleMachineON
//  W=whiteLightsON
//  C1=colorLights_1_ON
//  C2=colorLights_2_ON
//  F=flickersON
//  P=songPlaying
}

void init_bt_buffer() {
  for (int i=0; i<BT_MAX_MESSAGE_LEN; i++)
    bt_message[i] = '\0';
  return;
}

// ----------- Bluetooth END -----------

// -------- LoRa --------
void LoRa_read(char * buff) {
   int i = 0;
   while (LoRa.available()) {
      buff[i] = LoRa.read();
      DEBUG_PRINT("LORA current data: ");
      DEBUG_PRINTLN(buff[i]);
      i++;
   }
   LoRa_RecvFlag = true;
   DEBUG_PRINTLN("data from lora");
   DEBUG_PRINTLN(buff);
}
 
void LoRa_send(char* buff,int len)
{
  DEBUG_PRINT("Send to LoRa: ");
  DEBUG_PRINTLN(buff);  
  LoRa.beginPacket();
  LoRa.write(buff, len);
  LoRa.endPacket(true);
  return;
}

event_t parse_LoRa_msg(char* buff) {
    event_t buzzer_lora_event = NO_EVENT;
    char msg_type;

    // read incoming LoRa data into buffer:
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("new LoRa data");
    LoRa_read(buff);
    DEBUG_PRINTLN(buff);
    // parse LoRa message:
    msg_type = buff[0]-'0';
    DEBUG_PRINT("msg_type: ");
    DEBUG_PRINTLN(msg_type);
    switch (msg_type) {
      case 3: // buzzer -> stage
              // msg = {TBB = Type Buzzer} Type=3
        DEBUG_PRINT("MSG buzzer -> stage: ");
        DEBUG_PRINTLN(buff);
        buzzer_lora_event = parse_buzzer_data(buff); // parse and handle recieved buzzer data
        break;
      default:// very strange...
      DEBUG_PRINTLN("No such message type");
        break; 
    }
  return buzzer_lora_event;
  }


// parse buzzer event from lora received packet
event_t parse_buzzer_data(char* buff) {
  // buzzer event -> stage
  // msg = {TBB = Type Buzzer Buzzer} Type=3
  char buzzer_char1 = buff[1]-'0';
  char buzzer_char2 = buff[2]-'0';
  
  switch (buzzer_char1) {
    case 'C':
      return SINGLE_CLICK;
    case 'D':
      return DOUBLE_CLICK;
    case 'L':
      return LONG_PRESS;
    case 'T':
      if ('S' == buzzer_char2)
        return SHORT_TAP;
      if ('L' == buzzer_char2)
        return LONG_TAP;
      if ('C' == buzzer_char2)
        return CONT_TAP;
    default:
      return NO_EVENT;
  }           
}

void init_lora_incoming_buff() {
  for (int i=0; i<LORA_BUFF_SIZE; i++) {    
    LoRa_incoming_buff[i] = 0;
  }
  return;
}

// -------- LoRa END --------
