#ifndef MAOPAPERANG_PRINTER_H
#define MAOPAPERANG_PRINTER_H

#include <Arduino.h>

void goFront(uint32_t steps, uint16_t wait);
void goFront1();
void clearAddTime();
void sendData(uint8_t *dataPointer);
void clearData();
uint32_t startPrint();
void startPrint(uint8_t stb);
void clearSTB();
void testPage(uint8_t stb);
void testSTB();

#endif
