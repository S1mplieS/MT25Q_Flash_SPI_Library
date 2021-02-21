#include "MT25Q.h"
#include <cstdint>

MT25Q::MT25Q(SPI* spiPort, PinName cs) : chipSelect(cs)
{
  sectorBuffer = (uint8_t*)malloc(sizeof(uint8_t) * MT25Q_SUBSECTOR_SIZE);
  this->spiPort = spiPort;

  // Code for further initizialation of device
}

/*
  void finishOperation(void) waits until current operation is finished.
*/
void MT25Q::finishOperation(void)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_READ_STATUS_REG);

  while(spiPort->write(0x00) & 0x01){};

  chipSelect = HIGH;
}

/*
  void getJedecId(uint8_t*, uint8_t*, uint8_t*) reads and returns the 3 byte JEDEC ID
  of the connected device.
*/
void MT25Q::getJdecId(uint8_t *mfrId, uint8_t *memType, uint8_t *capacity)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_JEDEC_ID);
  *mfrId = spiPort->write(0x00);     // Read Manufacturer ID (1st Byte of JEDEC ID)
  *memType = spiPort->write(0x00);   // Read Memory Type (2nd Byte of JEDEC ID)
  *capacity = spiPort->write(0x00);  // Read Memory Capacity (3rd Byte of JEDEC ID)

  chipSelect = HIGH;
}

/*
  bool isAvailable(void) checks if the chip is working through JEDEC ID comparison.

  Returns:
    - false    if JEDEC ID does match the expected values
    - true     if JEDEC ID does not match the expected values 
*/
bool MT25Q::isAvailable(void)
{
  uint8_t mfrId, memType, capacity;

  getJdecId(&mfrId, &memType, &capacity);

  if(mfrId != MT25Q_MANUFACUTER_ID || memType != MT25Q_MEMORY_TYPE || capacity != MT25Q_MEMORY_CAPACITY)
  {
    return false;
  }

  return true;
}

/*
  void readBytes(uint32_t, uint8_t*, uint16_t) reads bytes from the flash chip starting from a 4 Byte Address.
*/
void MT25Q::readBytes(uint32_t addrBytes, uint8_t* dataBuffer, uint16_t dataSize)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_READ_DATA);

  spiPort->write((addrBytes & 0xFF000000) >> 24);
  spiPort->write((addrBytes & 0xFF0000) >> 16);
  spiPort->write((addrBytes & 0xFF00) >> 8);
  spiPort->write((addrBytes & 0xFF));

  for(auto i = 0; i < dataSize; i++)
  {
    dataBuffer[i] = spiPort->write(0x00);
  }

  chipSelect = HIGH;
}

/*
  void writePage(uint32_t, uint8_t*) writes a page (256 bytes) of data to the flash chip.
*/
void MT25Q::writePage(uint32_t addrBytes, uint8_t *dataBuffer)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_WRITE_ENABLE);

  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_PAGE_PROGRAM);

  spiPort->write((addrBytes & 0xFF000000) >> 24);
  spiPort->write((addrBytes & 0xFF0000) >> 16);
  spiPort->write((addrBytes & 0xFF00) >> 8);
  spiPort->write(0x00);

  for(auto i = 0; i < MT25Q_PAGE_SIZE; i++)
  {
    spiPort->write(dataBuffer[i]);
  }

  chipSelect = HIGH;
  finishOperation();
}

/*
  void eraseChip(void) erases all data from flash chip.
*/
void MT25Q::eraseChip(void)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_WRITE_ENABLE);

  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_CHIP_EREASE);

  chipSelect = HIGH;
  finishOperation();
}

/*
  void eraseSubsector(uint32_t) erases all data from a Subsector.
*/
void MT25Q::eraseSubsector(uint32_t addrBytes)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_WRITE_ENABLE);

  chipSelect = HIGH;
  chipSelect = LOW;

  spiPort->write(MT25Q_SUBSECTOR_ERASE);

  spiPort->write((addrBytes & 0xFF000000) >> 24);
  spiPort->write((addrBytes & 0xFF0000) >> 16);
  spiPort->write((addrBytes & 0xFF00) >> 8);
  spiPort->write((addrBytes & 0xFF));

  chipSelect = HIGH;
  finishOperation();
}

/*
  void updatePage(uint32_t, uint8_t*) re-writes a page. For this to work
  this function needs to buffer a whole sector and erase it to write all updated pages.
*/
void MT25Q::updatePage(uint32_t addrBytes, uint8_t *dataBuffer)
{
  readBytes(addrBytes & 0xFFFFFF00, sectorBuffer, 4096);

  uint16_t pageAddr = addrBytes & 0xF00;
  for(auto i = pageAddr; i < pageAddr + MT25Q_PAGE_SIZE; i++)
  {
    sectorBuffer[i] = dataBuffer[i - pageAddr];
  }

  eraseSubsector(addrBytes & 0xFFFFF000);

  for(auto i = 0; i < MT25Q_SUBSECTOR_SIZE / MT25Q_PAGE_SIZE; i++)
  {
    pageAddr = (addrBytes & 0xFFFFF000) | (i << 8);

    uint8_t pageBuffer[MT25Q_PAGE_SIZE];
    for(auto j = 0; j < MT25Q_PAGE_SIZE; j++)
    {
      pageBuffer[j] = sectorBuffer[(i << 8) | j];
    }

    writePage(pageAddr, pageBuffer);
  }
}