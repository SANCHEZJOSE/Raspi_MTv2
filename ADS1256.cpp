#include "ADS1256.h"
#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>

ADS1256::ADS1256(int clockspdMhz, float vref, bool usoReset,float t11in,float t6in) {
t11=t11in;
t6=t6in;
  // seleciona el DRDY como entrada
  bcm2835_gpio_fsel(pinDRDY,BCM2835_GPIO_FSEL_INPT);
  // seleciona el CS como salida(control de ads1256)
  bcm2835_gpio_fsel(pinCS,BCM2835_GPIO_FSEL_OUTP);
  // seleciona pin 12 para LED
  bcm2835_gpio_fsel(12,BCM2835_GPIO_FSEL_OUTP);
    // Voltage Reference
  _VREF= vref;

  // Default conversion factor
  _conversionFactor=1.0;

  // Start SPI en BCM2835
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);
  bcm2835_spi_chipSelect(3);//0 control de CS0 de la GPIO,3 control personalizado de CS
  bcm2835_spi_setClockDivider(clockspdMhz);
  //SPI.beginTransaction(SPISettings(clockspdMhz * 1000000/4, MSBFIRST, SPI_MODE1));
  CSON();
  if (usoReset) {
    // selecciona RESET como salida
    bcm2835_gpio_fsel(pinReset,BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(pinReset,LOW);
    bcm2835_delayMicroseconds(10000);
    // reset a high
    bcm2835_gpio_write(pinReset,HIGH);
    bcm2835_delayMicroseconds(1000000);
    bcm2835_spi_transfer(RESET);
    bcm2835_delayMicroseconds(200000);
	}
CSOFF();

}

void ADS1256::writeRegister(unsigned char reg, unsigned char wdata) {
  CSON();
  bcm2835_spi_transfer(WREG | reg);
  bcm2835_spi_transfer(0);
  bcm2835_spi_transfer(wdata);
  bcm2835_delayMicroseconds(t11);  // t11 delay (4*tCLKIN) after WREG command,
                                  // 400Mhz avr clock more
                                  // faster that 7.68 Mhz ADS1256 master clock
  CSOFF();
}

unsigned char ADS1256::readRegister(unsigned char reg) {
  unsigned char readValue;

  CSON();
  bcm2835_spi_transfer(RREG | reg);
  bcm2835_spi_transfer(0);
  bcm2835_delayMicroseconds(t6);  // t6 delay (50*tCLKIN), 16Mhz avr clock is
                                    // approximately twice faster that 7.68 Mhz
                                    // ADS1256 master clock
  readValue = bcm2835_spi_transfer(0);
  bcm2835_delayMicroseconds(t11);  // t11 delay
  CSOFF();

  return readValue;
}

void ADS1256::sendCommand(unsigned char reg) {
  CSON();
  waitDRDY();
  bcm2835_spi_transfer(reg);
  bcm2835_delayMicroseconds(t11);  // t11
  CSOFF();
}

void ADS1256::setConversionFactor(float val) { _conversionFactor = val; }

/*void ADS1256::readTest() {
  unsigned char _highByte, _midByte, _lowByte;
  //CSON();
  bcm2835_spi_transfer(RDATA);
  bcm2835_delayMicroseconds(t6);  // t6 delay

  _highByte = bcm2835_spi_transfer(WAKEUP);
  _midByte = bcm2835_spi_transfer(WAKEUP);
  _lowByte = bcm2835_spi_transfer(WAKEUP);

  //CSOFF();
}*/

float ADS1256::readChannel() {
  CSON();
  bcm2835_spi_transfer(RDATA);
  bcm2835_delayMicroseconds(t6);  // t6 delay
  float adsCode = read_float32();
  CSOFF();
  return ((adsCode / 0x7FFFFF) * ((2 * _VREF) / (float)_pga)) *
         _conversionFactor;
}

// Call this ONLY after RDATA command
unsigned long ADS1256::read_uint24() {
  unsigned char _highByte, _midByte, _lowByte;
  unsigned long value;

  _highByte = bcm2835_spi_transfer(WAKEUP);
  _midByte = bcm2835_spi_transfer(WAKEUP);
  _lowByte = bcm2835_spi_transfer(WAKEUP);

  // Combine all 3-bytes to 24-bit data using byte shifting.
  value = ((long)_highByte << 16) + ((long)_midByte << 8) + ((long)_lowByte);
  return value;
}

// Call this ONLY after RDATA command
long ADS1256::read_int32() {
  long value = read_uint24();

  if (value & 0x00800000) {
    value |= 0xff000000;
  }

  return value;
}

// Call this ONLY after RDATA command
float ADS1256::read_float32() {
  long value = read_int32();
  return (float)value;
}

// Channel switching for single ended mode. Negative input channel are
// automatically set to AINCOM
void ADS1256::setChannel(unsigned char channel) { setChannel(channel, -1); }

// Channel Switching for differential mode. Use -1 to set input channel to
// AINCOM
void ADS1256::setChannel(unsigned char AIN_P, unsigned char AIN_N) {
  unsigned char MUX_CHANNEL;
  unsigned char MUXP;
  unsigned char MUXN;

  switch (AIN_P) {
    case 0:
      MUXP = ADS1256_MUXP_AIN0;
      break;
    case 1:
      MUXP = ADS1256_MUXP_AIN1;
      break;
    case 2:
      MUXP = ADS1256_MUXP_AIN2;
      break;
    case 3:
      MUXP = ADS1256_MUXP_AIN3;
      break;
    case 4:
      MUXP = ADS1256_MUXP_AIN4;
      break;
    case 5:
      MUXP = ADS1256_MUXP_AIN5;
      break;
    case 6:
      MUXP = ADS1256_MUXP_AIN6;
      break;
    case 7:
      MUXP = ADS1256_MUXP_AIN7;
      break;
    default:
      MUXP = ADS1256_MUXP_AINCOM;
  }

  switch (AIN_N) {
    case 0:
      MUXN = ADS1256_MUXN_AIN0;
      break;
    case 1:
      MUXN = ADS1256_MUXN_AIN1;
      break;
    case 2:
      MUXN = ADS1256_MUXN_AIN2;
      break;
    case 3:
      MUXN = ADS1256_MUXN_AIN3;
      break;
    case 4:
      MUXN = ADS1256_MUXN_AIN4;
      break;
    case 5:
      MUXN = ADS1256_MUXN_AIN5;
      break;
    case 6:
      MUXN = ADS1256_MUXN_AIN6;
      break;
    case 7:
      MUXN = ADS1256_MUXN_AIN7;
      break;
    default:
      MUXN = ADS1256_MUXN_AINCOM;
  }

  MUX_CHANNEL = MUXP | MUXN;

  CSON();
  writeRegister(MUX, MUX_CHANNEL);
  sendCommand(SYNC);
  sendCommand(WAKEUP);
  CSOFF();
}

void ADS1256::begin(unsigned char drate, unsigned char gain,bool buffenable) {
  _pga = 1 << gain;
  sendCommand(SDATAC);  // send out SDATAC command to stop continous reading mode.
  writeRegister(DRATE, drate);  // write data rate register
  uint8_t bytemask =0x07;//B00000111;
  uint8_t adcon = readRegister(ADCON);
  uint8_t byte2send = (adcon & ~bytemask) | gain;
  writeRegister(ADCON, byte2send);
  if (buffenable) {
 /*   uint8_t status = readRegister(STATUS);
    bitSet(status, 1);
    writeRegister(STATUS, status);*/
  }
  sendCommand(SELFCAL);  // perform self calibration
  waitDRDY();
    // wait ADS1256 to settle after self calibration
}

void ADS1256::CSON() {
  bcm2835_gpio_write(pinCS,LOW);
}  // digitalWrite(_CS, LOW); }

void ADS1256::CSOFF() {
 bcm2835_gpio_write(pinCS,HIGH);
}

void ADS1256::waitDRDY() {
  while (bcm2835_gpio_lev(pinDRDY))
  ;
}
