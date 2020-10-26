#include <Arduino.h>

#include <Adafruit_NeoPixel.h>

#define NUMPIXELS   16
#define LED_PIN   8
#define RED_PIN   A0
#define GREEN_PIN A1
#define BLUE_PIN  A2

byte redVal, greenVal, blueVal;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);

  strip.begin();
  strip.setBrightness(100);
  strip.show();
}

void loop() {
  strip.clear();
  redVal = map(analogRead(RED_PIN), 0, 1024, 0, 255);
  greenVal = map(analogRead(GREEN_PIN), 0, 1024, 0, 255);
  blueVal = map(analogRead(BLUE_PIN), 0, 1024, 0, 255);

  strip.fill(strip.Color(redVal, greenVal, blueVal), 0, NUMPIXELS);
  //strip.fill(strip.Color(redVal, greenVal, blueVal), 0, 1);
  //strip.fill(strip.Color(255, 0, 0), 3, 1);
  //strip.fill(strip.Color(0, 255, 0), 4, 1);
  //strip.fill(strip.Color(0, 0, 255), 5, 1);

  strip.show();
}
