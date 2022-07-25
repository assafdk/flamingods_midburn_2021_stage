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

#define DATA_PIN    4
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    (60*5)

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120
#define LED_DELAY          0

// function pointer type.
// this will be used to choose an led operation function (such as rainbow) and pass it to led_run() for activation
typedef void (*func_ptr_t)();

void led_setup();
void led_run(func_ptr_t);

void nextPattern();

// --- LED plan functions ---
void rainbow();   // good for the flamingo
void rainbowWithGlitter();
void flickering_rainbow(); // doesn't work -- debug it
void addGlitter(fract8);
void confetti();  // gentle
void sinelon();   // red directional spread pixels
void bpm();       // rainbow that runs back and forth - Dima likes it for Flamingo party
void juggle();    // lots of different colors running (sin waves on top of on another?) - Dima likes it for Flamingo party
void runway();
void trail();     // one pixel traveling
void flow();
void back_flow();
void sawtooth();
void all_white();
void all_pink();
<<<<<<< HEAD
void random_solid();
=======
>>>>>>> 3d9cec259824c4588052b90225f9a5f384651ebf
void led_multiplan();

#endif
