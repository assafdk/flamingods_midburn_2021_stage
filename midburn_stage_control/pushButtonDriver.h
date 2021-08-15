//#define DEBUG

#ifndef PUSH_BUTTON_LIBRARY_H
#define PUSH_BUTTON_LIBRARY_H

#include <Arduino.h>

//MEGA
//#define PUSH_BTN_PIN 53
//UNO
#define PUSH_BTN_PIN 13

#define LONG_PRESS_TIME 2000
#define RELEASE_TIMEOUT 300 // check button state after X ms from release

#define SHORT_TAPPING_COOLDOWN_TIME 10000
#define LONG_TAPPING_COOLDOWN_TIME  10000
#define CONT_TAPPING_COOLDOWN_TIME  1000

#define SHORT_TAP_COUNT  10  // tap this many times for SHORT_TAP event
#define LONG_TAP_COUNT   20  // tap this many times for LONG_TAP event
#define CONT_TAP_COUNT   (LONG_TAP_COUNT + 10)  // after this many taps you've entered CONTINUES TAP mode
#define CONT_TAP_CYCLE   10  // after tapping LONG_TAP_COUNT times, activate FX every CONT_TAP_CYCLE taps

typedef enum {
  SINGLE_CLICK,
  DOUBLE_CLICK,
  TRIPPLE_CLICK,
  SHORT_TAP,
  LONG_TAP,
  CONT_TAP,
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
