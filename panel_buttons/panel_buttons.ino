
#include "Arduino.h"
#include "LoRa.h"

#define PANEL_ID '4'

#define DEBUG

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
#define RED_BUTTON_PIN 3
#define GREEN_BUTTON_PIN 4
#define BLUE_BUTTON_PIN 5
#define YELLOW_BUTTON_PIN 6
#define WHITE_BUTTON_PIN 7
#define SLEEP_TIME 10
#define BUTTON_TIMEOUT 120000000 // 2 minutes
#define PACKET_SIZE 8
#define ID_SEED (PANEL_ID - '0')
#define PACKET_TYPE '0'
#define ZERO_REDUNDANCY 50
#define LORA_INITIAL_SEND_INTERVAL  5     // initially send every 5 millis
#define LORA_MAX_SEND_INTERVAL      2000  // stop sending when interval hits this value
#define SPREAD_FACTOR   1.5   // multiply send interval by this value
#define WD_TIME 6000

void (* reset_func) (void) = 0;

typedef enum {
  FALLING_EDGE,
  RISING_EDGE,
  NO_EDGE
} edge_t;

uint8_t data[8] = {PACKET_TYPE, PANEL_ID, '0', '0', '0', '0', '0', '\0'};
uint8_t pseudo_data[8] = {PACKET_TYPE, PANEL_ID, '0', '0', '0', '0', '0', '\0'};
bool buttons_enabled[BUTTONS_COUNT] = {true, true, true, true, true};
uint8_t button_pins[BUTTONS_COUNT] = {RED_BUTTON_PIN, GREEN_BUTTON_PIN, BLUE_BUTTON_PIN, YELLOW_BUTTON_PIN, WHITE_BUTTON_PIN};
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

void sendDataToLORA(uint8_t * data, size_t data_size) {
  DEBUG_PRINTLN((char *)data);
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

void setup() {
  pinMode(RESET_PIN,INPUT_PULLUP);
  digitalWrite(RESET_PIN,HIGH);
  init_push_buttons();
  lora_send_interval = LORA_MAX_SEND_INTERVAL + 1;
  last_send_time = 0;
  randomSeed(ID_SEED);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  DEBUG_PRINTLN("Starting LoRa");
  if (!LoRa.begin(433E6)) {
    DEBUG_PRINTLN("Starting LoRa failed!");
    while (1);
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
    
    DEBUG_PRINTLN("button is LOW");
    data[payload_indx] = '1';
    continue;
  }
  return;
}

void loop() {
  uint32_t now = millis();
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
        break;

      case RISING_EDGE:
        DEBUG_PRINTLN("Rising");
        press_start_time[i] = 0;
        buttons_enabled[i] = true;
        data[i+2] = '0';
        prepare_lora_packet(data);
        lora_send_interval = LORA_INITIAL_SEND_INTERVAL;        
        send_flag = true;
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
  watchdog();
  delay(DEBOUNCE_DELAY);
}
