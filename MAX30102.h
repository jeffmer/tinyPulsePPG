/*
*  MAX30102 driver - uses a combination of ideas from the Maxim & Sparkfun drivers
*                    used Technolbogy's TinyI2C 
*
* j.n.magee 15-10-2019
*/
#ifndef MAX30102_H
#define MAX30102_H
#include <arduino.h>

#define MAX30105_ADDRESS 0x57 

//register addresses
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
#define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

#define STORAGE_SIZE 3 // buffer size in samples

class MAX30102{
 public: 
    MAX30102(void);
  
    boolean begin(uint8_t i2caddr = MAX30105_ADDRESS);  
    
    // Setup the IC with 
    // powerLevel = 0x1F,  sampleAverage = 4,  Mode = Red and IR,  
    // sampleRate = 100,  pulseWidth = 411,  adcRange = 4096  
    void setup();
    void off() {writeRegister8(REG_MODE_CONFIG,0x80);}
    
    //FIFO Reading
    uint16_t check(void);     //Checks for new data and fills FIFO
    uint8_t  available(void); //returns number of samples  available (head - tail)
    void     nextSample(void);//Advances the tail of the sense array
    uint32_t getRed(void);    //Returns the FIFO sample pointed to by tail
    uint32_t getIR(void);     //Returns the FIFO sample pointed to by tail
  
    // Low-level I2C communication
    uint8_t  readRegister8(uint8_t reg);
    uint32_t readFIFOSample(void);
    void     writeRegister8(uint8_t reg, uint8_t value);
 
 private:
    uint8_t _i2caddr;
    struct {
      uint32_t red[STORAGE_SIZE];
      uint32_t IR[STORAGE_SIZE];
      byte head;
      byte tail;
    } sense; //Circular buffer of readings from the sensor

};
#endif
