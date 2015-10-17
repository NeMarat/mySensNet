#include <Arduino.h>
#include <EEPROM.h>

void writeULong (unsigned long l, int address) {
  EEPROM.write(address + 0, *((byte*)&l + 0)); //byte #1 from long
  EEPROM.write(address + 1, *((byte*)&l + 1)); // byte #2
  EEPROM.write(address + 2, *((byte*)&l + 2)); // #3
  EEPROM.write(address + 3, *((byte*)&l + 3)); // #4
}

unsigned long readULong(int address) {
  unsigned long l = 0;
  *((byte*)&l + 0) = EEPROM.read(address + 0);
  *((byte*)&l + 1) = EEPROM.read(address + 1);
  *((byte*)&l + 2) = EEPROM.read(address + 2);
  *((byte*)&l + 3) = EEPROM.read(address + 3);
  return l;
}

