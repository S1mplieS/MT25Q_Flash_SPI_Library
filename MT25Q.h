#include "mbed.h"
#include <cstdint>

#define NO_ADDRESS_COMMAND      UINT64_MAX
#define WRITE_IN_PROGRESS       0x01

#define HIGH                    0x01
#define LOW                     0x00

// MT25Q JDEC ID Constants (from datasheet of Micron MT25QL256ABA)
#define MT25Q_MANUFACUTER_ID      0x20
#define MT25Q_MEMORY_TYPE         0xBA  // 3V
#define MT25Q_MEMORY_CAPACITY     0x19  // 256Mb

// MT25Q Memory Constants (from datasheet of Micron MT25QL256ABA)
#define MT25Q_PAGE_SIZE           256   // 256 Byte
#define MT25Q_SUBSECTOR_SIZE      4096  // 4KB

// SPI Commands for MT25Q Nor Flash Chips
#define MT25Q_ENABLE_4BYTE_ADDR 0xB7
#define MT25Q_READ_DATA         0x13  // 4 Byte Address Mode
#define MT25Q_JEDEC_ID          0x9F
#define MT25Q_WRITE_ENABLE      0x06
#define MT25Q_WRITE_DISABLE     0x04
#define MT25Q_PROGRAM           0x12  // 4 Byte Address Mode
#define MT25Q_READ_STATUS_REG   0x05
#define MT25Q_SUBSECTOR_ERASE   0x21  // 4 Byte Address Mode (4KB Subsector)
#define MT25Q_BULK_ERASE        0xC7

#ifndef MT25Q_H
#define MT25Q_H
class MT25Q
{
  public:
    MT25Q(PinName mosi, PinName miso, PinName clk, PinName cs);
    bool isAvailable(void);
    bool isMemoryReady(void);
    void readBytes(uint32_t addr, uint8_t* buffer, size_t size);
    void writeBytes(uint32_t addr, const uint8_t* data, size_t size);
    void eraseBytes(uint32_t addr);
    void eraseChip(void);
    void updateBytes(uint32_t addr, const uint8_t* data);

  private:
    SPI spi;
    DigitalOut chipSelect;

    void sendGeneralCommand(uint8_t cmd, uint64_t addr, const uint8_t* txBuffer, size_t txSize, uint8_t* rxBuffer, size_t rxSize);
    void sendReadCommand(uint64_t addr, uint8_t* buffer, size_t size);
    void sendProgramCommand(uint64_t addr, const uint8_t* data);
    void sendEraseCommand(uint8_t eraseCmd, uint64_t addr);
};
#endif