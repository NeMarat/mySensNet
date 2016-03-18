#ifndef _WEATHER_OREGON_H_
#define _WEATHER_OREGON_H_

volatile uint16_t pulse;

class DecodeOOK 
{
   protected:
      uint8_t total_bits, bits, flip, state, pos, data[25];
      virtual uint8_t decode(word width) = 0;
   public:

enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };

DecodeOOK () { resetDecoder(); }

bool nextPulse(uint16_t width) 
      {
         if (state != DONE)
            switch (decode(width)) 
            {
               case -1: resetDecoder(); break;
               case 1:  done(); break;
            }
         return isDone();
      }

bool isDone () const { return state == DONE; }

uint8_t* getData (uint8_t& count) {
         count = pos;
         return data;
      }

void resetDecoder() 
      {
         total_bits = bits = pos = flip = 0;
         state = UNKNOWN;
      }

      // add one bit to the packet data buffer
virtual void gotBit (uint8_t value) 
      {
         total_bits++;
         uint8_t *ptr = data + pos;
         *ptr = (*ptr >> 1) | (value << 7);

         if (++bits >= 8) 
         {
            bits = 0;
            if (++pos >= sizeof data) 
            {
               resetDecoder();
               return;
            }
         }
         state = OK;
      }

      // store a bit using Manchester encoding
void manchester(uint8_t value) 
      {
         flip ^= value; // manchester code, long pulse flips the bit
         gotBit(flip);
      }

      // move bits to the front so that all the bits are aligned to the end
void alignTail(uint8_t max =0) 
      {
         // align bits
         if (bits != 0) 
         {
            data[pos] >>= 8 - bits;
            for (uint8_t i = 0; i < pos; ++i)
               data[i] = (data[i] >> bits) | (data[i+1] << (8 - bits));
            bits = 0;
         }

         // optionally shift bytes down if there are too many of 'em
         if (max > 0 && pos > max) 
         {
            uint8_t n = pos - max;
            pos = max;
            for (uint8_t i = 0; i < pos; ++i)
               data[i] = data[i+n];
         }
      }

void reverseBits() 
      {
         for (uint8_t i = 0; i < pos; ++i) 
         {
            uint8_t b = data[i];
            for (uint8_t j = 0; j < 8; ++j) 
            {
               data[i] = (data[i] << 1) | (b & 1);
               b >>= 1;
            }
         }
      }

void reverseNibbles() 
      {
         for (uint8_t i = 0; i < pos; ++i)
            data[i] = (data[i] << 4) | (data[i] >> 4);
      }

void done() 
      {
         while (bits)
            gotBit(0); // padding
         state = DONE;
      }
};

class OregonDecoderV2 : public DecodeOOK 
{
   public:  
  OregonDecoderV2() {}

      // add one bit to the packet data buffer
  virtual void gotBit (uint8_t value) 
      {
         if(!(total_bits & 0x01))
         {
            data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
         }

         total_bits++;
         pos = total_bits >> 4;

         if (pos >= sizeof data) 
         {
            resetDecoder();
            return;
         }
         state = OK;
      }

  virtual uint8_t decode(word width) 
      {
         if (200 <= width && width < 1200) 
         {
            byte w = width >= 700;

            switch (state) 
            {
               case UNKNOWN:
                  if (w != 0) 
                  {
                     // Long pulse
                     ++flip;
                  } 
                  else if (w == 0 && 24 <= flip) 
                  {
                     // Short pulse, start bit
                     flip = 0;
                     state = T0;
                  } 
                  else 
                  {
                     // Reset decoder
                     return -1;
                  }
                  break;
               case OK:
                  if (w == 0) 
                  {
                     // Short pulse
                     state = T0;
                  }
                  else 
                  {
                     // Long pulse
                     manchester(1);
                  }
                  break;
               case T0:
                  if (w == 0) 
                  {
                     // Second short pulse
                     manchester(0);
                  } 
                  else 
                  {
                     // Reset decoder
                     return -1;
                  }
               break;
            }
         } 
         else if (width >= 2500  && pos >= 8) 
         {
            return 1;
         } 
         else 
         {
            return -1;
         }
         return 0;
      }
};

////////////////////////////////////

void oregonrd(void)
{
   static uint16_t last;
   pulse = micros() - last;
   last += pulse;
}

float temperature(uint8_t * data)
{
   int8_t sign = (data[6]&0x8) ? -1 : 1;
   float temp = ((data[5]&0xF0) >> 4)*10 + (data[5]&0xF) + (((data[4]&0xF0) >> 4) / 10.0);
   return (sign * temp);
}

uint8_t humidity(uint8_t * data)
{
   return (data[7]&0xF) * 10 + ((data[6]&0xF0) >> 4);
}

uint8_t battery(uint8_t* data)
{
   return (data[4] & 0x4) ? 10 : 90;
}

uint8_t channel(uint8_t* data)
{
   uint8_t channel;
   switch (data[2])
   {
      case 0x10:
         channel = 1;
         break;
      case 0x20:
         channel = 2;
         break;
      case 0x40:
         channel = 3;
         break;
   }
 return channel;
} 

#endif
