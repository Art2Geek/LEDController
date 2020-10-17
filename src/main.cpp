#include <Arduino.h>

#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIXELSPIN   6
#define NUMPIXELS   16
#define CALIBRATIONTIME 20000

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELSPIN, NEO_GRB + NEO_KHZ800);

unsigned long pixelsInterval=50;  // the time we need to wait
unsigned long colorWipePreviousMillis=0;
unsigned long theaterChasePreviousMillis=0;
unsigned long theaterChaseRainbowPreviousMillis=0;
unsigned long rainbowPreviousMillis=0;
unsigned long rainbowCyclesPreviousMillis=0;

int theaterChaseQ = 0;
int theaterChaseRainbowQ = 0;
int theaterChaseRainbowCycles = 0;
int rainbowCycles = 0;
int rainbowCycleCycles = 0;

uint16_t currentPixel = 0;// what pixel are we operating on
String currentMode = "";


const int RECV_PIN = 3;
IRrecv irrecv(RECV_PIN);
decode_results results;
unsigned long key_value = 0;

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c){
  pixels.setPixelColor(currentPixel,c);
  pixels.show();
  currentPixel++;
  if(currentPixel == NUMPIXELS){
    currentPixel = 0;
  }
}

void rainbow() {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, Wheel((i+rainbowCycles) & 255));
  }
  pixels.show();
  rainbowCycles++;
  if(rainbowCycles >= 256) rainbowCycles = 0;
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i;

  //for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + rainbowCycleCycles) & 255));
    }
    pixels.show();

  rainbowCycleCycles++;
  if(rainbowCycleCycles >= 256*5) rainbowCycleCycles = 0;
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow() {
  for (int i=0; i < pixels.numPixels(); i=i+3) {
    pixels.setPixelColor(i+theaterChaseRainbowQ, Wheel( (i+theaterChaseRainbowCycles) % 255));    //turn every third pixel on
  }

  pixels.show();
  for (int i=0; i < pixels.numPixels(); i=i+3) {
    pixels.setPixelColor(i+theaterChaseRainbowQ, 0);        //turn every third pixel off
  }
  theaterChaseRainbowQ++;
  theaterChaseRainbowCycles++;
  if(theaterChaseRainbowQ >= 3) theaterChaseRainbowQ = 0;
  if(theaterChaseRainbowCycles >= 256) theaterChaseRainbowCycles = 0;
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c) {
  for (int i=0; i < pixels.numPixels(); i=i+3) {
      pixels.setPixelColor(i+theaterChaseQ, c);    //turn every third pixel on
    }
    pixels.show();
    for (int i=0; i < pixels.numPixels(); i=i+3) {
      pixels.setPixelColor(i+theaterChaseQ, 0);        //turn every third pixel off
    }
    theaterChaseQ++;
    if(theaterChaseQ >= 3) theaterChaseQ = 0;
}




void setup(){
  Serial.begin(9600);
  irrecv.enableIRIn();
  irrecv.blink13(true);

  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  currentPixel = 0;

  pixels.begin();
  pixels.setBrightness(50);
  pixels.show(); // Initialize all pixels to 'off'

}

void loop(){


  if (currentMode == "wipe") {
    if ((unsigned long)(millis() - colorWipePreviousMillis) >= pixelsInterval) {
      colorWipePreviousMillis = millis();
      colorWipe(pixels.Color(0,255,125));
    }
  }
  else if (currentMode == "theaterChase") {
    if ((unsigned long)(millis() - theaterChasePreviousMillis) >= pixelsInterval) {
      theaterChasePreviousMillis = millis();
      theaterChase(pixels.Color(127, 127, 127)); // White
    }
  }
  else if (currentMode == "theaterChaseRaibow") {
    if ((unsigned long)(millis() - theaterChaseRainbowPreviousMillis) >= pixelsInterval) {
      theaterChaseRainbowPreviousMillis = millis();
      theaterChaseRainbow();
    }
  }
  else if (currentMode == "rainbow") {
    if ((unsigned long)(millis() - rainbowPreviousMillis) >= pixelsInterval) {
      rainbowPreviousMillis = millis();
      rainbow();
    }
  }
  else if (currentMode == "rainbowCycles") {
    if ((unsigned long)(millis() - rainbowCyclesPreviousMillis) >= pixelsInterval) {
        rainbowCyclesPreviousMillis = millis();
        rainbowCycle();
    }
  }

  if (irrecv.decode(&results)){

        if (results.value == 0XFFFFFFFF) {
          results.value = key_value;
        }

        switch(results.value){
          case 0xFFA25D:
            Serial.println("1");
            pixels.clear();
            currentMode = "wipe";
          break;
          case 0xFF629D:
            Serial.println("2");
            pixels.clear();
            currentMode = "theaterChase";
          break;
          case 0xFFE21D:
            Serial.println("3");
            pixels.clear();
            currentMode = "theaterChaseRaibow";
          break;
          case 0xFF22DD:
            Serial.println("4");
            currentMode = "rainbow";
          break;
          case 0xFF02FD:
            Serial.println("5");
            currentMode = "rainbowCycles";
          break ;
          case 0xFFC23D:
          Serial.println("6");
          break ;
          case 0xFFE01F:
          Serial.println("7");
          break ;
          case 0xFFA857:
          Serial.println("8");
          break ;
          case 0xFF906F:
          Serial.println("9");
          break ;
          case 0xFF6897:
          Serial.println("*");
          break ;
          case 0xFF9867:
          Serial.println("0");
          break ;
          case 0xFFB04F:
          Serial.println("#");
          break ;
          case 0xFF18E7:
          Serial.println("UP");
          break ;
          case 0xFF10EF:
          Serial.println("LEFT");
          break ;
          case 0xFF38C7:
          Serial.println("OK");
          break ;
          case 0xFF5AA5:
          Serial.println("RIGHT");
          break ;
          case 0xFF4AB5:
          Serial.println("DOWN");
          break ;
        }
        key_value = results.value;
        irrecv.resume();
  }

  delay(500);
}
