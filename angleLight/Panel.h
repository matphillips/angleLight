#ifndef Panel_h
#define Panel_h

#include <stdint.h>
#include <Arduino.h>

class Panel {
  public: 
    void advance();
    uint32_t getNewColor();

    Panel() {}
    Panel(uint32_t _colorStart, uint32_t _colorEnd, int _bufferStart, int _bufferEnd, int _currentStep, bool _bounce, bool _randomise) {
      colorStart = _colorStart;
      colorEnd = _colorEnd;
      bufferStart = _bufferStart;
      bufferEnd = _bufferEnd;
      bounce = _bounce;
      currentStep = _currentStep;
      currentlyForwards = false;
      randomise = _randomise;
    }
    
  private:
    uint32_t colorStart;
    uint32_t colorEnd;
    int bufferStart;
    int bufferEnd;
    int currentStep;
    bool currentlyForwards;
    bool bounce;
    bool randomise;
    void colorChange();
};

#endif
