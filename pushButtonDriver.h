//#define DEBUG

#ifndef PUSH_BUTTON_LIBRARY_H
#define PUSH_BUTTON_LIBRARY_H

#include <Arduino.h>

#define PUSH_BTN_PIN 8

#define LONG_PRESS_TIME 2000
#define RELEASE_TIMEOUT 300 // check button state after X ms from release

#define TAPPING_COOLDOWN_TIME 30000

#define SHORT_TAP_COUNT  30  // tap duration in millisec
#define LONG_TAP_COUNT   60  // tap duration in millisec

typedef enum {
  SINGLE_CLICK,
  DOUBLE_CLICK,
  TRIPPLE_CLICK,
  SHORT_TAP,
  LONG_TAP,
  LONG_PRESS,
  EASTER_TIMEOUT_EVENT,
  SHOW_TIMEOUT_EVENT,
  SONG_END,
  NO_EVENT
} event_t;

event_t btnPushSense();


//#define SHORT_TAP_DURATION  200  // tap duration in millisec
//#define LONG_TAP_DURATION   500  // tap duration in millisec

#endif
