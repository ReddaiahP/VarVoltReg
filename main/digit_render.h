#ifndef DIGIT_RENDER_H
#define DIGIT_RENDER_H

#include <stdint.h>

// --------------------------------------------------
// Global spacing control
// --------------------------------------------------
#define DIGIT_CELL_WIDTH          15
#define DIGIT_CELL_HEIGHT         20

#define DIGIT_X_OVERLAP_PERCENT   10
#define DIGIT_Y_OVERLAP_PERCENT   0

// --------------------------------------------------
// Display functions
// --------------------------------------------------
void outputVoltage(float voltage);
void outputCurrent(float current);

void setVoltage(float voltage);
void setCurrent(float current);

void powerIn(float power);
void powerConsumption(float power);

#endif