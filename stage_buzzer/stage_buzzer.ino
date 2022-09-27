#include "pushButtonDriver.h"
#include "LoRa.h"

#define DEBUG
#define BUTTON_DELAY_MS  50      // push button debouncibg delay in millisec

// -------- LoRa --------
#define LORA_BUFF_SIZE  16      // no reason why I chose 16. has to be >7
#define LORA_MSG_SINGLE_CLICK   "3C"  // {TC*} - Message_Type=3 Command_Type=C Command="" 
#define LORA_MSG_DOUBLE_CLICK   "3D"  // {TC} - Type=3 Command_Type=D Command=""
#define LORA_MSG_TRIPLE_CLICK   LORA_MSG_DOUBLE_CLICK  // {TC*} - Type=3 Command_Type=D Command="" 
#define LORA_MSG_CONT_TAP "3TC"  // {TTC} - Type=3 Command_Type=T Command="C"  
#define LORA_MSG_SHORT_TAP "3TS"  // {TTC} - Type=3 Command_Type=T Command="S"  
#define LORA_MSG_LONG_TAP "3TL"  // {TTC} - Type=3 Command_Type=T Command="L" 
#define LORA_MSG_LONG_PRESS "3L"  // {TC*} - Type=3 Command_Type=L Command=""  
#define LORA_MSG_LEN    2

#ifdef DEBUG
#define DEBUG_PRINT(x)    Serial.print (x)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif


void (* reset_func) (void) = 0;
void sendDataToLORA(char * data, size_t data_size);


event_t btnEvent = NO_EVENT;
event_t event = NO_EVENT;
extern unsigned long now;
bool timeToCheckEvent = true;
uint32_t lastEventCheckTime;

void setup() {
  // put your setup code here, to run once:
  lastEventCheckTime = millis();
  event = NO_EVENT;
  // input pin
  pinMode(PUSH_BTN_PIN, INPUT_PULLUP);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  DEBUG_PRINTLN("Starting LoRa");
  if (!LoRa.begin(433E6)) {
    DEBUG_PRINTLN("Starting LoRa failed!");
    delay(3000);
    reset_func();
  }
}

void loop() {
  now = millis();
  // event handling
  timeToCheckEvent = ((lastEventCheckTime + BUTTON_DELAY_MS) < now);
  if (true == timeToCheckEvent) {
    lastEventCheckTime = now;
    event = getEvent();
    switch (event)
    {
      case SINGLE_CLICK:
        sendDataToLORA(LORA_MSG_SINGLE_CLICK, strlen(LORA_MSG_SINGLE_CLICK));
        break;

      case LONG_PRESS:
        sendDataToLORA(LORA_MSG_LONG_PRESS, strlen(LORA_MSG_LONG_PRESS));
        break;

      case SHORT_TAP:
        sendDataToLORA(LORA_MSG_LONG_TAP, strlen(LORA_MSG_LONG_TAP));
        break;
      case LONG_TAP:
        sendDataToLORA(LORA_MSG_SINGLE_CLICK, strlen(LORA_MSG_SINGLE_CLICK));
        break;
      case CONT_TAP:
        sendDataToLORA(LORA_MSG_CONT_TAP, strlen(LORA_MSG_CONT_TAP));
        break;

      case DOUBLE_CLICK:
        sendDataToLORA(LORA_MSG_DOUBLE_CLICK, strlen(LORA_MSG_DOUBLE_CLICK));
        break;
      case TRIPPLE_CLICK:
        sendDataToLORA(LORA_MSG_TRIPLE_CLICK, strlen(LORA_MSG_TRIPLE_CLICK));
        break;
      case NO_EVENT:
        break;
    }
  }
  event = NO_EVENT;
}


// -------------------------- functions ---------------------------
void sendDataToLORA(char * data, size_t data_size) {
  DEBUG_PRINT("Sending: ");
  DEBUG_PRINTLN((char *)data);
  LoRa.beginPacket();
  LoRa.write(data, data_size);
  LoRa.endPacket(true);
}

event_t getEvent() {
  // Button event:
  btnEvent = btnPushSense();
  if (NO_EVENT != btnEvent) {
    DEBUG_PRINT("BUTTON EVENT: ");
    DEBUG_PRINTLN(btnEvent);
    return btnEvent;
  }
  return NO_EVENT;
}
