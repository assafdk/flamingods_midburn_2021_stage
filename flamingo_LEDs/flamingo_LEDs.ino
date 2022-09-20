#include <Arduino.h>
#include "ledDriver.h"
#include "LoRa.h"

#define DEBUG
#define DEBUG_PRINT_DELAY   5   // Seconds

#define jumperPin 8
#define ledPin 13

#ifdef DEBUG
  #define DEBUG_BEGIN(x)    Serial.begin(x); // open the serial port at 9600 bps
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
  #define DEBUG_DELAY   delay(0)
#else
  #define DEBUG_BEGIN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_DELAY
#endif

//#define TIMEOUT 10000                 // someone is pressing for 10 sec?
#define COM_COOLING_TIME   0         // I might not want to get new data too often to prevent glitches

#ifdef DEBUG
#define COMMAND_HOLD_TIME  600000       // IF THE LIGHTS ARE FLASHING INCREASE THIS VALUE!!! should be { prod>110  debug>170 }
#else
#define COMMAND_HOLD_TIME  60000       // IF THE LIGHTS ARE FLASHING INCREASE THIS VALUE!!! should be { prod>110  debug>170 }
#endif

#define LORA_DATA_LENGTH   20 

// Panels table
#define PANELS_COUNT 5
#define COLORS_COUNT 5                // number of colors on each panel: i.e. RGBYW
#define PANELS_ARRAY_ROWS    PANELS_COUNT+1
#define PANELS_ARRAY_COLUMNS COLORS_COUNT

#define SUM_ROW PANELS_COUNT

// Panels table columns
#define RED_COLUMN    0
#define GREEN_COLUMN  1
#define BLUE_COLUMN   2
#define YELLOW_COLUMN 3
#define WHITE_COLUMN  4


// ------- Global variables: -------
unsigned long now, party_start_time;
bool jumperState = HIGH;      // HIGH - Jumper is  disconnected. LOW - Jumper is connected
bool prevJumperState = HIGH;  // HIGH - Jumper was disconnected. LOW - Jumper is connected
int pressFlag = 0;            // 0 = no one is pressing any button on any panel. >0 otherwise. 

typedef enum {
  LED_IDLE,
  LED_SHOW,
  LED_EASTER,
  LED_FUN
} ledState_t;

ledState_t ledState;
unsigned long dataTimestamps[PANELS_COUNT+1] = {0};
int panelsRGB[PANELS_ARRAY_ROWS][PANELS_ARRAY_COLUMNS]; // a table that holds the current button status of all panels 
char incomingBuffer[LORA_DATA_LENGTH];
bool panelEnabled[PANELS_COUNT];

int msg_type, red, green, blue;

// ----------- functions: -----------
ledState_t getSerialCommand();
bool isJumperConnected();
void jumper_polling();
void run_old_flamingo();
void run_panel_LED_plan();
void reset_old_rows();
void update_total_RGB_values();
void init_RGB_array();
int parse_panel_data(char* incomingBuffer);
void parse_stage_data(char* incomingBuffer);
void parse_panel_enable(char* incomingBuffer);
void enable_all_panels();
void disable_all_panels();

// external function
extern void led_setup();
extern void led_run();
bool LoRa_isDataReady();
void LoRa_read(String buff);

// this holds a pointer to the current LED plan
// choose a plan from LED plan functions in "ledDriver.h"
func_ptr_t ledPlan;

void setup() {
  party_start_time = 0;
  // Configure the pins as input or output:
  pinMode(ledPin, OUTPUT);
  pinMode(jumperPin, INPUT_PULLUP);
  
  DEBUG_BEGIN(9600) // open the serial port at 9600 bps
  delay(300);
  LoRa.begin(433E6);
  DEBUG_PRINTLN("DEBUG MODE");
  // put your setup code here, to run once:
  ledState = LED_IDLE;
  led_setup();        // init led stuff
  ledPlan = bpm;  // choose first LED plan
  
  init_incoming_buffer();
  init_RGB_array();
  enable_all_panels();

    //debug
  // TPRGBYW
  // incomingBuffer[0] = '0';  // T 
  // incomingBuffer[1] = '3';  // P
  // incomingBuffer[2] = '1';  // R
  // incomingBuffer[3] = '0';  // G
  // incomingBuffer[4] = '0';  // B
  // incomingBuffer[5] = '0';  // Y
  // incomingBuffer[6] = '0';  // W
  // incomingBuffer[7] = '\0';
}

// -------------------------- main ---------------------------
void loop() {
  // DEBUG_DELAY;
  now = millis();                     // get current time
  
  // if jumper is connected then ignore all panels
  jumper_polling();

  // if jumper is not connected...
  if (LoRa_isDataReady()) {           // check if LoRa has incoming data waiting to be read
    // read incoming LoRa data into buffer:
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("new LoRa data");
    LoRa_read(incomingBuffer);
    // DEBUG_PRINTLN(incomingBuffer);
    // parse LoRa message:
    msg_type = incomingBuffer[0]-'0';
    DEBUG_PRINT("msg_type: ");
    DEBUG_PRINTLN(msg_type);
    switch (msg_type) {
      case 0: // panel -> flamingo
              // msg = {TPRGBYW = Type Panel COLORS} Type=0
        DEBUG_PRINT("MSG panel -> flamingo: ");
        DEBUG_PRINTLN(incomingBuffer);
        parse_panel_data(incomingBuffer); // parse and handle recieved panel data
        update_total_RGB_values();
        //run_panel_LED_plan();
        break;
      case 1: // stage -> flamingo
              // msg = {TC = Type Command} Type=1. i.e. {12} means Type=1 Command=2 (Command: 0,1,2=Idle,Show,Easter)
          DEBUG_PRINT("MSG stage -> flamingo: ");
          DEBUG_PRINTLN(incomingBuffer);
          parse_stage_data(incomingBuffer); // update main LED plan by what's going on on stage
        break;
      case 2: // stage (disable/enable panel) -> flamingo
              // msg = {TPE = Type Panel Enable} Type=2
        DEBUG_PRINT("MSG Disable/Enable: ");
        DEBUG_PRINTLN(incomingBuffer);
        parse_panel_enable(incomingBuffer);
        break;
      default:// very strange...
      DEBUG_PRINTLN("No such message type");
        break; 
    }
  init_incoming_buffer();    
  }
  reset_old_rows();           // clear panel row after buttons released
  update_total_RGB_values();  // to clear the SUM row if any row was deleted in the previous line
  
  // DEBUG_PRINT("pressFlag = ");
  // DEBUG_PRINTLN(pressFlag);
    
  // decide which LED plan to play now
  if (pressFlag) {
    run_panel_LED_plan();   // someone pressed a button on one of the panels
  } else {
    run_old_flamingo();     // no one is pressing any button on any of the panels
  }
}

// -------------------------- functions ---------------------------
void init_RGB_array() {
  for (int i =0; i < PANELS_ARRAY_ROWS; i++) {
    for (int j =0; j < PANELS_ARRAY_COLUMNS; j++) {
      panelsRGB[i][j] = 0;
    }
  }
}

bool LoRa_isDataReady() {
  return  LoRa.parsePacket();
}


void LoRa_read(char * buff) {
   int i = 0;
   while (LoRa.available()) {
      buff[i] = LoRa.read();
      // DEBUG_PRINT("current data: ");
      // DEBUG_PRINTLN(buff[i]);
      i++;
   }
  //  DEBUG_PRINTLN("data from lora");
  //  DEBUG_PRINTLN(buff);
}

// parse data coming from one of the button panels
int parse_panel_data(char* incomingBuffer) {
  int panel = incomingBuffer[1]-'0';    // panel number
  unsigned long dataTime = millis();
  DEBUG_PRINT("panel: ");
  DEBUG_PRINTLN(panel);
  if (false == panelEnabled[panel])
  { // ignore disabled panels
    DEBUG_PRINTLN("This panel was disabled");
    return 1;
  }
  if (dataTime > dataTimestamps[panel] + COM_COOLING_TIME) {
    dataTimestamps[panel] = dataTime;
    panelsRGB[panel][RED_COLUMN]    = (int)(incomingBuffer[2]-'0');
    panelsRGB[panel][GREEN_COLUMN]  = (int)(incomingBuffer[3]-'0');
    panelsRGB[panel][BLUE_COLUMN]   = (int)(incomingBuffer[4]-'0');
    panelsRGB[panel][YELLOW_COLUMN] = (int)(incomingBuffer[5]-'0');
    panelsRGB[panel][WHITE_COLUMN]  = (int)(incomingBuffer[6]-'0');
    return 0;
  }
  return 1;
}

// calculates the sum of values on each color row
// to find out how many buttons are pressed per color
void update_total_RGB_values() {
  int sum = 0;
  for (int col=0; col<COLORS_COUNT; col++){
    for (int pan=0; pan<PANELS_COUNT; pan++)
      sum += panelsRGB[pan][col];
    panelsRGB[SUM_ROW][col] = sum;
    sum = 0;
  }

  pressFlag = 0;
  for (int col=0; col<COLORS_COUNT; col++){
    pressFlag += panelsRGB[SUM_ROW][col];
  }
  return;
}

void reset_old_rows() {
  for (int panel=0; panel < PANELS_COUNT; panel++) {
    if ((dataTimestamps[panel] > 0) && (now > dataTimestamps[panel] + COMMAND_HOLD_TIME)) {
      DEBUG_PRINT("now: ");
      DEBUG_PRINTLN(now);
      DEBUG_PRINT("delta_t = ");
      DEBUG_PRINTLN(now - dataTimestamps[panel]);
      print_timestamps();         //debug
      for (int col=0; col < PANELS_ARRAY_COLUMNS; col++) {
        panelsRGB[panel][col] = 0;
        dataTimestamps[panel] = 0;
      }
      DEBUG_PRINT("old rows deleted. panel: ");
      DEBUG_PRINTLN(panel);
    }
  }
}

void run_panel_LED_plan() {
  red   = (255/PANELS_COUNT) * panelsRGB[SUM_ROW][RED_COLUMN];
  green = (255/PANELS_COUNT) * panelsRGB[SUM_ROW][GREEN_COLUMN];
  blue  = (255/PANELS_COUNT) * panelsRGB[SUM_ROW][BLUE_COLUMN];
  
  // yellow = red + green
  red   += ((255/PANELS_COUNT) * panelsRGB[SUM_ROW][YELLOW_COLUMN])/2;
  green += ((255/PANELS_COUNT) * panelsRGB[SUM_ROW][YELLOW_COLUMN])/2;

  // white = red + green + blue
  red   += ((255/PANELS_COUNT) * panelsRGB[SUM_ROW][WHITE_COLUMN])/3;
  green += ((255/PANELS_COUNT) * panelsRGB[SUM_ROW][WHITE_COLUMN])/3;
  blue  += ((255/PANELS_COUNT) * panelsRGB[SUM_ROW][WHITE_COLUMN])/3;

  if (red+green+blue > 0) {
    DEBUG_PRINT("RGBYW sums: ");  
    DEBUG_PRINT(panelsRGB[SUM_ROW][RED_COLUMN]);
    DEBUG_PRINT(panelsRGB[SUM_ROW][GREEN_COLUMN]);
    DEBUG_PRINT(panelsRGB[SUM_ROW][BLUE_COLUMN]);
    DEBUG_PRINT(panelsRGB[SUM_ROW][YELLOW_COLUMN]);
    DEBUG_PRINT(panelsRGB[SUM_ROW][WHITE_COLUMN]);
    DEBUG_PRINTLN("");

    DEBUG_PRINT("RGB colors: ");
    DEBUG_PRINT(red);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(green);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(blue);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("");

    print_panels_table();
  }
  solid_color(green,red,blue);
  return;
}

void run_old_flamingo() {
  switch (ledState) {
    case LED_IDLE:
      EVERY_N_SECONDS( DEBUG_PRINT_DELAY ) { DEBUG_PRINTLN("LED_IDLE"); }
      // led_multiplan();
      // runway();
      flow();      
      break;
    case LED_SHOW:
      EVERY_N_SECONDS( DEBUG_PRINT_DELAY ) { DEBUG_PRINTLN("LED_SHOW"); }
      // ledPlan = rainbow; //sawtooth; // flow;
      // ledPlan = bpm;
      ledPlan = all_pink;   // change to pounding pink...
      // run current LED plan
      led_run(ledPlan);
      break;
    case LED_EASTER:
      EVERY_N_SECONDS( DEBUG_PRINT_DELAY ) { DEBUG_PRINTLN("LED_EASTER"); }
      ledPlan = rainbow; //sawtooth; // flow;
      led_run(ledPlan);
      break;
    case LED_FUN:
      EVERY_N_SECONDS( DEBUG_PRINT_DELAY ) { DEBUG_PRINTLN("LED_FUN"); }
      ledPlan = flickering_rainbow;;
      //led_run(juggle);
      led_run(ledPlan);
      break;
  }
  FastLED.show();
  EVERY_N_SECONDS( DEBUG_PRINT_DELAY ) { DEBUG_PRINT("ledState = "); DEBUG_PRINTLN(ledState);}
}

// if jumper is connected then ignore all panels
// connection event - DISABLE all panels
// disconnection event - ENABLE all panels again 
void jumper_polling() {
  // Read out the jumperPin
  jumperState = digitalRead(jumperPin);
  if (prevJumperState == jumperState)
    // nothing changed
    return;

  if (LOW == jumperState)
    // Somewone just connected the jumper
    // ignore all panels
    disable_all_panels();

  if (HIGH == jumperState)
    // Somewone just disconnected the jumper
    // enable all panels again
    enable_all_panels();

  prevJumperState = jumperState;
  return;
}

bool isJumperConnected() {
  // Read out the jumperPin
  jumperState = digitalRead(jumperPin);
  if (LOW == jumperState) { // jumper connected
    return true;
  }
  if (HIGH == jumperState) { // jumper NOT connected
    return false;
  }
}

// debug function to print the panels table
void print_panels_table() {
  DEBUG_PRINTLN ("P R G B Y W");
  for (int panel=0; panel<PANELS_ARRAY_ROWS; panel++) {
    DEBUG_PRINT(panel);
    DEBUG_PRINT(" ");
    for (int col=0; col<PANELS_ARRAY_COLUMNS; col++) {
      DEBUG_PRINT(panelsRGB[panel][col]);
      DEBUG_PRINT(" ");
    }
    DEBUG_PRINTLN("");
  }
  return;   
}

void print_timestamps() {
  for (int panel=0; panel<PANELS_COUNT; panel++) {
    DEBUG_PRINT("panel: ");
    DEBUG_PRINT(panel);
    DEBUG_PRINT(" ; ");
    DEBUG_PRINT("timestamp: ");
    DEBUG_PRINTLN(dataTimestamps[panel]);
  }
  return;
}

// stage -> flamingo
// msg = {TC} - Type=1 Command=0 (0=Idle)
// store new main LED plan for later
void parse_stage_data(char* incomingBuffer) {
  // incomingBuffer[0] == '1' indicates stage message
  switch(incomingBuffer[1]) {
    case '0': //Idle
      ledState = LED_IDLE;
      break;
    case '1': //Show
      ledState = LED_SHOW;
      break;
    case '2': //Easter
      ledState = LED_EASTER;
      break;
    default:
      ledState = LED_FUN;
      break;
  }
  return;
}

// enable/disable panel according to command received from the tablet
void parse_panel_enable(char* incomingBuffer) {
  // stage (disable/enable panel) -> flamingo
  // msg = {TPE = Type Panel Enable} Type=2
  int panel_number;
  bool enable;

  panel_number = incomingBuffer[1]-'0';
  enable = incomingBuffer[2]-'0';
  
  panelEnabled[panel_number] = enable;
  return;
}

void enable_all_panels() {
  for (int i=0; i<PANELS_COUNT; i++)
    panelEnabled[i] = true;
  return;
}

void disable_all_panels() {
  for (int i=0; i<PANELS_COUNT; i++)
    panelEnabled[i] = false;
  return;
}

void init_incoming_buffer() {
  for (int i=0; i<LORA_DATA_LENGTH; i++)
    incomingBuffer[i] = '\0';
  return;
}


// #ifdef DEBUG
// ledState_t getSerialCommand() {
//   char incomingByte;
//   // Seial COM event: recieve byte from PC to know when song is over
//   if (Serial.available() > 0) {
//     incomingByte = Serial.read();
//     if ((incomingByte < '0') || (incomingByte > '3')) {
//       return ledState;
//     }
//     return (incomingByte - '0');
//   }
//   return ledState;
// }
// #endif
