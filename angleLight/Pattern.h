#ifndef Pattern_h
#define Pattern_h

#include <stdint.h>
#include <Arduino.h>

class Pattern {
  public: 
    uint32_t colorStart;
    uint32_t colorEnd;
    int bufferStart;
    int bufferEnd;
    int delayAmount = 10;
    int width = 0;
    bool bounce;
    bool randomise;

    Pattern(int _pattern) {
      bufferStart = 0;
      bufferEnd = 0;
      bounce = true;
      delayAmount = 10;
      randomise = false;
      
      switch(_pattern) {
        case 1: // blue to pink bounce
          colorStart = Adafruit_NeoPixel::Color(0, 0, 255);
          colorEnd = Adafruit_NeoPixel::Color(255, 0, 0);    
          break;
        case 2: // golden orange
          colorStart = Adafruit_NeoPixel::Color(173, 72, 13);
          colorEnd = Adafruit_NeoPixel::Color(244, 217, 66);
          delayAmount = 20;
          bounce = false;
          width = 3;
          break;
        case 3: // white dots
          colorStart = Adafruit_NeoPixel::Color(0, 0, 0);
          colorEnd = Adafruit_NeoPixel::Color(255, 255, 255);
          bufferStart = 40;
          bufferEnd = 10;
          width = 5;
          break;
        case 4: // feeling blue
          colorStart = Adafruit_NeoPixel::Color(30, 30, 255);
          colorEnd = Adafruit_NeoPixel::Color(100, 30, 205);
          break;
        case 5: // randomised (start color does not matter, end color is what they change to before new random is picked)
          colorStart = Adafruit_NeoPixel::Color(0, 0, 0);
          colorEnd = Adafruit_NeoPixel::Color(0, 0, 0);
          randomise = true;
          delayAmount = 1;
          bufferStart = 2000;
          break;
        case 6: // all off, used to allow button to turn off display
          colorStart = Adafruit_NeoPixel::Color(0, 0, 0);
          colorEnd = Adafruit_NeoPixel::Color(0, 0, 0);
          bufferStart = 200;
          delayAmount = 10;
          width = 4;
          break;
      }
    }
};

#endif
