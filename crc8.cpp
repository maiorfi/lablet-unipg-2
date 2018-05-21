/*  
 * crc8.c
 * 
 * Computes a 8-bit CRC 
 * http://www.rajivchakravorty.com/source-code/uncertainty/multimedia-sim/html/crc8_8c-source.html
 */

#include <stdio.h>

// #define GP 0x107  // but it is never used
/* if Generating polynomial GP is 0x107  x^8 + x^2 + x + 1 then DI=0x07 */
/* if Generating polynomial GP is 0x131  x^8 + x^5 + x^4 + 1 then DI=0x31 */
// #define DI  0x07  // for MLX90614
#define DI 0x31 // for Si7021

static unsigned char crc8_table[256]; /* 8-bit table */
static int made_table = 0;

static void init_crc8()
/*
      * Should be called before any other crc function.  
      */
{
  int i, j;
  unsigned char crc;

  if (!made_table)
  {
    for (i = 0; i < 256; i++)
    {
      crc = i;
      for (j = 0; j < 8; j++)
        crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
      crc8_table[i] = crc & 0xFF;
      /* printf("table[%d] = %d (0x%X)\n", i, crc, crc); */
    }
    made_table = 1;
  }
}

void crc8(unsigned char *crc, unsigned char m)
/*
      * For a byte array whose accumulated crc value is stored in *crc, computes
      * resultant crc obtained by appending m to the byte array
      */
{
  if (!made_table)
    init_crc8();

  *crc = crc8_table[(*crc) ^ m];
  *crc &= 0xFF;
}
