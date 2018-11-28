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
}

uint32_t Panel::getNewColor() {
  byte startRed = colorStart > 0 ? (colorStart >> 16) & 0xff : random(256);
  byte startBlue = colorStart > 0 ? colorStart & 0xff : random(256);
  byte startGreen = colorStart > 0 ? (colorStart >> 8) & 0xff : random(256);
  byte endRed = (colorEnd >> 16) & 0xff;
  byte endBlue = colorEnd & 0xff;
  byte endGreen = (colorEnd >> 8) & 0xff;
  int step = currentStep > 255 ? 255 : currentStep < 0 ? 0 : currentStep;
  byte red = map(step, 0, 255, startRed, endRed);
  byte blue = map(step, 0, 255, startBlue, endBlue);
  byte green = map(step, 0, 255, startGreen, endGreen);
  return red << 16 | green << 8 | blue;
}
