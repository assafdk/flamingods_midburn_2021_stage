#ifndef LED_LIBRARY_H
#define LED_LIBRARY_H

#include <Arduino.h>
#include <FastLED.h>

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    (60*5)

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

typedef enum {
  LED_IDLE,
  LED_SHOW,
  LED_EASTER,
  LED_FUN
} ledState_t;

// function pointer type.
// this will be used to choose an led operation function (such as rainbow) and pass it to led_run() for activation
typedef void (*func_ptr_t)();

void led_setup();
void led_run(func_ptr_t);

void nextPattern();

// --- LED plan functions ---
void rainbow();
void rainbowWithGlitter();
void addGlitter(fract8);
void confetti();
void sinelon();
void bpm();
void juggle();
void runway();

#endif
