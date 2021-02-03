#include "mbed.h"

// SPI Commands for MT25Q Nor Flash Chips
#define MT25Q_WRITE_ENABLE        0x06
#define MT25Q_WRITE_DISABLE       0x04
#define MT25Q_CHIP_EREASE         0xC7
#define MT25Q_READ_STATUS_REG     0x05
#define MT25Q_READ_DATA           0x13  // 4 Byte Address Mode
#define MT25Q_PAGE_PROGRAM        0x12  // 4 Byte Address Mode
#define MT25Q_JEDEC_ID            0x9F
#define MT25Q_RESET_ENABLE        0x66
#define MT25Q_RESET_MEMORY        0x99
#define MT25Q_SUBSECTOR_ERASE     0x21  // 4 Byte Address Mode

// MT25Q JDEC ID Constants (from datasheet of Micron MT25QL256ABA)
#define MT25Q_MANUFACUTER_ID      0x20
#define MT25Q_MEMORY_TYPE         0xBA  // 3V
#define MT25Q_MEMORY_CAPACITY     0x19  // 256Mb

// Mt25Q Memory Constants (from datasheet of Micron MT25QL256ABA)
#define MT25Q_PAGE_SIZE           256   // 256 Byte
#define MT25Q_SUBSECTOR_SIZE      4096  // 4KB

#define HIGH                      0x01
#define LOW                       0x00

class SPIFlash
{
  public:
    SPIFlash(SPI* spiPort, PinName cs);
    int isAvailable(void);
    void getJdecId(uint8_t* mfrId, uint8_t* memType, uint8_t* capacity);
    void readBytes(uint32_t addrBytes, uint8_t* dataBuffer, uint16_t dataSize);
    void writePage(uint32_t addrBytes, uint8_t* dataBuffer);
    void updatePage(uint32_t addrBytes, uint8_t* dataBuffer);
    void eraseChip(void);
    void eraseSubsector(uint32_t addrBytes);

  private:
    SPI* spiPort;
    DigitalOut chipSelect;
    uint8_t* sectorBuffer;

    void finishOperation(void);
};