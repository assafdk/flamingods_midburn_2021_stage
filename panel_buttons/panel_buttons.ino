
#include "Arduino.h"
#include "LoRa.h"

#define PANEL_ID '4'

// #define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#define DEBOUNCE_DELAY 0
#define RESET_PIN A5

#define BUTTONS_COUNT 5

/// COLOR PINS:
#define RED_BUTTON_PIN 3
#define GREEN_BUTTON_PIN 4
#define BLUE_BUTTON_PIN 5
#define YELLOW_BUTTON_PIN 6
#define WHITE_BUTTON_PIN 7

#define RED_BUTTON_POWER_PIN    8
#define GREEN_BUTTON_POWER_PIN  9
#define BLUE_BUTTON_POWER_PIN   10
#define YELLOW_BUTTON_POWER_PIN 11
#define WHITE_BUTTON_POWER_PIN  12

#define SLEEP_TIME 10
#define BUTTON_TIMEOUT 120000000 // 2 minutes
#define PACKET_SIZE 8
#define ID_SEED (PANEL_ID - '0')
#define PACKET_TYPE '0'
#define ZERO_REDUNDANCY 50
#define LORA_INITIAL_SEND_INTERVAL  5     // initially send every 5 millis
#define LORA_MAX_SEND_INTERVAL      2000  // stop sending when interval hits this value
#define SPREAD_FACTOR   1.5   // multiply send interval by this value
#define WD_TIME 60000          // Watchdog - Reset Arduino every WD_TIME
#define ZERO_PACKETS_AT_STARTUP 3


// -- receive --
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


void (* reset_func) (void) = 0;


// ------- Global variables: -------
typedef enum {
  FALLING_EDGE,
  RISING_EDGE,
  NO_EDGE
} edge_t;

uint8_t data[8] = {PACKET_TYPE, PANEL_ID, '0', '0', '0', '0', '0', '\0'};
uint8_t pseudo_data[8] = {PACKET_TYPE, PANEL_ID, '0', '0', '0', '0', '0', '\0'};
bool buttons_enabled[BUTTONS_COUNT] = {true, true, true, true, true};
uint8_t button_pins[BUTTONS_COUNT] = {RED_BUTTON_PIN, GREEN_BUTTON_PIN, BLUE_BUTTON_PIN, YELLOW_BUTTON_PIN, WHITE_BUTTON_PIN};
uint8_t button_power_pins[BUTTONS_COUNT] = {RED_BUTTON_POWER_PIN, GREEN_BUTTON_POWER_PIN, BLUE_BUTTON_POWER_PIN, YELLOW_BUTTON_POWER_PIN, WHITE_BUTTON_POWER_PIN};
uint32_t press_start_time[BUTTONS_COUNT] = {0, 0, 0, 0, 0};
int buttons_current_state[BUTTONS_COUNT] = {HIGH, HIGH, HIGH, HIGH, HIGH};
int buttons_last_state[BUTTONS_COUNT] = {HIGH, HIGH, HIGH, HIGH, HIGH};
//uint32_t BUTTON_TIMEOUT = 1000 * 60 * 2;
uint32_t start_time = millis();
uint32_t last_send_time = millis();
uint32_t lora_send_interval;
bool send_flag = false;
bool time_to_reset = false;
bool all_buttons_high = false;
uint32_t now = millis();

// -- receiver --

unsigned long dataTimestamps[PANELS_COUNT+1] = {0};
int panelsRGB[PANELS_ARRAY_ROWS][PANELS_ARRAY_COLUMNS]; // a table that holds the current button status of all panels 
char incomingBuffer[LORA_DATA_LENGTH];
bool panelEnabled[PANELS_COUNT];

int msg_type, red, green, blue;

// functions
bool LoRa_isDataReady();
void LoRa_read(String buff);
void enable_all_panels();
void disable_all_panels();
void init_incoming_buffer();
void sendDataToLORA(uint8_t * data, size_t data_size);
edge_t detect_button_edge(int button);
void init_push_buttons();
bool is_button_pressed();
void watchdog();
void prepare_lora_packet(uint8_t * data);
void handle_incoming_data();
void init_buttons_power_pins();
void display_other_panels_state_on_buttons();
void update_total_RGB_values();

void setup() {
  pinMode(RESET_PIN,INPUT_PULLUP);
  digitalWrite(RESET_PIN,HIGH);
  init_push_buttons();
  init_buttons_power_pins();
  lora_send_interval = LORA_MAX_SEND_INTERVAL + 1;
  last_send_time = 0;
  randomSeed(ID_SEED);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  DEBUG_PRINTLN("Starting LoRa");
  if (!LoRa.begin(433E6)) {
    DEBUG_PRINTLN("Starting LoRa failed!");
    delay(3000);
    reset_func();    
    while (1);
  }
  for (int i = 0; i < ZERO_PACKETS_AT_STARTUP; i++) {
    sendDataToLORA(data, PACKET_SIZE);
    delay(5);
  }
  init_incoming_buffer();
  init_RGB_array();
  enable_all_panels();
}

// -------------------------- main ---------------------------
void loop() {
  now = millis();
  edge_t current_edge;

  for (int i = 0; i < BUTTONS_COUNT; i++) {
    current_edge = detect_button_edge(i);
    switch (current_edge) {
      case FALLING_EDGE:
        DEBUG_PRINTLN("Falling");
        press_start_time[i] = now;
        prepare_lora_packet(data);
        lora_send_interval = LORA_INITIAL_SEND_INTERVAL;
        send_flag = true;
        
        panelsRGB[PANEL_ID - '0'][i] = 1;
        dataTimestamps[PANEL_ID - '0'] = millis();    
        
        DEBUG_PRINT("Button ");
        DEBUG_PRINT(i);
        DEBUG_PRINTLN(": LOW");   
        print_panels_table();   
        break;

      case RISING_EDGE:
        DEBUG_PRINTLN("Rising");
        press_start_time[i] = 0;
        buttons_enabled[i] = true;
        data[i+2] = '0';
        prepare_lora_packet(data);
        lora_send_interval = LORA_INITIAL_SEND_INTERVAL;        
        send_flag = true;
        
        panelsRGB[PANEL_ID - '0'][i] = 0;
        print_panels_table();
        dataTimestamps[PANEL_ID - '0'] = millis();
                
        DEBUG_PRINT("Button ");
        DEBUG_PRINT(i);
        DEBUG_PRINTLN(": HIGH");
        print_panels_table();
        
        break;

      case NO_EDGE:
        if (buttons_current_state[i] == LOW and buttons_enabled[i]) {
          if (now - press_start_time[i] > BUTTON_TIMEOUT) {
            buttons_enabled[i] = false;
            data[i + 2] = '0';
            prepare_lora_packet(data);
            lora_send_interval = LORA_INITIAL_SEND_INTERVAL;
            send_flag = true;
            }
          }

        break;
      default:
        DEBUG_PRINTLN("Weird stuff here");
    }
  }


  // -- BEGIN: LORA Send --  
  if (lora_send_interval > LORA_MAX_SEND_INTERVAL) {
    send_flag = false;
  }

  if ((send_flag) && (now > last_send_time + lora_send_interval)) {
    sendDataToLORA(data, PACKET_SIZE);
    lora_send_interval = (uint32_t)(lora_send_interval * SPREAD_FACTOR);
    last_send_time = now;
  }
  // -- END: LORA SEND --

  // -- BEGIN: LORA RECEIVE -- 
  // check if LoRa has incoming data waiting to be read
  if (LoRa_isDataReady()) {
    handle_incoming_data();
    print_panels_table();
  }
  // -- END: LORA RECEIVE -- 
  update_total_RGB_values();  // clear the SUM row if any row was deleted in the previous line
  reset_old_rows();           // clear panel row after buttons released
  display_other_panels_state_on_buttons();

  watchdog();
  delay(DEBOUNCE_DELAY);
}
// -------------------------- END: Main ---------------------------


// -------------------------- functions ---------------------------
void sendDataToLORA(uint8_t * data, size_t data_size) {
  // DEBUG_PRINT("Sending: ");
  // DEBUG_PRINTLN((char *)data);
  LoRa.beginPacket();
  LoRa.write(data, data_size);
  LoRa.endPacket(true);
}

edge_t detect_button_edge(int button) {
  // check if btn state is changed
  buttons_current_state[button] = digitalRead(button_pins[button]);
  if (buttons_last_state[button] != buttons_current_state[button]) {
    buttons_last_state[button] = buttons_current_state[button];
    // edge detecded
    if (LOW == buttons_current_state[button]) {
      return FALLING_EDGE;
    }
    if (HIGH == buttons_current_state[button]) {
      return RISING_EDGE;
    }
  }
  buttons_last_state[button] =  buttons_current_state[button];
  return NO_EDGE;
}


void init_push_buttons() {
  // loop on all buttons
  for (int i = 0; i < BUTTONS_COUNT; i++) {
    pinMode(button_pins[i], INPUT_PULLUP);
  }
}

bool is_button_pressed() {
  for (int i = 0; i < BUTTONS_COUNT; i++) {
    if (buttons_current_state[i] == LOW) {
      return true;
    }
  }
  return false;
}

void watchdog() {
  if((millis() - last_send_time > WD_TIME) && !is_button_pressed()) {
    reset_func();
  }
}

void prepare_lora_packet(uint8_t * data) {
  int payload_indx = 2;
  int curr_button = 0;
  for (; payload_indx < 7; payload_indx++, curr_button++) {
    
    if (!buttons_enabled[curr_button]) {
      DEBUG_PRINTLN("button is DISABLED");
      data[payload_indx] = '0';
      continue;
    }
    if (buttons_current_state[curr_button]) {
      DEBUG_PRINT(curr_button);
      DEBUG_PRINTLN(" button is HIGH");
      data[payload_indx] = '0';
      continue;
    }
    DEBUG_PRINT(curr_button);
    DEBUG_PRINTLN(" button is LOW");
    data[payload_indx] = '1';
    continue;
  }
  return;
}

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
  return;
}

void reset_old_rows() {
  for (int panel=0; panel < PANELS_COUNT; panel++) {
    if ((dataTimestamps[panel] > 0) && (now > dataTimestamps[panel] + COMMAND_HOLD_TIME)) {
      DEBUG_PRINT("now: ");
      DEBUG_PRINTLN(now);
      DEBUG_PRINT("delta_t = ");
      DEBUG_PRINTLN(now - dataTimestamps[panel]);
      for (int col=0; col < PANELS_ARRAY_COLUMNS; col++) {
        panelsRGB[panel][col] = 0;
        dataTimestamps[panel] = 0;
      }
      DEBUG_PRINT("old rows deleted. panel: ");
      DEBUG_PRINTLN(panel);
    }
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

void handle_incoming_data() {
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
  return;
  }

void init_buttons_power_pins() {
    pinMode(RED_BUTTON_POWER_PIN,OUTPUT);
    pinMode(GREEN_BUTTON_POWER_PIN,OUTPUT);
    pinMode(BLUE_BUTTON_POWER_PIN,OUTPUT);
    pinMode(YELLOW_BUTTON_POWER_PIN,OUTPUT);
    pinMode(WHITE_BUTTON_POWER_PIN,OUTPUT);

    digitalWrite(RED_BUTTON_POWER_PIN,HIGH);
    digitalWrite(GREEN_BUTTON_POWER_PIN,HIGH);
    digitalWrite(BLUE_BUTTON_POWER_PIN,HIGH);
    digitalWrite(YELLOW_BUTTON_POWER_PIN,HIGH);
    digitalWrite(WHITE_BUTTON_POWER_PIN,HIGH);
    
    DEBUG_PRINTLN("RED_BUTTON_POWER_PIN = HIGH");
    DEBUG_PRINTLN("GREEN_BUTTON_POWER_PIN = HIGH");
    DEBUG_PRINTLN("BLUE_BUTTON_POWER_PIN = HIGH");
    DEBUG_PRINTLN("YELLOW_BUTTON_POWER_PIN = HIGH");
    DEBUG_PRINTLN("WHITE_BUTTON_POWER_PIN = HIGH");
}

void display_other_panels_state_on_buttons() {
    digitalWrite(RED_BUTTON_POWER_PIN,panelsRGB[SUM_ROW][RED_COLUMN] == 0);
    digitalWrite(GREEN_BUTTON_POWER_PIN,panelsRGB[SUM_ROW][GREEN_COLUMN] == 0);
    digitalWrite(BLUE_BUTTON_POWER_PIN,panelsRGB[SUM_ROW][BLUE_COLUMN] == 0);
    digitalWrite(YELLOW_BUTTON_POWER_PIN,panelsRGB[SUM_ROW][YELLOW_COLUMN] == 0);
    digitalWrite(WHITE_BUTTON_POWER_PIN,panelsRGB[SUM_ROW][WHITE_COLUMN] == 0);

    // DEBUG_PRINT("RED_BUTTON_POWER_PIN: ");
    // DEBUG_PRINTLN(panelsRGB[SUM_ROW][RED_COLUMN] == 0);
    // DEBUG_PRINT("GREEN_BUTTON_POWER_PIN: ");
    // DEBUG_PRINTLN(panelsRGB[SUM_ROW][GREEN_COLUMN] == 0);
    // DEBUG_PRINT("BLUE_BUTTON_POWER_PIN: ");
    // DEBUG_PRINTLN(panelsRGB[SUM_ROW][BLUE_COLUMN] == 0);
    // DEBUG_PRINT("YELLOW_BUTTON_POWER_PIN: ");
    // DEBUG_PRINTLN(panelsRGB[SUM_ROW][YELLOW_COLUMN] == 0);
    // DEBUG_PRINT("WHITE_BUTTON_POWER_PIN: ");
    // DEBUG_PRINTLN(panelsRGB[SUM_ROW][WHITE_COLUMN] == 0);    
    return;
}

