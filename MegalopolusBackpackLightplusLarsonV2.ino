#include <Boards.h>
#include <Firmata.h>

#include <Adafruit_NeoPixel.h>
#include <math.h>
boolean usingInterrupt = false;
#define N_PIXELSA  16  // Number of pixels in strand
#define N_PIXELSB  12  // Number of pixels in strand
#define MIC_PIN   9  // Microphone is attached to this analog pin
#define LED_PINA    6  // NeoPixel LED strand is connected to this pin
#define LED_PINB    12  
#define DC_OFFSET  0  // DC offset in mic signal - if unusure, leave 0
#define NOISE     10  // Noise/hum/interference in mic signal
#define SAMPLES   60  // Length of buffer for dynamic level adjustment
#define TOP       (N_PIXELSB + 2) // Allow dot to go slightly off scale
#define PEAK_FALL 40  // Rate of peak falling dot


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
Adafruit_NeoPixel strip_a = Adafruit_NeoPixel(N_PIXELSA, LED_PINA, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_b = Adafruit_NeoPixel(N_PIXELSB, LED_PINB, NEO_GRB + NEO_KHZ800);

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
  strip_a.begin();
  strip_a.show(); // Initialize all pixels to 'off'
  strip_b.begin();
  strip_b.show();
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
  strip_a.begin();
  strip_a.show(); // Initialize all pixels to 'off'
  strip_b.begin();
  strip_b.show();
  LarsonScan_a(); //Larson Scan Circle
  }
  
  if (mode == 1) {
  strip_a.begin();
  strip_a.show(); // Initialize all pixels to 'off'
  strip_b.begin();
  strip_b.show();
  LarsonScan_b(); // Larson Scan Side
  }
  
  if (mode == 2) {
  strip_a.begin();
  strip_a.show(); // Initialize all pixels to 'off'
  strip_b.begin();
  strip_b.show();
  flashRandom(5, 3);
  flashRandom_b(5, 3);
//  LarsonScan_b();  
}
  
 if (mode == 3) {
  strip_a.begin();
  strip_a.show(); // Initialize all pixels to 'off'
  strip_b.begin();
  strip_b.show();
  rainbow(20);
  }
 if (mode == 4) {
  strip_a.begin();
  strip_a.show(); // Initialize all pixels to 'off'
  strip_b.begin();
  strip_b.show();
//  LarsonScan_a();
  amplitie();
  }
} 
void flashRandom(int wait, uint8_t howmany) {
 
  for(uint16_t i=0; i<howmany; i++) {
    // pick a random favorite color!
    int c = random(FAVCOLORS);
    int red = myColors[c][0];
    int green = myColors[c][1];
    int blue = myColors[c][2]; 
 
    // get a random pixel from the list
    int j = random(strip_a.numPixels());
    
    // now we will 'fade' it in 5 steps
    for (int x=0; x < 5; x++) {
      int r = red * (x+1); r /= 5;
      int g = green * (x+1); g /= 5;
      int b = blue * (x+1); b /= 5;
      
      strip_a.setPixelColor(j, strip_a.Color(r, g, b));
      strip_a.show();
      delay(wait);
    }
    // & fade out in 5 steps
    for (int x=5; x >= 0; x--) {
      int r = red * x; r /= 5;
      int g = green * x; g /= 5;
      int b = blue * x; b /= 5;
      
      strip_a.setPixelColor(j, strip_a.Color(r, g, b));
      strip_a.show();
      delay(wait);
    }
  }
  // LEDs will be off when done (they are faded to 0)
}
void flashRandom_b(int wait, uint8_t howmany) {
 
  for(uint16_t i=0; i<howmany; i++) {
    // pick a random favorite color!
    int c = random(FAVCOLORS);
    int red = myColors[c][0];
    int green = myColors[c][1];
    int blue = myColors[c][2]; 
 
    // get a random pixel from the list
    int j = random(strip_b.numPixels());
    
    // now we will 'fade' it in 5 steps
    for (int x=0; x < 5; x++) {
      int r = red * (x+1); r /= 5;
      int g = green * (x+1); g /= 5;
      int b = blue * (x+1); b /= 5;
      
      strip_b.setPixelColor(j, strip_b.Color(r, g, b));
      strip_b.show();
      delay(wait);
    }
    // & fade out in 5 steps
    for (int x=5; x >= 0; x--) {
      int r = red * x; r /= 5;
      int g = green * x; g /= 5;
      int b = blue * x; b /= 5;
      
      strip_b.setPixelColor(j, strip_b.Color(r, g, b));
      strip_b.show();
      delay(wait);
    }
  }
  // LEDs will be off when done (they are faded to 0)
}
// Fill the strip at once with one color
void SetFullStrip(uint32_t c) {
  for(uint16_t i=0; i<strip_a.numPixels(); i++) {
      strip_a.setPixelColor(i, c);
      strip_a.setBrightness(100);
      strip_a.show();
      }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip_a.numPixels(); i++) {
      strip_a.setBrightness(100);
      strip_a.setPixelColor(i, Wheel_a((i+j) & 255));
      strip_b.setBrightness(100);
      strip_b.setPixelColor(i, Wheel_a((i+j) & 255));
    }
    strip_a.show();
    strip_b.show();
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
        SetFullStrip(strip_a.Color(0, 0, 0));
        buttonHoldTime = millis();
      } else {
        mode = mode + 1;
        SetFullStrip(strip_a.Color(0, 0, 0));
        buttonHoldTime = millis();
      }
    }
  }
}

void amplitie()
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


  // Color pixels based on rainbow gradient
  for(i=0; i<N_PIXELSB; i++) {
    if(i >= height)               strip_b.setPixelColor(i,   0,   0, 0);
    else strip_b.setPixelColor(i,Wheel_b(map(i,0,strip_b.numPixels()-1,30,150)));
    
  }



  // Draw peak dot  
  if(peak > 0 && peak <= N_PIXELSB-1) strip_b.setPixelColor(peak,Wheel_b(map(peak,0,strip_b.numPixels()-1,30,150)));
  
   strip_b.show(); // Update strip_b

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

void LarsonScan_a() {
  int j;

  // Draw 5 pixels centered on pos.  setPixelColor() will clip any
  // pixels off the ends of the strip, we don't need to watch for that.
  strip_a.setPixelColor(pos - 2, 0x100000); // Dark red
  strip_a.setPixelColor(pos - 1, 0x800000); // Medium red
  strip_a.setPixelColor(pos    , 0xFF3000); // Center pixel is brightest
  strip_a.setPixelColor(pos + 1, 0x800000); // Medium red
  strip_a.setPixelColor(pos + 2, 0x100000); // Dark red

  strip_a.show();
  delay(50);

  // Rather than being sneaky and erasing just the tail pixel,
  // it's easier to erase it all and draw a new one next time.
  for(j=-2; j<= 2; j++) strip_a.setPixelColor(pos+j, 0);

  // Bounce off ends of strip
  pos += dir;
  if(pos < 0) {
    pos = 1;
    dir = -dir;
  } else if(pos >= strip_a.numPixels()) {
    pos = strip_a.numPixels() - 2;
    dir = -dir;
  }
}

void LarsonScan_b() {
  int j;

  // Draw 5 pixels centered on pos.  setPixelColor() will clip any
  // pixels off the ends of the strip, we don't need to watch for that.
  strip_b.setPixelColor(pos - 2, 0x100000); // Dark red
  strip_b.setPixelColor(pos - 1, 0x800000); // Medium red
  strip_b.setPixelColor(pos    , 0xFF3000); // Center pixel is brightest
  strip_b.setPixelColor(pos + 1, 0x800000); // Medium red
  strip_b.setPixelColor(pos + 2, 0x100000); // Dark red

  strip_b.show();
  delay(50);

  // Rather than being sneaky and erasing just the tail pixel,
  // it's easier to erase it all and draw a new one next time.
  for(j=-2; j<= 2; j++) strip_b.setPixelColor(pos+j, 0);

  // Bounce off ends of strip
  pos += dir;
  if(pos < 0) {
    pos = 1;
    dir = -dir;
  } else if(pos >= strip_b.numPixels()) {
    pos = strip_b.numPixels() - 2;
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
    strip_b.setPixelColor(i, c);
  }
}
//void RandomSparcs_a()
//   {    
//    i = random(32);
//    c = random(4294967295);
//    strip_a.setPixelColor(i, c);
//    strip_a.show();
//    delay(10);
//    strip_a.setPixelColor(i, 0);
//   }
//    i = random(32);
//    strip_a.setPixelColor(i, color);
//    strip_a.show();
//    delay(10);
//    strip_a.setPixelColor(i, 0);
//    break;
//   )
   
//void RandomSparcs_b(uint8_t  i)
//  (
//    i = random(32);
//    strip_b.setPixelColor(i, color);
//    strip_b.show();
//    delay(10);
//    strip_b.setPixelColor(i, 0);
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
   return strip_a.Color(Wheel_aPos * 3, 255 - Wheel_aPos * 3, 0);
  } else if(Wheel_aPos < 170) {
   Wheel_aPos -= 85;
   return strip_a.Color(255 - Wheel_aPos * 3, 0, Wheel_aPos * 3);
   } else {
   Wheel_aPos -= 170;
   return strip_a.Color(0, Wheel_aPos * 3, 255 - Wheel_aPos * 3);
  }
}

uint32_t Wheel_b(byte Wheel_bPos) {
  if(Wheel_bPos < 85) {
    return strip_b.Color(Wheel_bPos * 3, 255 - Wheel_bPos * 3, 0);
  } 
  else if(Wheel_bPos < 170) {
    Wheel_bPos -= 85;
    return strip_b.Color(255 - Wheel_bPos * 3, 0, Wheel_bPos * 3);
  } 
  else {
    Wheel_bPos -= 170;
    return strip_b.Color(0, Wheel_bPos * 3, 255 - Wheel_bPos * 3);
  }
}

