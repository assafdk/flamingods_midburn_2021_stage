#include "pushButtonDriver.h"
#include <Arduino.h>

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// internal functions declaration
void clearAllTimersCountersEvents();
event_t clickLookupTable(unsigned int);
event_t btnTapSense();

//typedef enum {
//  SINGLE_CLICK,
//  DOUBLE_CLICK,
//  TRIPPLE_CLICK,
//  SHORT_TAP,
//  LONG_TAP,
//  LONG_PRESS,
//  TIMEOUT,
//  SONG_END,
//  NO_EVENT
//} event_t;

event_t currentButtonEvent;
event_t lastEvent;
event_t tapEvent = NO_EVENT;

bool tapFlag = false;

//#define TAP_SUM 3
//#define MOVING_WINDOW_DURATION 2000
unsigned long shortTapCooldownTimer = 0;
unsigned long longTapCooldownTimer = 0;
unsigned long now = 0;
unsigned long lastShortTapTime = 0;
unsigned long lastLongTapTime = 0;
//unsigned long curWindowDuration = 0;
//unsigned long windowStartTime = 0;
unsigned long shortTapStartTime = 0;
unsigned long longTapStartTime = 0;

bool fallingEdge, risingEdge;

typedef enum {
  FALLING_EDGE,
  RISING_EDGE,
  NO_EDGE
} edge_t;
edge_t lastEdge;

#define PUSH_COUNT_SIZE 10
//unsigned int pushCount[PUSH_COUNT_SIZE];
int lastBtnState, curBtnState;

unsigned int fallingEdgeCounter = 0;
unsigned long highTime = 0;
unsigned long lowTime = 0;
unsigned long fallingEdgeTime = 0;
unsigned long risingEdgeTime = 0;
unsigned long tapDuration = 0;

event_t clickState;

event_t btnPushSense() {
  // counts the button pushes
  // OUT: SINGLE_PUSH, DOUBLE_PUSH, TRIPPLE_PUSH, SLOW_TAPPING, MID_TAPPING, FAST_TAPPING

  // --- Rising/Falling Edge : check if button state changed ---
  curBtnState = digitalRead(PUSH_BTN_PIN);
  now = millis();
  
  // check if btn state is changed
  if (lastBtnState!=curBtnState) {
    //egde detecded
    if (LOW == curBtnState){
      //falling edge
      DEBUG_PRINTLN("falling edge");
      fallingEdge = true;
      risingEdge = false;
      lastEdge = FALLING_EDGE;
    }
    if (HIGH == curBtnState){
      //rising edge
      DEBUG_PRINTLN("rising edge");
      risingEdge = true;
      fallingEdge = false;
      lastEdge = RISING_EDGE;
    }
  } else {
    fallingEdge = false;
    risingEdge = false;
  }
  lastBtnState = curBtnState; // for next iteration

  // --- translate edge to clicks ---
  // button is pressed
  if (fallingEdge) {
    fallingEdgeCounter++; // counts button clicks
    lowTime = 0;
    fallingEdgeTime=now;  // reset timer
    DEBUG_PRINTLN(fallingEdgeCounter);
  }

  // button is released
  if (risingEdge) {
    risingEdgeTime=now;  // reset timer
    highTime = 0;
  }

  // --- button release ---
  // calc how much time the button is released for
  if (RISING_EDGE == lastEdge) {
    highTime = now-risingEdgeTime;
  }
  // if button is released and not pressed again for a long time, it's time to determine how many clicks there were
  if ((RISING_EDGE == lastEdge) && (highTime > RELEASE_TIMEOUT)) {
    // button is released. return whatever the current state is.
    DEBUG_PRINTLN("Button released");
    DEBUG_PRINT("fallingEdgeCounter: ");
    DEBUG_PRINTLN(fallingEdgeCounter);
    clickState = clickLookupTable(fallingEdgeCounter);
    clearAllTimersCountersEvents();
    DEBUG_PRINT("Click state is ");
    DEBUG_PRINTLN(clickState);
    return clickState;
  }
  
  // --- long press ---
  // calc how much time the button is pressed for
  if (FALLING_EDGE == lastEdge) {
    lowTime  = now-fallingEdgeTime;
  }
  // if button is pressed continuously for a long time this is a LONG_PRESS
  if ((FALLING_EDGE == lastEdge) && (lowTime > LONG_PRESS_TIME)) {
    // one long press detected
    DEBUG_PRINTLN("button long press");
    clearAllTimersCountersEvents();
    return LONG_PRESS;
  }

  // --- rappid tapping ---
  // sense short/long tap
  tapEvent = btnTapSense();
  if (NO_EVENT != tapEvent) {
    DEBUG_PRINTLN("tapping event!");
    return tapEvent;
  }

  return NO_EVENT;
}

// -----------------------------------------------------------

void clearAllTimersCountersEvents() {
      highTime = 0;
      lowTime = 0;
      fallingEdgeCounter = 0;
      lastEdge = NO_EDGE;
      lastEvent = NO_EVENT;
      return;
    }

event_t clickLookupTable(unsigned int clickCounter) {
  DEBUG_PRINTLN("func: clickLookupTable");
  DEBUG_PRINT("clickCounter:");
  DEBUG_PRINTLN(clickCounter);
  switch (clickCounter) {
      case 1:
        return SINGLE_CLICK;
      case 2:
        return DOUBLE_CLICK;
      case 3:
        return TRIPPLE_CLICK;
      default:
        return NO_EVENT;
    }
  }

void clearTapTracking() {
    tapDuration = 0;
    tapFlag = false;
    return;
  }

event_t btnTapSense() {
  shortTapCooldownTimer = now - lastShortTapTime;
  longTapCooldownTimer = now - lastLongTapTime;  
//  DEBUG_PRINT("shortTapCooldownTimer = ");
//  DEBUG_PRINTLN(shortTapCooldownTimer);
//  DEBUG_PRINT("longTapCooldownTimer = ");
//  DEBUG_PRINTLN(longTapCooldownTimer);
  if ((fallingEdgeCounter > LONG_TAP_COUNT) && (longTapCooldownTimer > TAPPING_COOLDOWN_TIME)) {
    lastLongTapTime = now;
    return LONG_TAP;
  }
  if ((fallingEdgeCounter > SHORT_TAP_COUNT) && (shortTapCooldownTimer > TAPPING_COOLDOWN_TIME)) {
    lastShortTapTime = now;
    return SHORT_TAP;
  }
  return NO_EVENT;
}

// -------------------------------------- draft ----------------

//#define COUNT_ARRAY_LENGTH 10
//unsigned int countArray[COUNT_ARRAY_LENGTH] = {0,0,0,0,0,0,0,0,0,0};
//unsigned int sumArrayIndex = 0;
//unsigned int movingSum = 0;
//int i;
//
//unsigned int addToArrayAndCalcMovingSum(unsigned int count){
//  countArray[sumArrayIndex++] = count;
//  if (sumArrayIndex > COUNT_ARRAY_LENGTH) {
//    sumArrayIndex = 0;}
//
//  for (i=0;i<COUNT_ARRAY_LENGTH;i++){
//    movingSum += countArray[i];}
//    
//  return movingSum;
//}

//unsigned int clickSum = 0;
//event_t btnTapSense() {
//    //DEBUG_PRINTLN("func: btnTapSense");
//    // if there was already tapping recently, ignore the tapping
//    tapCooldownTimer = now - lastTapTime;
//    if (tapCooldownTimer < TAPPING_COOLDOWN_TIME) {
//      clearTapTracking();
//      return NO_EVENT;
//    }
//
//    // check moving sum
//    curWindowDuration = now - windowStartTime;
//    if (curWindowDuration > MOVING_WINDOW_DURATION) {
//      clickSum = addToArrayAndCalcMovingSum(fallingEdgeCounter); // calc how many clicks in this time window
//      DEBUG_PRINT("clickSum = ");
//      DEBUG_PRINTLN(clickSum);
//      windowStartTime = now;  // reset window
//      curWindowDuration = 0;  // reset window
//      
//      // tapping is fast enough?
//      if (clickSum > TAP_SUM) { 
//        if (false == tapFlag) {
//          // first tap
//          tapFlag = true;
//          tapStartTime = now;
//          tapDuration = 0;
//        } else {
//          // continues tap
//          tapDuration =  now - tapStartTime;
//        }
//        lastTapTime = now;
//        if (tapDuration > LONG_TAP_DURATION) {
//          return LONG_TAP;}
//        if (tapDuration > SHORT_TAP_DURATION) {
//          return SHORT_TAP;}
//      }
//        // no tap
//        clearTapTracking();
//        return NO_EVENT;
//      }
//      // tapping window is still open
//      // will check again in  the next  iteration
//      return NO_EVENT;
//  }

//
//  
//  if (curWindow > PUSH_WINDOW_TIME) {
//    total_pushes = array_sum(push_count, PUSH_COUNT_SIZE);
//    if (total_pushes > HIGH_BAR) {
//      return FAST_TAPPING;}
//    if (total_pushes > MID_BAR) {
//      return MID_TAPPING;}
//    if (total_pushes > LOW_BAR) {
//      return SLOW_TAPPING;}
//  }
//
//
//event_t btnTapSense(){
//    curWindow = now - windowStartTime;
//    if (curWindow > MOVING_WINDOW_TIME) {
//      windowStartTime = now; // reset window
//      clickSum = addToArrayAndCalcMovingSum(fallingEdgeCounter); // calc how many clicks in this time window
//      fallingEdgeCounter = 0; // reset click counter
//      if (clickSum > SLOW_TAP_SUM) && (false == slowTapFlag) {
//        tapDuration =  now - tapStartTime;
//        slowTapFlag = true;
//        tapTime = now;
//        return SLOW_TAP;
//      }
//      if (sum > MID_TAP_SUM) && (false == midTapFlag) {
//        tapDuration =  now - tapStartTime;
//        midTapFlag = true;
//        tapTime = now;
//        return MID_TAP;
//      }
//      if (sum > FAST_TAP_SUM) && (false == fastTapFlag) {
//        tapDuration =  now - tapStartTime;
//        fastTapFlag = true;
//        tapTime = now;
//        return FAST_TAP;
//      }
//      // no tap
//      clearTapTracking();
//    }
//    tapCooldownTimer = now - tapTime;
//    if (tapCooldownTimer > TAPPING_COOLDOWN_TIME) {
//      clearTapTracking();
//    }
//    return NO_EVENT;
//  }
