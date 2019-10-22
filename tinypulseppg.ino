/*
*  ATTiny84 - Pulse Oximeter with PPG display
*  DEFINITELY NOT FOR MEDICAL USE
*
* j.n.magee 15-10-2019
*/


#include <avr/sleep.h>
#include "ssd1306h.h"
#include "MAX30102.h"
#include "Pulse.h"
#include <avr/pgmspace.h>

SSD1306 oled; 
MAX30102 sensor;
Pulse pulseIR;
Pulse pulseRed;

#define LED 1

//spo2_table is approximated as  -45.060*ratioAverage* ratioAverage + 30.354 *ratioAverage + 94.845 ;
const uint8_t spo2_table[184] PROGMEM =
        { 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99, 
          99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
          100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97, 
          97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 
          90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 
          80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 
          66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 
          49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29, 
          28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5, 
          3, 2, 1 } ;

const byte RATE_SIZE = 8; //Increase this for more averaging. 4 is good.
uint16_t MA_rate = 60 * RATE_SIZE;     // Moving Average Total
long lastBeat = 0; //Time at which the last beat occurred
long beatsPerMinute;
int  beatAvg;
long RX100;
int  SPO2, SPO2f;
int  voltage;

const uint8_t MAXWAVE = 80;
uint8_t waveform[MAXWAVE];
uint8_t disp_wave[MAXWAVE];
uint8_t wavep = 0;


int getVCC() {
   return min(11264/analogRead(12),99); 
}

void print_digit(int x, int y, long val, char c=' ', uint8_t field = 3,
                    const int BIG = 2){  
    uint8_t ff = field;
    do { 
        char ch = (val!=0) ? val%10+'0': c;
        oled.drawChar( x+BIG*(ff-1)*6, y, ch, BIG);
        val = val/10; 
        --ff;
    } while (ff>0);
}

void scale_wave() {
  uint8_t maxw = 0;
  uint8_t minw = 255;
  for (int i=0; i<MAXWAVE; i++) { 
    maxw = waveform[i]>maxw?waveform[i]:maxw;
    minw = waveform[i]<minw?waveform[i]:minw;
  }
  uint8_t scale = (maxw-minw)/32 + 1;
  uint8_t index = wavep;
  uint8_t nexty = 31-(waveform[index]-minw)/scale;
  for (int i=0; i<MAXWAVE; i++) {
    disp_wave[i] = 31-(waveform[index]-minw)/scale;
    index = (index + 1) % MAXWAVE;
  }
}

void draw_wave() {
   for (int i=0; i<MAXWAVE; i++) {
    uint8_t y = disp_wave[i];
    oled.drawPixel(i, y);
    if (i<MAXWAVE-1) {
      uint8_t nexty = disp_wave[i+1];
      if (nexty>y) {
        for (uint8_t iy = y+1; iy<nexty; ++iy)  oled.drawPixel(i, iy);
      } else if (nexty<y) {
        for (uint8_t iy = nexty+1; iy<y; ++iy)  oled.drawPixel(i, iy);
      }
    }
  }  
}

void draw_oled(int msg) {
    oled.firstPage();
    do{
    switch(msg){
        case 0:  oled.drawStr(10,0,F("Device error"),1); 
                 break;
        case 1:  oled.drawStr(10,12,F("Place Finger"),1); 
                 oled.drawChar(100,0,voltage/10+'0');
                 oled.drawChar(106,0,'.');
                 oled.drawChar(112,0,voltage%10+'0');
                 oled.drawChar(118,0,'V');
                 break;
        case 2:  print_digit(86,0,beatAvg);
                 draw_wave();
                 print_digit(98,16,SPO2f,' ',3,1);
                 oled.drawChar(116,16,'%');
                 print_digit(98,24,SPO2,' ',3,1);
                 oled.drawChar(116,24,'%');
                 break;
        case 3:  oled.drawStr(10,12,F("Heart Rate"),1);
                 break;
        case 4:  oled.drawStr(10,12,F("Detecting..."),1);
                 break;
        }
    } while (oled.nextPage());
}

void setup(void) {
  pinMode(LED, OUTPUT);
  oled.init();
  oled.fill(0x00);
  draw_oled(3);
  delay(2000);
  if (!sensor.begin())  {
    draw_oled(0);
    while (1);
  }
  sensor.setup(); 
}


const int SPERIOD = 1;
long displaytime = 0;
bool led_on = false;

void loop()  {
    uint32_t irValue = 0;
    uint32_t redValue = 0;
    sensor.check();
    long now = millis();   //start of this cycle
    if (sensor.available()) {
      irValue = sensor.getIR(); 
      redValue = sensor.getRed();
      sensor.nextSample();
      if (irValue < 7000) {
          voltage =getVCC();
          draw_oled(1); 
          delay(100);
          return;
      } 
      int16_t IR_signal = pulseIR.dc_filter(irValue);
      int16_t Red_signal = pulseRed.dc_filter(redValue);
      // record IR in waveform array
      int waveval = -IR_signal/8; // invert wabveform to get classical BP waveshape
      waveval += 128;              //shift so entired waveform is +ve
      waveval = waveval<0? 0 : waveval;
      waveform[wavep] = (uint8_t) (waveval>255)?255:waveval; 
      wavep = (wavep+1) % MAXWAVE;
      // display beat if detected
      pulseRed.isBeat(pulseRed.ma_filter(Red_signal));
      if (pulseIR.isBeat(pulseIR.ma_filter(IR_signal))){
          long thisbeat = now;
          beatsPerMinute = 60000/(thisbeat - lastBeat);
          lastBeat = thisbeat; 
          if (beatsPerMinute < 200 && beatsPerMinute > 20){ 
            MA_rate += beatsPerMinute - MA_rate/RATE_SIZE;
            beatAvg =  MA_rate/RATE_SIZE;
          }
          digitalWrite(LED, HIGH); 
          led_on = true;
          // computed SpO2 ratio
          long numerator   = (pulseRed.avgAC() * pulseIR.avgDC())/256;
          long denominator = (pulseRed.avgDC() * pulseIR.avgAC())/256;
          if (denominator>0) 
                RX100 = (numerator * 100)/denominator;
          else
                RX100 = 999;
          SPO2f = (10400 - RX100*17+50)/100;  //round up
          if ((RX100>=0) && (RX100<184))
                SPO2 = pgm_read_byte_near(&spo2_table[RX100]);
      }
    }
    if (led_on && (now - lastBeat)>25){
        digitalWrite(LED, LOW);
        led_on = false;
    }
    if ((now-displaytime>50) && irValue>=7000) {
        displaytime = now;
        scale_wave();
        draw_oled(2);
    }
}
