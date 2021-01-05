#include "SPIFlash.h"
#include <cstdint>

SPIFlash::SPIFlash(PinName mosi, PinName miso, PinName clk, PinName cs) : spiHandle(mosi, miso, clk), chipSelect(cs)
{
  // Code for further initizialation of device
}

/*
  void finishOperation(void) waits until current operation is finished.
*/
void SPIFlash::finishOperation(void)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_READ_STATUS_REG);

  while(spiHandle.write(0x00) & 0x01){};

  chipSelect = HIGH;
}

/*
  void getJedecId(uint8_t*, uint8_t*, uint8_t*) reads and returns the 3 byte JEDEC ID
  of the connected device.
*/
void SPIFlash::getJdecId(uint8_t *mfrId, uint8_t *memType, uint8_t *capacity)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_JEDEC_ID);
  *mfrId = spiHandle.write(0x00);     // Read Manufacturer ID (1st Byte of JEDEC ID)
  *memType = spiHandle.write(0x00);   // Read Memory Type (2nd Byte of JEDEC ID)
  *capacity = spiHandle.write(0x00);  // Read Memory Capacity (3rd Byte of JEDEC ID)

  chipSelect = HIGH;
}

/*
  int isAvailable(void) checks if the chip is working through JEDEC ID comparison.

  Returns:
    - (  0 )    if JEDEC ID does match the expected values
    - ( -1 )    if JEDEC ID does not match the expected values 
*/
int SPIFlash::isAvailable(void)
{
  uint8_t mfrId, memType, capacity;

  getJdecId(&mfrId, &memType, &capacity);

  if(mfrId != MT25Q_MANUFACUTER_ID || memType != MT25Q_MEMORY_TYPE || capacity != MT25Q_MEMORY_CAPACITY)
  {
    return -1;
  }

  return 0;
}

/*
  void readBytes(uint32_t, uint8_t*, uint16_t) reads bytes from the flash chip starting from a 4 Byte Address.
*/
void SPIFlash::readBytes(uint32_t addrBytes, uint8_t* dataBuffer, uint16_t dataSize)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_READ_DATA);

  spiHandle.write((addrBytes & 0xFF000000) >> 24);
  spiHandle.write((addrBytes & 0xFF0000) >> 16);
  spiHandle.write((addrBytes & 0xFF00) >> 8);
  spiHandle.write((addrBytes & 0xFF));

  for(auto i = 0; i < dataSize; i++)
  {
    dataBuffer[i] = spiHandle.write(0x00);
  }

  chipSelect = HIGH;
}

/*
  void writePage(uint32_t, uint8_t*) writes a page (256 bytes) of data to the flash chip.
*/
void SPIFlash::writePage(uint32_t addrBytes, uint8_t *dataBuffer)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_WRITE_ENABLE);

  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_PAGE_PROGRAM);

  spiHandle.write((addrBytes & 0xFF000000) >> 24);
  spiHandle.write((addrBytes & 0xFF0000) >> 16);
  spiHandle.write((addrBytes & 0xFF00) >> 8);
  spiHandle.write(0x00);

  for(auto i = 0; i < 256; i++)
  {
    spiHandle.write(dataBuffer[i]);
  }

  chipSelect = HIGH;
  finishOperation();
}

/*
  void eraseChip(void) erases all data from flash chip.
*/
void SPIFlash::eraseChip(void)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_WRITE_ENABLE);

  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_CHIP_EREASE);

  chipSelect = HIGH;
  finishOperation();
}

/*
  void eraseSubsector(uint32_t) erases all data from a Subsector.
*/
void SPIFlash::eraseSubsector(uint32_t addrBytes)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_WRITE_ENABLE);

  chipSelect = HIGH;
  chipSelect = LOW;

  spiHandle.write(MT25Q_SUBSECTOR_ERASE);

  spiHandle.write((addrBytes & 0xFF000000) >> 24);
  spiHandle.write((addrBytes & 0xFF0000) >> 16);
  spiHandle.write((addrBytes & 0xFF00) >> 8);
  spiHandle.write((addrBytes & 0xFF));

  chipSelect = HIGH;
  finishOperation();
}