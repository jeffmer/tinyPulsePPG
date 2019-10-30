/*
*  SSD1306 driver - optimised for low memory 
*                   uses 1 128 byte page buffer
*
* j.n.magee 15-10-2019
*/
/*
 * Uses initialisation ideas from Ref.:
 * DigisparkOLED: https://github.com/digistump/DigistumpArduino/tree/master/digistump-avr/libraries/DigisparkOLED
 * SSD1306 data sheet: https://www.adafruit.com/datasheets/SSD1306.pdf
 */
 
#include <avr/pgmspace.h>
#include "Wire.h"
#include "ssd1306h.h"

/*
 * Software Configuration, data sheet page 64
 */

static const uint8_t ssd1306_configuration[] PROGMEM = {
#ifdef SCREEN_128X64
  0xA8, 0x3F,   // Set MUX Ratio, 0F-3F
#else // SCREEN_128X32 / SCREED_64X32
  0xA8, 0x1F,   // Set MUX Ratio, 0F-3F
#endif
  0xD3, 0x00,   // Set Display Offset
  0x40,         // Set Display Start line
  0xA1,         // Set Segment re-map, mirror, A0/A1
  0xC8,         // Set COM Output Scan Direction, flip, C0/C8
#ifdef SCREEN_128X32
  0xDA, 0x02,   // Set COM Pins hardware configuration, Sequential
#else // SCREEN_128X64 / SCREEN_64X32
  0xDA, 0x12,   // Set Com Pins hardware configuration, Alternative
#endif
  0x81, 0xFF,   // Set Contrast Control, 01-FF
  0xA4,         // Disable Entire Display On, 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
  0xA6,         // Set Display Mode. A6=Normal; A7=Inverse
  0xD5, 0x80,   // Set Osc Frequency
  0x8D, 0x14,   // Enable charge pump regulator
  0xAF          // Display ON in normal mode
};

SSD1306::SSD1306(void) {}

void SSD1306::init(void) {
  Wire.begin();
  Wire.setClock(400000);
  for (uint8_t i = 0; i < sizeof (ssd1306_configuration); i++) {
    command(pgm_read_byte_near(&ssd1306_configuration[i]));
  }
  currentPage = 0;
  for (uint8_t i=0; i<COLUMNS; ++i) pageBuf[i]=0x00;
}

void SSD1306::command_start(void) {
  Wire.beginTransmission(SSD1306_I2C_ADDR);
  Wire.write(0x00); //command
}

void SSD1306::command_stop(void) {
  Wire.endTransmission();
}

void SSD1306::command(uint8_t command) {
  command_start();
  Wire.write(command);
  command_stop();
}

void SSD1306::data_start(void) {
  Wire.beginTransmission(SSD1306_I2C_ADDR);
  Wire.write(0x40); //data
}

void SSD1306::data_stop(void) {
  Wire.endTransmission();
}

void SSD1306::data_byte(uint8_t data) {
  if (Wire.write(data) == 0) {
    // push data if detect buffer used up
    data_stop();
    data_start();
    Wire.write(data);
  }
}

void SSD1306::set_area(uint8_t col, uint8_t page, uint8_t col_range, uint8_t page_range) {
  command_start();
  Wire.write(0x20);
  Wire.write(0x01);
  Wire.write(0x21);
  Wire.write(col);
  Wire.write(col + col_range - 1);
  Wire.write(0x22);
  Wire.write(page);
  Wire.write(page + page_range - 1);
  command_stop();
}

void SSD1306::fill(uint8_t data) {
  set_area(0, 0, COLUMNS, PAGES);
  uint16_t size = (COLUMNS) * (PAGES);
  data_start();
  for (uint16_t i = 0; i <size; i++) {
    data_byte(data);
  }
  data_stop();
}

uint8_t currentPage = 0;
uint8_t pageBuf[COLUMNS];

void SSD1306::writePage(uint8_t page){
  set_area(0, page, COLUMNS, 1);
  data_start();
  for (uint8_t i = 0; i < COLUMNS; i++) {
    data_byte(pageBuf[i]);
  }
  data_stop();
}

bool SSD1306::inPage(uint8_t y,uint8_t h){
   return currentPage >= y/8 && currentPage <= (y+h-1)/8;
}

void SSD1306::off(void){
  command(0xAE);
}

void SSD1306::on(void){
  command(0xAF);
}

inline void SSD1306::drawPixel(uint8_t x, uint8_t y){
    if (y/8 != currentPage) return;
    pageBuf[x] |= 1<<(y%8);
}

void SSD1306::drawHLine(uint8_t x, uint8_t y, uint8_t w){
    if (y/8 != currentPage) return;
    for(uint8_t i=0; i<w; ++i) pageBuf[x+i] |= 1<<(y%8);
}

void SSD1306::drawVLine(uint8_t x, uint8_t y, uint8_t h){
    for (uint8_t i=0; i<h; ++i) drawPixel(x,y+i);
}


uint16_t Stretch (uint16_t x) {
  x = (x & 0xF0)<<4 | (x & 0x0F);
  x = (x<<2 | x) & 0x3333;
  x = (x<<1 | x) & 0x5555;
  return x | x<<1;
}

void SSD1306::drawChar(int x, int y,  unsigned char c, const int BIG = 1) {
  uint16_t data;
  for (uint8_t i = 0; i < FONT_WIDTH; i++) {
    data = pgm_read_byte_near(&font_bitmap[c - FONT_START ][i]);
    if (BIG==2) data = Stretch(data);
    for (uint8_t d = 0; d < BIG; d++) {
        for ( uint8_t j = 0; j<FONT_HEIGHT*BIG; j++) {
            if (data & (1<<j)) drawPixel(x+(i*BIG+d),y+j);
        }
    }
  }
}

void SSD1306::drawStr(int x, int y, const char *s, const int BIG = 1) {
    if (!inPage(y,FONT_HEIGHT*BIG)) return;
    for (int k=0; s[k]!='\0'; ++k) {
      drawChar(x,y,s[k],BIG);
      x += (FONT_WIDTH+1)*BIG;
    }
}

void SSD1306::drawStr(int x, int y, const __FlashStringHelper *s, const int BIG = 1) {
  if (!inPage(y,FONT_HEIGHT*BIG)) return;
  PGM_P p = reinterpret_cast<PGM_P>(s);
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    drawChar(x,y,c,BIG);
    x += (FONT_WIDTH+1)*BIG;
  }
}

//assumes that horizontal lines are padded into an integral number of bytes
void SSD1306::drawXBMP(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap){
    uint8_t data;
    if (!inPage(y,h)) return;
    uint8_t bytewidth = (w%8 == 0) ? w/8 : w/8+1;
    for (int j = 0; j<h; ++j){
        for (int i= 0; i<w; ++i) {
            uint8_t bitno = i%8;
            if (bitno == 0) data = pgm_read_byte_near(&bitmap[j*bytewidth+i/8]);
            if (data & (1<<bitno)) drawPixel(x+i,y+j);
        }
    }
}

void SSD1306::drawNibbles(uint8_t y, uint8_t nibbles[COLUMNS/2]){
    if (y/8 != currentPage) return;
    uint8_t high = y%8==4 ? 1 : 0;
    for (int i = 0; i<COLUMNS/2; i++) {
        if (high==0) {
            pageBuf[i*2]  |= nibbles[i] & 0x0F;
            pageBuf[i*2+1] |= (nibbles[i] & 0xF0)>>4;
        } else {
            pageBuf[i*2]  |= (nibbles[i] & 0x0F)<<4;
            pageBuf[i*2+1] |= nibbles[i] & 0xF0;
        }
    }
}
            
void SSD1306::firstPage(void){
    currentPage = 0;
}

bool SSD1306::nextPage(void){
    writePage(currentPage++);
    memset(pageBuf, 0x00, COLUMNS);
    return currentPage != PAGES;
}
