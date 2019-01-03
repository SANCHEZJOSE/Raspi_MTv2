#ifndef ADS_1256_H_
#define ADS_1256_H_
#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>
// Pines GPIO con BCM2835 http://www.airspayce.com/mikem/bcm2835/group__constants.html#gga63c029bd6500167152db4e57736d0939a1517b7c4655c6717f16b83effd033a9e
#define pinReset 17 
#define pinCS 27
#define pinDRDY 22 
// ADS1256 Register
#define STATUS 0x00
#define MUX 0x01
#define ADCON 0x02
#define DRATE 0x03
#define IO 0x04
#define OFC0 0x05
#define OFC1 0x06
#define OFC2 0x07
#define FSC0 0x08
#define FSC1 0x09
#define FSC2 0x0A
// ADS1256 Command
#define WAKEUP 0x00
#define RDATA 0x01
#define RDATAC 0x03
#define SDATAC 0x0f
#define RREG 0x10
#define WREG 0x50
#define SELFCAL 0xF0
#define SELFOCAL 0xF1
#define SELFGCAL 0xF2
#define SYSOCAL 0xF3
#define SYSGCAL 0xF4
#define SYNC 0xFC
#define STANDBY 0xFD
#define RESET 0xFE
// define multiplexer codes
#define ADS1256_MUXP_AIN0 0x00
#define ADS1256_MUXP_AIN1 0x10
#define ADS1256_MUXP_AIN2 0x20
#define ADS1256_MUXP_AIN3 0x30
#define ADS1256_MUXP_AIN4 0x40
#define ADS1256_MUXP_AIN5 0x50
#define ADS1256_MUXP_AIN6 0x60
#define ADS1256_MUXP_AIN7 0x70
#define ADS1256_MUXP_AINCOM 0x80

#define ADS1256_MUXN_AIN0 0x00
#define ADS1256_MUXN_AIN1 0x01
#define ADS1256_MUXN_AIN2 0x02
#define ADS1256_MUXN_AIN3 0x03
#define ADS1256_MUXN_AIN4 0x04
#define ADS1256_MUXN_AIN5 0x05
#define ADS1256_MUXN_AIN6 0x06
#define ADS1256_MUXN_AIN7 0x07
#define ADS1256_MUXN_AINCOM 0x08

// define gain codes
#define ADS1256_GAIN_1 0x00
#define ADS1256_GAIN_2 0x01
#define ADS1256_GAIN_4 0x02
#define ADS1256_GAIN_8 0x03
#define ADS1256_GAIN_16 0x04
#define ADS1256_GAIN_32 0x05
#define ADS1256_GAIN_64 0x06

// define drate codes
#define ADS1256_DRATE_30000SPS 0xF0
#define ADS1256_DRATE_15000SPS 0xE0
#define ADS1256_DRATE_7500SPS 0xD0
#define ADS1256_DRATE_3750SPS 0xC0
#define ADS1256_DRATE_2000SPS 0xB0
#define ADS1256_DRATE_1000SPS 0xA1
#define ADS1256_DRATE_500SPS 0x92
#define ADS1256_DRATE_100SPS 0x82
#define ADS1256_DRATE_60SPS 0x72
#define ADS1256_DRATE_50SPS 0x63
#define ADS1256_DRATE_30SPS 0x53
#define ADS1256_DRATE_25SPS 0x43
#define ADS1256_DRATE_15SPS 0x33
#define ADS1256_DRATE_10SPS 0x23
#define ADS1256_DRATE_5SPS 0x13
#define ADS1256_DRATE_2_5SPS 0x03
class ADS1256 {
 public:
  ADS1256(int clockspdMhz, float vref, bool useresetpin,float t11in,float t6in);
  void writeRegister(unsigned char reg, unsigned char wdata);
  unsigned char readRegister(unsigned char reg);
  void sendCommand(unsigned char cmd);
  float readChannel();
  void setConversionFactor(float val);
  void setChannel(unsigned char channel);
  void setChannel(unsigned char AIP, unsigned char AIN);
  void begin(unsigned char drate, unsigned char gain, bool bufferenable);
  void waitDRDY();
  void setGain(uint8_t gain);
  ~ADS1256() {bcm2835_spi_end();}
  //void readTest();
  
 private:
  void CSON();
  void CSOFF();
  unsigned long read_uint24();
  long read_int32();
  float read_float32(),t11,t6;
  unsigned char _pga;
  float _VREF;
  float _conversionFactor;
};
#endif
/*bcm2835_spi_begin() Inicializa el módulo de SPI.
bcm2835_spi_end() Libera los recursos empleados en la inicialización.
bcm2835_spi_setClockDivider(div)  Define el divisor del reloj.
bcm2835_spi_setDataMode(modo) Modo SPI.
bcm2835_spi_chipSelect(cs)  Pin de CS (0, 1, 2), 3 = ninguno.
bcm2835_spi_setChipSelectPolarity(cs, v)  Define la polaridad del pin cs como activa a valor v.
bcm2835_spi_transfer(v) Transmite y recibe (devuelve) un byte.
bcm2835_spi_transfernb(tbuf, rbuf, len) Transmite y recibe len bytes.
bcm2835_spi_transfern (buf, len)  Transmite y recibe len bytes en el mismo buffer.
bcm2835_spi_writenb (buf, len)  Transmite len bytes.*/
