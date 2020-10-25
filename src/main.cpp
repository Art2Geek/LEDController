#include <Arduino.h>

#include <IRremote.h>

const int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;
unsigned long key_value = 0;

#include <Adafruit_NeoPixel.h>
#define PIXELSPIN   5
#define NUMPIXELS   144
#define PIXELS_BY_ROW 48

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELSPIN, NEO_GRB + NEO_KHZ800);

uint32_t colorRow1;
uint32_t colorRow2;
uint32_t colorRow3;

int selectedRow = -1;
int brightness = 50;

bool isRowSelected = false;
int lastRowSelectionMilliss = 0;

int currentMode = 0;

int lastStroboscopeStateChangedMilliss = 0;
bool isStroboscopeOn = false;

int lastSnakeMilliss = 0;
int snakeCurrentPixel = 0;
bool isLightingUp = true;

int rainbowCycleCycles = 0;
unsigned long rainbowCyclesPreviousMillis=0;
unsigned long pixelsInterval=50;  // the time we need to wait

void setup(){
  Serial.begin(115200);

  irrecv.enableIRIn();
  irrecv.blink13(true);

  colorRow1 = pixels.Color(255, 255, 255);
  colorRow2 = pixels.Color(255, 255, 255);
  colorRow3 = pixels.Color(255, 255, 255);

  pixels.begin();
  pixels.setBrightness(brightness);
  pixels.show(); // Initialize all pixels to 'off'
}

void setRowColor(int row, uint32_t color) {
  if (row == 0) {
    colorRow1 = color;
  } else if (row == 1) {
    colorRow2 = color;
  } else if (row == 2) {
    colorRow3 = color;
  } else {
    colorRow1 = colorRow2 = colorRow3 = color;
  }
}

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

void loop() {
  int currentMillis = millis();

  // Couleurs uniformes
  if (currentMode == 0) {
    if (isRowSelected && currentMillis - lastRowSelectionMilliss > 15000) {
      isRowSelected = false;
      selectedRow = -1;
    }

    pixels.fill(colorRow1, 0, PIXELS_BY_ROW);
    pixels.fill(colorRow2, PIXELS_BY_ROW, PIXELS_BY_ROW*2);
    pixels.fill(colorRow3, PIXELS_BY_ROW*2, NUMPIXELS);
    pixels.show();
  }
  // Arc-en-ciel
  else if (currentMode == 1) {
    if ((unsigned long)(millis() - rainbowCyclesPreviousMillis) >= pixelsInterval) {
        rainbowCyclesPreviousMillis = millis();
        rainbowCycle();
    }
  }
  // Barre de progression
  else if (currentMode == 2) {
    if (currentMillis - lastSnakeMilliss >= 10) {
      lastSnakeMilliss = millis();

      int pixel = snakeCurrentPixel >= PIXELS_BY_ROW && snakeCurrentPixel < PIXELS_BY_ROW*2 ? PIXELS_BY_ROW + (PIXELS_BY_ROW*2-1 - snakeCurrentPixel) : snakeCurrentPixel;

      // Allumer au fur et à mesure
      if (isLightingUp) {
        uint32_t color;
        if (pixel < PIXELS_BY_ROW) {
          color = colorRow1;
        } else if (pixel >= PIXELS_BY_ROW && pixel < PIXELS_BY_ROW*2) {
          color = colorRow2;
        } else {
          color = colorRow3;
        }

        pixels.setPixelColor(pixel, color);
        pixels.show();

        if (pixel >= NUMPIXELS-1) {
          isLightingUp = false;
        }

        snakeCurrentPixel++;
      }
      // Éteindre au fur et à mesure
      else {
        pixels.setPixelColor(pixel, pixels.Color(0, 0, 0));
        pixels.show();

        if (pixel <= 0) {
          isLightingUp = true;
        }

        snakeCurrentPixel--;
      }
    }
  }
  // Stroboscope
  else if (currentMode == 3) {
    if (currentMillis - lastStroboscopeStateChangedMilliss >= 50) {
      lastStroboscopeStateChangedMilliss = millis();

      if (!isStroboscopeOn) {
        pixels.fill(colorRow1, 0, PIXELS_BY_ROW);
        pixels.fill(colorRow2, PIXELS_BY_ROW, PIXELS_BY_ROW*2);
        pixels.fill(colorRow3, PIXELS_BY_ROW*2, NUMPIXELS);
        pixels.show();
        isStroboscopeOn = true;
      } else {
        pixels.clear();
        pixels.show();
        isStroboscopeOn = false;
      }
    }
  }

  // On reçoit un signal de la télécommande
  if (irrecv.decode(&results)) {
    Serial.println(results.value); // Afficher la valeur reçue

    if (results.value == 0XFFFFFFFF) {
      results.value = key_value;
    }

    switch(results.value){
      case 0xFFA25D:
        Serial.println("1");
        selectedRow = 2; // Étagère du haut
        isRowSelected = true;
        lastRowSelectionMilliss = millis();
      break;

      case 0xFF629D:
        Serial.println("2");
        selectedRow = 1; // Étagère du millieu
        isRowSelected = true;
        lastRowSelectionMilliss = millis();
      break;

      case 0xFFE21D:
        Serial.println("3");
        selectedRow = 0; // Étagère du bas
        isRowSelected = true;
        lastRowSelectionMilliss = millis();
      break;

      case 0xFF22DD:
        Serial.println("4");
        setRowColor(selectedRow, pixels.Color(255, 0, 0)); // Rouge
      break;

      case 0xFF02FD:
        Serial.println("5");
        setRowColor(selectedRow, pixels.Color(0, 255, 0)); // Vert
      break ;

      case 0xFFC23D:
        setRowColor(selectedRow, pixels.Color(0, 0, 255)); // Bleu
      break ;

      case 0xFFE01F:
        Serial.println("7");
        setRowColor(selectedRow, pixels.Color(255, 0, 191)); // Rose
      break ;

      case 0xFFA857:
        Serial.println("8");
        setRowColor(selectedRow, pixels.Color(0, 255, 233)); // Turquoise
      break ;

      case 0xFF906F:
        Serial.println("9");
        setRowColor(selectedRow, pixels.Color(255, 140, 0)); // Orange
      break ;

      case 0xFF9867:
        Serial.println("0");
        setRowColor(selectedRow, pixels.Color(255, 255, 255)); // Blanc
      break ;

      case 0xFF6897:
        Serial.println("*");
        setRowColor(selectedRow, pixels.Color(0, 0, 0)); // Noir
      break ;

      case 0xFFB04F:
        Serial.println("#");
        selectedRow = -1; // Toutes les étagères
      break ;

      case 0xFF18E7:
        Serial.println("UP");

        // Augmenter la luminosité
        if (brightness == 0) {
          brightness = 1; // 1 %
        } else if (brightness == 1) {
          brightness = 10; // 10 %
        } else if (brightness == 10) {
          brightness = 25; // 25 %
        } else if (brightness == 25) {
          brightness = 33; // 33 %
        } else if (brightness == 33) {
          brightness = 50; // 50 %
        } else if (brightness == 50) {
          brightness = 75; // 75 %
        } else if (brightness == 75) {
          brightness = 100; // 100 %
        } else {
          brightness = 100;
        }

        pixels.setBrightness(brightness);

        Serial.print("Brightness: ");
        Serial.println(brightness);
      break ;

      case 0xFF4AB5:
        Serial.println("DOWN");

        // Réduire la luminosité
        if (brightness == 100) {
          brightness = 75; // 75 %
        } else if (brightness == 75) {
          brightness = 50; // 50 %
        } else if (brightness == 50) {
          brightness = 33; // 33 %
        } else if (brightness == 33) {
          brightness = 25; // 25 %
        } else if (brightness == 25) {
          brightness = 10; // 10 %
        } else if (brightness == 10) {
          brightness = 1; // 1 %
        } else if (brightness == 1) {
          brightness = 0; // 0 %
        } else {
          brightness = 0;
        }

        pixels.setBrightness(brightness);

        Serial.print("Brightness: ");
        Serial.println(brightness);
      break ;

      case 0xFF10EF:
        currentMode--; // Mode précédent
        if (currentMode < 0) {
          currentMode = 3;
        }
        Serial.print("Mode ");
        Serial.println(currentMode);

        pixels.clear();
        pixels.show();
        snakeCurrentPixel = 0;
        delay(500);
      break ;

      case 0xFF5AA5:
        currentMode++; // Mode suivant
        if (currentMode > 3) {
          currentMode = 0;
        }
        Serial.print("Mode ");
        Serial.println(currentMode);

        pixels.clear();
        pixels.show();
        snakeCurrentPixel = 0;
        delay(500);
      break ;

      case 0xFF38C7:
        Serial.println("Power");
        if (brightness > 0) {
          brightness = 0; // Tout éteindre
        } else {
          brightness = 50; // Tout allumer à 50%
          selectedRow = -1;
          colorRow1 = colorRow2 = colorRow3 = pixels.Color(255, 255, 255);
        }

        pixels.setBrightness(brightness);
        delay(1000);
      break ;

    }
    key_value = results.value;
    irrecv.resume();
  }

  delay(200);
}
