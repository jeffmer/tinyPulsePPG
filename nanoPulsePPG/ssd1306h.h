/*
*  SSD1306 driver - optimised for low memory 
*                   uses 1 128 byte page buffer
*
* j.n.magee 15-10-2019
*/

#include "Wire.h"
#include "font.h"

// custom I2C address by define SSD1306_I2C_ADDR
#ifndef SSD1306_I2C_ADDR
  #define SSD1306_I2C_ADDR 0x3C
#endif

#define SCREEN_128X32
#define COLUMNS 0x0080
#define PAGES 0x04

class SSD1306 {

  private:
    uint8_t currentPage;
    uint8_t pageBuf[COLUMNS];

  public:
    SSD1306(void);
    void init(void);
    void command_start(void);
    void command_stop(void);
    void command(uint8_t command);
    void data_start(void);
    void data_stop(void);
    void data_byte(uint8_t byte);
    void set_area(uint8_t col, uint8_t page, uint8_t col_range, uint8_t page_range);
    void fill(uint8_t fill);
    void writePage(uint8_t page);
    bool inPage(uint8_t y,uint8_t h);
    void drawPixel(uint8_t x, uint8_t y);
    void drawHLine(uint8_t x, uint8_t y, uint8_t w);  
    void drawVLine(uint8_t x, uint8_t y, uint8_t h); 
    void drawXBMP(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap);
    void drawNibbles(uint8_t y, uint8_t nibbles[COLUMNS/2]);
    void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
    void drawChar(int x, int y,  unsigned char c, const int BIG = 1);
    void drawStr(int x, int y, const char *s, int BIG = 1);
    void drawStr(int x, int y, const __FlashStringHelper *s, const int BIG = 1);
    void firstPage(void);
    bool nextPage(void);
    void off();
    void on();
};
