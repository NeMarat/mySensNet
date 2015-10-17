#ifndef __RTC_SAVE_H__
#define __RTC_SAVE_H__

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>

void writeLong (unsigned long l, byte address, RTC_DS1307 * r) {
  r->writenvram(address + 0, *((byte*)&l + 0)); //byte #1 int
  r->writenvram(address + 1, *((byte*)&l + 1)); // byte #2
  r->writenvram(address + 2, *((byte*)&l + 2)); // byte #3
  r->writenvram(address + 3, *((byte*)&l + 3)); // byte #4
}

void writeFloat (float l, byte address, RTC_DS1307 * r) {
  r->writenvram(address + 0, *((byte*)&l + 0)); //byte #1 int
  r->writenvram(address + 1, *((byte*)&l + 1)); // byte #2
  r->writenvram(address + 2, *((byte*)&l + 2)); // byte #3
  r->writenvram(address + 3, *((byte*)&l + 3)); // byte #4
}

void writeInt (int l, byte address, RTC_DS1307 * r) {
  r->writenvram(address + 0, *((byte*)&l + 0)); //byte #1 int
  r->writenvram(address + 1, *((byte*)&l + 1)); // byte #2
}

unsigned long readLong(byte address, RTC_DS1307 * r) {
  unsigned long l = 0;
  *((byte*)&l + 0) = r->readnvram(address + 0);
  *((byte*)&l + 1) = r->readnvram(address + 1);
  *((byte*)&l + 2) = r->readnvram(address + 2);
  *((byte*)&l + 3) = r->readnvram(address + 3);
  return l;
}

float readFloat(byte address, RTC_DS1307 * r) {
  float l = 0;
  *((byte*)&l + 0) = r->readnvram(address + 0);
  *((byte*)&l + 1) = r->readnvram(address + 1);
  *((byte*)&l + 2) = r->readnvram(address + 2);
  *((byte*)&l + 3) = r->readnvram(address + 3);
  return l;
}

int readInt(byte address, RTC_DS1307 * r) {
  int l = 0;
  *((byte*)&l + 0) = r->readnvram(address + 0);
  *((byte*)&l + 1) = r->readnvram(address + 1);
  return l;
}

#endif

