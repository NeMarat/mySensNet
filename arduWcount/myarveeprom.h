/*
Simple class, that impements round-robin algorithm to maximize 
avr eeprom life. It shoud be used only for frequently changed
data. It is not fust. But it is simple and easy to use. 
*/

#ifndef __MYARVEEPROM__
#define __MYARVEEPROM__

#include <avr/eeprom.h>
#include <avr/interrupt.h>

enum {P_VALID = 17, P_INVALID};

class MyEeprom {
    public:
      /*
      first_address - first real addres or eeprom area, available for 
                      routation 
      max_address   - last real addres or eeprom area, available for 
                      routation
      data_length   - how many bytes in user-data
      reload_pages  - shoud we try to find in eerom last saved page
      */
      MyEeprom (uint32_t first_address, uint32_t max_address, uint16_t data_length, bool reload_pages = false);
      MyEeprom (uint32_t first_address);
      /*
      relative_address - relative address in user-data area after first_addres, bytes
      */
      void writeByte(uint8_t b, uint16_t relative_address);
      uint8_t readByte(uint16_t relative_address);
      void writeUInt (uint16_t l, uint16_t relative_address);
      uint16_t readUInt (uint16_t relative_address);
      void writeULong (uint32_t l, uint16_t relative_address);
      uint32_t readULong (uint16_t relative_address);
      
      uint16_t getCurrentOffset();
      uint32_t getCurrentFirstAddress();

    private:
        uint16_t dataLength;
        uint32_t firstAddress;
        uint32_t maxAddress;
        uint16_t currentOffset;
        uint8_t servisInfo = 1;
        bool rotateEeprom;
        void write_byte(uint8_t b, uint16_t relative_address);
        void simple_write_byte(uint8_t b, uint16_t relative_address);
        void write_ulong(uint32_t l, uint16_t relative_address);
        void simple_write_ulong(uint32_t l, uint16_t relative_address);
        void write_uint(uint16_t l, uint16_t relative_address);
        void simple_write_uint(uint16_t l, uint16_t relative_address);
};

#endif // __MYARVEEPROM__
