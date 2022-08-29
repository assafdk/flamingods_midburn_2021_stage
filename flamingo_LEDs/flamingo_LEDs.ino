#include <Arduino.h>
#include "ledDriver.h"

#define DEBUG
#define jumperPin 8
#define ledPin 13

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
  #define DEBUG_DELAY   delay(0)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_DELAY
#endif

//#define TIMEOUT 10000                 // someone is pressing for 10 sec?
#define COM_COOLING_TIME  50         // I might don't want to get new data too often to prevent glitches
#define COMMAND_HOLD_TIME  100
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
bool jumperState = HIGH;  // HIGH - Jumper is disconnected. LOW - Jumper is connected
int pressFlag = 0;        // 0 = no one is pressing any button on any panel. >0 otherwise. 

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
bool panelEnabled[PANELS_COUNT] = {1};

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

// external function
extern void led_setup();
extern void led_run();
bool LoRa_isDataReady() {return Serial.available();}
void LoRa_read(char* buff) {Serial.readBytes(buff,LORA_DATA_LENGTH); return;}

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
  
  init_RGB_array();

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
  DEBUG_DELAY;
  now = millis();                     // get current time
  // jumper_polling();                   // poll jumper to choose between 2 led plans (ledState)

  if (isJumperConnected()) {
    // if jumper is connected then override or something...
    // otherwise just continue with the normal plan
    led_multiplan();
    return;
  }

  // if jumper is not connected...
  if (LoRa_isDataReady()) {           // check if LoRa has incoming data waiting to be read
    // read incoming LoRa data into buffer:
    DEBUG_PRINTLN("new LoRa data");
    LoRa_read(incomingBuffer);
    
    // parse LoRa message:
    msg_type = incomingBuffer[0]-'0';
    DEBUG_PRINT("msg_type: ");
    DEBUG_PRINTLN(msg_type);
    switch (msg_type) {
      case 0: // panel -> flamingo
              // msg = {TPRGBYW = Type Panel COLORS} Type=0
        parse_panel_data(incomingBuffer); // parse and handle recieved panel data
        update_total_RGB_values();
        run_panel_LED_plan();
        break;
      case 1: // stage -> flamingo
              // msg = {TC = Type Command} Type=1. i.e. {12} means Type=1 Command=2 (Command: 0,1,2=Idle,Show,Easter)
          parse_stage_data(incomingBuffer); // update main LED plan by what's going on on stage
        break;
      case 2: // stage (disable/enable panel) -> flamingo
              // msg = {TPE = Type Panel Enable} Type=2
        parse_panel_enable(incomingBuffer);
        break;
      default:// very strange...
      DEBUG_PRINTLN("No such message type");
        break; 
    }
  }
  reset_old_rows();           // clear panel row after buttons released
  update_total_RGB_values();  // to clear the SUM row if any row was deleted in the previous line
  
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
  solid_color(red,green,blue);
  return;
}

void run_old_flamingo() {
  switch (ledState) {
    case LED_IDLE:
      // jumper is disconnected
      EVERY_N_SECONDS( 1 ) { DEBUG_PRINTLN("LED_IDLE"); }
      // ledPlan = rainbow; //sawtooth; // flow;
      //ledPlan = bpm;
      ledPlan = all_pink;   // change to pounding pink...
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
  Serial.println ("P R G B Y W");
  for (int panel=0; panel<PANELS_ARRAY_ROWS; panel++) {
    Serial.print(panel);
    Serial.print(" ");
    for (int col=0; col<PANELS_ARRAY_COLUMNS; col++) {
      Serial.print(panelsRGB[panel][col]);
      Serial.print(" ");
    }
    Serial.println("");
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
