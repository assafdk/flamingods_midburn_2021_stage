#include <Arduino.h>
#include "ledDriver.h"

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

CRGB leds[NUM_LEDS];

void led_setup() {
  delay(3000); // 3 second delay for recovery
  DEBUG_PRINTLN("led_setup()");
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
//SimplePatternList gPatterns = { confetti };//, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void led_run(func_ptr_t ledPlan_func_ptr)
{
  // Call the current pattern function once, updating the 'leds' array
  //gPatterns[gCurrentPatternNumber]();
  (*ledPlan_func_ptr)();
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  //FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
//  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
//  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  DEBUG_PRINTLN("rainbow()");
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void runway() { 
  int i;
  // Turn the LED on, then pause
  for (i=0;i<NUM_LEDS;i++){
    leds[i] = CRGB::Red;
    FastLED.show();
    //delay(50);
  }
  
  //delay(500);
  // Now turn the LED off, then pause
  for (i=0;i<NUM_LEDS;i++){
    leds[i] = CRGB::Black;
    FastLED.show();
    //delay(50);
  }
  
  //delay(250);
}

void trail() { 
  int i;
  for (i=1;i<NUM_LEDS;i++){
    leds[i] = CRGB::Red;
    leds[i-1] = CRGB::Black;
    FastLED.show();
    delay(20);
  }
}

// ------------ flow --------------

//void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250);} }

void flow() { 
  static uint8_t hue = 0;
  static int cur_led = 0;
  
  // slide the led in one direction
  if(cur_led < 0) {cur_led = NUM_LEDS-1;}
  leds[cur_led--] = CHSV(hue++, 255, 255);
  fadeToBlackBy(leds,NUM_LEDS,10);
  }

void back_flow() { 
  static uint8_t hue = 0;
  static int cur_led = 0;
  // First slide the led in one direction
  
//  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red
//    if(hue > 255) {hue = 0;}
    if(cur_led > NUM_LEDS) {cur_led = 0;}
    leds[cur_led++] = CHSV(hue++, 255, 255);
    // Show the leds
//    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeToBlackBy(leds,NUM_LEDS,10);
    // Wait a little bit before we loop around and do it again
//    delay(LED_DELAY);
  }

void sawtooth() { 
  static uint8_t hue = 0;
  // beat8 creates a sawtooth shape
  int pos = map(beat8(40,0),0,255,0,NUM_LEDS-1);
  leds[pos] = CHSV(hue, 255, 255);

  EVERY_N_MILLISECONDS(LED_DELAY) {
    hue++;
  }
  fadeToBlackBy(leds,NUM_LEDS,3);
}

//void flow_RGB() { 
//  static uint8_t hue = 0;
//  CRGB rgb_leds[NUM_LEDS];
//
//  triwave8()
//  uint8_t sinBeat = beatsin8(30,0,NUM_LEDS-1,0,0);
//  rgb_leds[sinBeat] = 
//  // First slide the led in one direction
//  for(int i = 0; i < NUM_LEDS; i++) {
//    // Set the i'th led to red
////    if(hue > 255) {hue = 0;}
//    leds[i] = CHSV(hue++, 255, 255);
//    // Show the leds
//    FastLED.show(); 
//    // now that we've shown the leds, reset the i'th led to black
//    // leds[i] = CRGB::Black;
//    fadeToBlackBy(leds,NUM_LEDS,10);
//  }
//  // Now go in the other direction.  
//  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
//    // Set the i'th led to red
////    if(hue > 255) {hue = 0;} 
//    leds[i] = CHSV(hue++, 255, 255);
//    // Show the leds
//    FastLED.show();
//    // now that we've shown the leds, reset the i'th led to black
//    // leds[i] = CRGB::Black;
////    fadeall();
//    // Wait a little bit before we loop around and do it again
//    delay(LED_DELAY);
//  }
//}

void all_white() { 
  int i;
  for (i=1;i<NUM_LEDS;i++){
    leds[i] = CRGB::White;
    FastLED.show();
  }
//  delay(5000);
}
