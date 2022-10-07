#include <Arduino.h>
#include "ledDriver.h"

// #define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

bool solid_color_flag = false;
int delay_between_plans;
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
SimplePatternList gPatterns = {random_solid, flow, all_pink, rainbow}; //sinelon, confetti, juggle, bpm
//SimplePatternList gPatterns = { confetti };//, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void led_run(func_ptr_t ledPlan_func_ptr)
{
  (*ledPlan_func_ptr)(); 
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
}

void led_multiplan()
{
  DEBUG_PRINTLN("led_multiplan()");
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();
  DEBUG_PRINTLN(gCurrentPatternNumber);
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  // EVERY_N_SECONDS( 120 ) { nextPattern(); } // change patterns periodically
  delay_between_plans = random(2*60,5*60); // RANDOM DEALAY (2-5 MINUTES)
  //delay_between_plans = 3;
  EVERY_N_SECONDS( delay_between_plans ) { nextPattern(); solid_color_flag = false; } // change patterns periodically
  
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
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
  DEBUG_PRINTLN("rainbowWithGlitter()");
  rainbow();
  addGlitter(80);
}

void flickering_rainbow() 
{
  DEBUG_PRINTLN("flickering_rainbow()");
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  fadeToBlackBy( leds, NUM_LEDS, 250);
  //FastLED.setBrightness(BRIGHTNESS/2);
  //EVERY_N_MILLISECONDS(LED_DELAY) { FastLED.setBrightness(BRIGHTNESS); }
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  DEBUG_PRINTLN("confetti()");
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  //FastLED.show(); 
}

void sinelon()
{
  DEBUG_PRINTLN("sinelon()");
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  DEBUG_PRINTLN("bpm()");
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  DEBUG_PRINTLN("juggle()");
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
  DEBUG_PRINTLN("runway()");
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
  DEBUG_PRINTLN("trail()");
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

  DEBUG_PRINTLN("flow()");
  // slide the led in one direction
  if(cur_led < 0) {cur_led = NUM_LEDS-1;}
  leds[cur_led--] = CHSV(hue++, 255, 255);
  fadeToBlackBy(leds,NUM_LEDS,10);
  }

void back_flow() { 
  static uint8_t hue = 0;
  static int cur_led = 0;
  DEBUG_PRINTLN("back_flow()");
  // First slide the led in one direction
    if(cur_led > NUM_LEDS) {cur_led = 0;}
    leds[cur_led++] = CHSV(hue++, 255, 255);
    // Show the leds
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeToBlackBy(leds,NUM_LEDS,10);
    // Wait a little bit before we loop around and do it again
    delay(LED_DELAY);
  }

void sawtooth() { 
  static uint8_t hue = 0;
  // beat8 creates a sawtooth shape
  DEBUG_PRINTLN("sawtooth()");
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

void all_pink() {
DEBUG_PRINTLN("all_pink()");
//CRGB(BLUE,RED,GREEN)
//fill_solid( leds, NUM_LEDS, CRGB(255,20,147));
//fill_solid( leds, NUM_LEDS, CRGB(146,255,20));
//fill_solid( leds, NUM_LEDS, CRGB(105,255,180));
//fill_solid( leds, NUM_LEDS, CRGB(50,255,0)); // Pink Barbie
//fill_solid( leds, NUM_LEDS, CRGB(25,255,0)); // Pink Flamingo (Magenta)
fill_solid( leds, NUM_LEDS, CRGB(25,255,10)); // Pink Dima
//fill_solid( leds, NUM_LEDS, CRGB(50,218,24)); // Purple
//fill_solid( leds, NUM_LEDS, CRGB(20,255,110)); // Purple 

//int i;
//  for (i=1;i<NUM_LEDS;i++){
//    leds[i] = CRGB::HotPink;
////    FastLED.show();
//  }

}

void solid_color(int red, int green, int blue) {
  fill_solid(leds, NUM_LEDS, CRGB(red,green,blue));
  FastLED.show();
  return;
}

void random_solid() {
  int red=0;
  int green=0; 
  int blue=0;
  int min_color=30;
  int max_color=200;

  DEBUG_PRINTLN("random_solid()");
  //CRGB(BLUE,RED,GREEN)

  if (solid_color_flag) { return; }

  while ((red+green+blue < min_color) or (red+green+blue > max_color)){
    red = random(0,255);
    green = random(0,255);
    blue = random(0,255);
    }
  solid_color_flag = true;
  fill_solid( leds, NUM_LEDS, CRGB(green,red,blue));
}
