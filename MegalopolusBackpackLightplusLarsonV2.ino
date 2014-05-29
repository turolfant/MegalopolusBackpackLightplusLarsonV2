/*
LED VU meter for Arduino and Adafruit NeoPixel LEDs.
 
Hardware requirements:
 - Most Arduino or Arduino-compatible boards (ATmega 328P or better).
 - Adafruit Electret Microphone Amplifier (ID: 1063)
 - Adafruit Flora RGB Smart Pixels (ID: 1260)
   OR
 - Adafruit NeoPixel Digital LED strip (ID: 1138)
 - Optional: battery for portable use (else power through USB or adapter)
Software requirements:
 - Adafruit NeoPixel library
 
Connections:
 - 3.3V to mic amp +
 - GND to mic amp -
 - Analog pin to microphone output (configurable below)
 - Digital pin to LED data input (configurable below)
 See notes in setup() regarding 5V vs. 3.3V boards - there may be an
 extra connection to make and one line of code to enable or disable.
 
Written by Adafruit Industries.  Distributed under the BSD license.
This paragraph must be included in any redistribution.
*/
#include <Boards.h>
#include <Firmata.h>

#include <Adafruit_NeoPixel.h>
#include <math.h>
boolean usingInterrupt = false;
#define N_RING  16                // Number of pixels in Neopixelring
#define N_STRAND  12              // Number of pixels in Strand
#define MIC_PIN   9               // Microphone is attached to this analog pin
#define LED_RING_PIN    6         // NeoPixel LED Ring is connected to this pin
#define LED_STRAND_PIN    12      // NeoPixel LED Strand is connected to this pin
#define DC_OFFSET  0              // DC offset in mic signal - if unusure, leave 0
#define NOISE     10              // Noise/hum/interference in mic signal
#define SAMPLES   60              // Length of buffer for dynamic level adjustment
#define TOP       (N_STRAND + 2)  // Allow dot to go slightly off scale
#define PEAK_FALL 40              // Rate of peak falling dot


byte
  peak      = 0,      // Used for falling dot
  dotCount  = 0,      // Frame counter for delaying dot-falling speed
  volCount  = 0;      // Frame counter for storing past volume data
int
  vol[SAMPLES],       // Collection of prior volume samples
  lvl       = 10,      // Current "dampened" audio level
  minLvlAvg = 0,      // For dynamic adjustment of graph low & high
  maxLvlAvg = 512;
//byte peak = 16;      // Peak level of column; used for falling dots
//unsigned int sample;

//byte dotCount = 0;  //Frame counter for peak dot
//byte dotHangCount = 0; //Frame counter for holding peak dot


// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel ring = Adafruit_NeoPixel(N_RING, LED_RING_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_STRAND, LED_STRAND_PIN, NEO_GRB + NEO_KHZ800);

// Here is where you can put in your favorite colors that will appear!
// just add new {nnn, nnn, nnn}, lines. They will be picked out randomly
//                          R   G   B
uint8_t myColors[][3] = {{255, 000, 000},   // red
                         {000, 255, 00},   // yellow 
                         {000, 000, 255},   // blue
                          };
                               
// don't edit the line below
#define FAVCOLORS sizeof(myColors) / 3

int mode = 0;
// Pushbutton setup
int buttonPin = 10;             // the number of the pushbutton pin
int buttonState;               // the current reading from the input pin
int lastButtonState = HIGH;    // the previous reading from the input pin
long buttonHoldTime = 0;         // the last time the output pin was toggled
long buttonHoldDelay = 1000;      // how long to hold the button down

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;     // the last time the output pin was toggled
long debounceDelay = 50;       // the debounce time; increase if the output flickers
long menuDelay = 2500;
long menuTime;

int pos = 6, dir = 1; // Position, direction of "eye" for Larson_Scanner
unsigned long color;
//unsigned long c;
//unsigned int  i;

void setup() {
memset(vol, 0, sizeof(vol));
  ring.begin();
  ring.show(); // Initialize all pixels to 'off'
  strip.begin();
  strip.show();
  pinMode(buttonPin, INPUT);// Make input & enable pull-up resistors on switch pins for pushbutton
  digitalWrite(buttonPin, HIGH); 
}

void loop() {
  
 {
   
 
   // read the state of the switch into a local variable:
  int buttonState = digitalRead(buttonPin);
  
  if (buttonState == LOW) {
    buttonCheck();
  }
  
  lastButtonState = buttonState;
  
  
  }
   
  if (mode == 0) {
  ring.begin();
  ring.show(); // Initialize all pixels to 'off'
  strip.begin();
  strip.show();
  LarsonScanRing(); //Larson Scan Circle
  }
  
  if (mode == 1) {
  ring.begin();
  ring.show(); // Initialize all pixels to 'off'
  strip.begin();
  strip.show();
  LarsonScanStrip(); // Larson Scan Side
  }
  
  if (mode == 2) {
  ring.begin();
  ring.show(); // Initialize all pixels to 'off'
  strip.begin();
  strip.show();
  FlashRandomRing(5, 3);
  FlashRandomString(5, 3);
//  LarsonScanStrip();  
}
  
 if (mode == 3) {
  ring.begin();
  ring.show(); // Initialize all pixels to 'off'
  strip.begin();
  strip.show();
  Rainbow(20);
  }
 if (mode == 4) {
  ring.begin();
  ring.show(); // Initialize all pixels to 'off'
  strip.begin();
  strip.show();
//  LarsonScanRing();
  Amplitie();
  }
} 
void FlashRandomRing(int wait, uint8_t howmany) {
 
  for(uint16_t i=0; i<howmany; i++) {
    // pick a random favorite color!
    int c = random(FAVCOLORS);
    int red = myColors[c][0];
    int green = myColors[c][1];
    int blue = myColors[c][2]; 
 
    // get a random pixel from the list
    int j = random(ring.numPixels());
    
    // now we will 'fade' it in 5 steps
    for (int x=0; x < 5; x++) {
      int r = red * (x+1); r /= 5;
      int g = green * (x+1); g /= 5;
      int b = blue * (x+1); b /= 5;
      
      ring.setPixelColor(j, ring.Color(r, g, b));
      ring.show();
      delay(wait);
    }
    // & fade out in 5 steps
    for (int x=5; x >= 0; x--) {
      int r = red * x; r /= 5;
      int g = green * x; g /= 5;
      int b = blue * x; b /= 5;
      
      ring.setPixelColor(j, ring.Color(r, g, b));
      ring.show();
      delay(wait);
    }
  }
  // LEDs will be off when done (they are faded to 0)
}
void FlashRandomString(int wait, uint8_t howmany) {
 
  for(uint16_t i=0; i<howmany; i++) {
    // pick a random favorite color!
    int c = random(FAVCOLORS);
    int red = myColors[c][0];
    int green = myColors[c][1];
    int blue = myColors[c][2]; 
 
    // get a random pixel from the list
    int j = random(strip.numPixels());
    
    // now we will 'fade' it in 5 steps
    for (int x=0; x < 5; x++) {
      int r = red * (x+1); r /= 5;
      int g = green * (x+1); g /= 5;
      int b = blue * (x+1); b /= 5;
      
      strip.setPixelColor(j, strip.Color(r, g, b));
      strip.show();
      delay(wait);
    }
    // & fade out in 5 steps
    for (int x=5; x >= 0; x--) {
      int r = red * x; r /= 5;
      int g = green * x; g /= 5;
      int b = blue * x; b /= 5;
      
      strip.setPixelColor(j, strip.Color(r, g, b));
      strip.show();
      delay(wait);
    }
  }
  // LEDs will be off when done (they are faded to 0)
}
// Fill the strip at once with one color
void SetFullStrip(uint32_t c) {
  for(uint16_t i=0; i<ring.numPixels(); i++) {
      ring.setPixelColor(i, c);
      ring.setBrightness(100);
      ring.show();
      }
}

void Rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<ring.numPixels(); i++) {
      ring.setBrightness(100);
      ring.setPixelColor(i, Wheel_a((i+j) & 255));
      strip.setBrightness(100);
      strip.setPixelColor(i, Wheel_a((i+j) & 255));
    }
    ring.show();
    strip.show();
    delay(wait);
  }
}
void buttonCheck() {
  menuTime = millis();
  int buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonHoldTime = millis();
  }
  
  if (buttonState == LOW && lastButtonState == LOW) {
    if ((millis() - buttonHoldTime) > buttonHoldDelay) {
      
      if(mode == 4) {
        mode = 0;
        SetFullStrip(ring.Color(0, 0, 0));
        buttonHoldTime = millis();
      } else {
        mode = mode + 1;
        SetFullStrip(ring.Color(0, 0, 0));
        buttonHoldTime = millis();
      }
    }
  }
}

void Amplitie()
{
  uint8_t  i;
  uint16_t minLvl, maxLvl;
  int      n, height;

 

  n   = analogRead(MIC_PIN);                        // Raw reading from mic 
  n   = abs(n - 512 - DC_OFFSET); // Center on zero
  n   = (n <= NOISE) ? 0 : (n - NOISE);             // Remove noise/hum
  lvl = ((lvl * 7) + n) >> 3;    // "Dampened" reading (else looks twitchy)

  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

  if(height < 0L)       height = 0;      // Clip output
  else if(height > TOP) height = TOP;
  if(height > peak)     peak   = height; // Keep 'peak' dot at top


  // Color pixels based on Rainbow gradient
  for(i=0; i<N_STRAND; i++) {
    if(i >= height)               strip.setPixelColor(i,   0,   0, 0);
    else strip.setPixelColor(i,Wheel_b(map(i,0,strip.numPixels()-1,30,150)));
    
  }



  // Draw peak dot  
  if(peak > 0 && peak <= N_STRAND-1) strip.setPixelColor(peak,Wheel_b(map(peak,0,strip.numPixels()-1,30,150)));
  
   strip.show(); // Update strip

// Every few frames, make the peak pixel drop by 1:

    if(++dotCount >= PEAK_FALL) { //fall rate 
      
      if(peak > 0) peak--;
      dotCount = 0;
    }



  vol[volCount] = n;                      // Save sample for dynamic leveling
  if(++volCount >= SAMPLES) volCount = 0; // Advance/rollover sample counter

  // Get volume range of prior frames
  minLvl = maxLvl = vol[0];
  for(i=1; i<SAMPLES; i++) {
    if(vol[i] < minLvl)      minLvl = vol[i];
    else if(vol[i] > maxLvl) maxLvl = vol[i];
  }
  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
  minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6; // Dampen min/max levels
  maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6; // (fake rolling average)

}

void LarsonScanRing() {
  int j;

  // Draw 5 pixels centered on pos.  setPixelColor() will clip any
  // pixels off the ends of the strip, we don't need to watch for that.
  ring.setPixelColor(pos - 2, 0x100000); // Dark red
  ring.setPixelColor(pos - 1, 0x800000); // Medium red
  ring.setPixelColor(pos    , 0xFF3000); // Center pixel is brightest
  ring.setPixelColor(pos + 1, 0x800000); // Medium red
  ring.setPixelColor(pos + 2, 0x100000); // Dark red

  ring.show();
  delay(50);

  // Rather than being sneaky and erasing just the tail pixel,
  // it's easier to erase it all and draw a new one next time.
  for(j=-2; j<= 2; j++) ring.setPixelColor(pos+j, 0);

  // Bounce off ends of strip
  pos += dir;
  if(pos < 0) {
    pos = 1;
    dir = -dir;
  } else if(pos >= ring.numPixels()) {
    pos = ring.numPixels() - 2;
    dir = -dir;
  }
}

void LarsonScanStrip() {
  int j;

  // Draw 5 pixels centered on pos.  setPixelColor() will clip any
  // pixels off the ends of the strip, we don't need to watch for that.
  strip.setPixelColor(pos - 2, 0x100000); // Dark red
  strip.setPixelColor(pos - 1, 0x800000); // Medium red
  strip.setPixelColor(pos    , 0xFF3000); // Center pixel is brightest
  strip.setPixelColor(pos + 1, 0x800000); // Medium red
  strip.setPixelColor(pos + 2, 0x100000); // Dark red

  strip.show();
  delay(50);

  // Rather than being sneaky and erasing just the tail pixel,
  // it's easier to erase it all and draw a new one next time.
  for(j=-2; j<= 2; j++) strip.setPixelColor(pos+j, 0);

  // Bounce off ends of strip
  pos += dir;
  if(pos < 0) {
    pos = 1;
    dir = -dir;
  } else if(pos >= strip.numPixels()) {
    pos = strip.numPixels() - 2;
    dir = -dir;
  }
}

//Used to draw a line between two points of a given color
void drawLine(uint8_t from, uint8_t to, uint32_t c) {
  uint8_t fromTemp;
  if (from > to) {
    fromTemp = from;
    from = to;
    to = fromTemp;
  }
  for(int i=from; i<=to; i++){
    strip.setPixelColor(i, c);
  }
}
//void RandomSparcs_a()
//   {    
//    i = random(32);
//    c = random(4294967295);
//    ring.setPixelColor(i, c);
//    ring.show();
//    delay(10);
//    ring.setPixelColor(i, 0);
//   }
//    i = random(32);
//    ring.setPixelColor(i, color);
//    ring.show();
//    delay(10);
//    ring.setPixelColor(i, 0);
//    break;
//   )
   
//void RandomSparcs_b(uint8_t  i)
//  (
//    i = random(32);
//    strip.setPixelColor(i, color);
//    strip.show();
//    delay(10);
//    strip.setPixelColor(i, 0);
//    break;
//   )
float fscale( float originalMin, float originalMax, float newBegin, float
newEnd, float inputValue, float curve){

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  // condition curve parameter
  // limit range

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output 
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  /*
   Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution  
   Serial.println(); 
   */

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin){ 
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd; 
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine 
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0){
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {   
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange); 
  }

  return rangedValue;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel_a(byte Wheel_aPos) {
  if(Wheel_aPos < 85) {
   return ring.Color(Wheel_aPos * 3, 255 - Wheel_aPos * 3, 0);
  } else if(Wheel_aPos < 170) {
   Wheel_aPos -= 85;
   return ring.Color(255 - Wheel_aPos * 3, 0, Wheel_aPos * 3);
   } else {
   Wheel_aPos -= 170;
   return ring.Color(0, Wheel_aPos * 3, 255 - Wheel_aPos * 3);
  }
}

uint32_t Wheel_b(byte Wheel_bPos) {
  if(Wheel_bPos < 85) {
    return strip.Color(Wheel_bPos * 3, 255 - Wheel_bPos * 3, 0);
  } 
  else if(Wheel_bPos < 170) {
    Wheel_bPos -= 85;
    return strip.Color(255 - Wheel_bPos * 3, 0, Wheel_bPos * 3);
  } 
  else {
    Wheel_bPos -= 170;
    return strip.Color(0, Wheel_bPos * 3, 255 - Wheel_bPos * 3);
  }
}

