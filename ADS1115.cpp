
/**************************************************************************/
#include <bcm2835.h>
#include <unistd.h>
#include "ADS1115.h"


/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
static void writeRegister(char i2cAddress, char reg, char16_t value) {
  char wr[]={ reg,(char)(value >> 8),(char)(value & 0xFF)};
  bcm2835_i2c_write(wr,3);
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
static uint16_t readRegister(uint8_t i2cAddress, uint8_t reg) {
  char wr[]={ADS1115_REG_POINTER_CONVERT};
  bcm2835_i2c_write(wr,1);
  char rd[2];
  bcm2835_i2c_read (rd,2);
  return ((rd[0] << 8) | rd[1]);  
}
/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties
*/
/**************************************************************************/
ADS1115::ADS1115(uint8_t i2cAddress)
{
   m_i2cAddress = i2cAddress;
   m_conversionDelay = ADS1115_CONVERSIONDELAY;
   m_bitShift = 0;
   m_gain = GAIN_TWOTHIRDS; /* +/- 6.144V range (limited to VDD +0.3V max!) */
   m_sps = SPS_860;
}

/**************************************************************************/
/*!
    @brief  Sets up the HW (reads coefficients values, etc.)
*/
/**************************************************************************/
void ADS1115::begin() {
  bcm2835_init();
  bcm2835_i2c_begin();
  bcm2835_i2c_setSlaveAddress(m_i2cAddress);
  bcm2835_i2c_setClockDivider (BCM2835_I2C_CLOCK_DIVIDER_626);
}

/**************************************************************************/
/*!
    @brief  Sets the gain and input voltage range
*/
/**************************************************************************/
void ADS1115::setGain(adsGain_t gain)
{
  m_gain = gain;
}

/**************************************************************************/
/*!
    @brief  Gets a gain and input voltage range
*/
/**************************************************************************/
adsGain_t ADS1115::getGain()
{
  return m_gain;
}
/// selección de SPS
void ADS1115::setSPS(adsSPS_t SPS)
{
  m_sps =SPS;
}

void ADS1115::Differential_0_1() {
  // Start with default values
  char16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    m_sps   | // 860 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;
                    
  // Set channels
  config |= ADS1115_REG_CONFIG_MUX_DIFF_0_1;          // AIN0 = P, AIN1 = N

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);

}
void ADS1115::Differential_2_3() {
  // Start with default values
  char16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    m_sps   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set channels
  config |= ADS1115_REG_CONFIG_MUX_DIFF_2_3;          // AIN2 = P, AIN3 = N

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);

  
}

int16_t ADS1115::getConversion()
{
  // Wait for the conversion to complete
  //bcm2835_delayMicroseconds(m_conversionDelay);

  // Read the conversion results
  uint16_t res = readRegister(m_i2cAddress, ADS1115_REG_POINTER_CONVERT) >> m_bitShift;
  if (m_bitShift == 0)
  {
    return (int16_t)res;
  }
  else
  {
    // Shift 12-bit results right 4 bits for the ADS1115,
    // making sure we keep the sign bit intact
    if (res > 0x07FF)
    {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}
