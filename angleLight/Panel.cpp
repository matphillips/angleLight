#include "Panel.h"
#include <Arduino.h>

void Panel::advance() {
  if (currentlyForwards) {
    if (currentStep > 255 + bufferEnd) {
      if (bounce) {
        currentlyForwards = false;
        currentStep --;
      } else {
        currentStep = 0 - bufferStart;
      }
    } else {
      currentStep ++;
    }
  } else {
    if (currentStep < 0 - bufferStart) {
      if (bounce) {
        currentlyForwards = true;
        currentStep ++;
      } else {
        currentStep = 255 + bufferEnd;
      }
    } else {
      currentStep --;
    }
  }
  
  if (randomise && currentStep == 255 + bufferEnd) {
    colorStart = (random(256) << 16 | random(256) << 8 | random(256));
  }
}

uint32_t Panel::getNewColor() {
  byte startRed = (colorStart >> 16);
  byte startBlue = colorStart & 0xff;
  byte startGreen = (colorStart >> 8) & 0xff;
  byte endRed = (colorEnd >> 16) & 0xff;
  byte endBlue = colorEnd & 0xff;
  byte endGreen = (colorEnd >> 8) & 0xff;
  int step = currentStep > 255 ? 255 : currentStep < 0 ? 0 : currentStep;
  byte red = map(step, 0, 255, startRed, endRed);
  byte blue = map(step, 0, 255, startBlue, endBlue);
  byte green = map(step, 0, 255, startGreen, endGreen);
  return red << 16 | green << 8 | blue;
}
