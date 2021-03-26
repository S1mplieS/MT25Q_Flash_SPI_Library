#include "MT25Q.h"
#include <chrono>
#include <cstdint>

uint8_t MT25Q::sectorBuffer[];

MT25Q::MT25Q(PinName mosi, PinName miso, PinName clk, PinName cs) : spi(mosi, miso, clk), chipSelect(cs)
{
  spi.format(8);
  spi.frequency(10000000);

  // Code for further initizialation of device
}

/*
  void sendGeneralCommand(uint8_t, uint64_t, const uint8_t*, size_t, uint8_t*, size_t) sends command to flash controller
  and recieves or transmits additional data.
*/
void MT25Q::sendGeneralCommand(uint8_t cmd, uint64_t addr, const uint8_t* txBuffer, size_t txSize, uint8_t* rxBuffer, size_t rxSize)
{
  //printf("command: 0x%02X addr: 0x%02X\n", cmd, (uint32_t)addr);

  chipSelect = HIGH;
  chipSelect = LOW;

  spi.write(cmd);

  if(addr != NO_ADDRESS_COMMAND)
  {
    // Write 4 Byte Address
    for(auto addrShift = 24; addrShift >= 0; addrShift -= 8)
    {
      spi.write((addr >> addrShift) & 0xFF);
    }
  }

  // Write Data
  for(auto i = 0; i < txSize; i++)
  {
    spi.write(txBuffer[i]);
  }

  // Read Data
  for(auto i = 0; i < rxSize; i++)
  {
    rxBuffer[i] = spi.write(0x00);
  }

  chipSelect = HIGH;
}

/*
  void sendReadCommand(uint64_t, uint8_t*, size_t) sends read command to flash controller and returns recieved data.
*/
void MT25Q::sendReadCommand(uint64_t addr, uint8_t* buffer, size_t size)
{
  chipSelect = HIGH;
  chipSelect = LOW;

  spi.write(MT25Q_READ_DATA);

  // Write 4 Byte Address
  for(auto addrShift = 24; addrShift >= 0; addrShift -= 8)
  {
    spi.write((addr >> addrShift) & 0xFF);
  }

  // Read Data
  for(auto i = 0; i < size; i++)
  {
    buffer[i] = spi.write(0x00);
  }

  chipSelect = HIGH;
}

/*
  void readBytes(uint32_t, uint8_t*, size_t) reads bytes starting from given 4 byte address.
*/
void MT25Q::readBytes(uint32_t addr, uint8_t* buffer, size_t size)
{
  sendReadCommand(addr, buffer, size);
}

/*
  bool isAvailable(void) checks if the chip is working through JEDEC ID comparison.

  Returns:
    - true    if JEDEC ID does match the expected values
    - false   if JEDEC ID does not match the expected values 
*/
bool MT25Q::isAvailable(void)
{
  uint8_t jedecInfo[3];
  sendGeneralCommand(MT25Q_JEDEC_ID, NO_ADDRESS_COMMAND, NULL, 0, jedecInfo, 3);

  printf("%X %X %X\n", jedecInfo[0], jedecInfo[1], jedecInfo[2]);

  if(jedecInfo[0] != MT25Q_MANUFACUTER_ID || jedecInfo[1] != MT25Q_MEMORY_TYPE || jedecInfo[2] != MT25Q_MEMORY_CAPACITY)
  {
    return false;
  }

  return true;
}

/*
  bool isMemoryReady(void) reads Status Register if a write is in progress. 
  If not, chip is ready to recieve further commands.
*/
bool MT25Q::isMemoryReady(void)
{
  uint8_t statusValues[2];
  int retries = 0;

  do
  {
    ThisThread::sleep_for(chrono::milliseconds(1));
    sendGeneralCommand(MT25Q_READ_STATUS_REG, NO_ADDRESS_COMMAND, NULL, 0, statusValues, 1);
    retries++;

  } while((statusValues[0] & WRITE_IN_PROGRESS) != 0 && retries < 1000);

  if((statusValues[0] & WRITE_IN_PROGRESS) != 0)
  {
    return false;
  }

  return true;
}

/*
  void sendProgramCommand(uint64_t, const uint8_t*) writes bytes to flash memory
  using program command.
*/
void MT25Q::sendProgramCommand(uint64_t addr, const uint8_t* data)
{
  sendGeneralCommand(MT25Q_WRITE_ENABLE, NO_ADDRESS_COMMAND, NULL, 0, NULL, 0);
  sendGeneralCommand(MT25Q_PROGRAM, addr, data, MT25Q_PAGE_SIZE, NULL, 0);

  // Wait until all write operations are finished
  isMemoryReady();
}

/*
  void writeBytes(uint32_t, const uint8_t*) writes bytes to flash.
  Data must be written a page at a time (256 bytes).
*/
void MT25Q::writeBytes(uint32_t addr, const uint8_t *data)
{
  sendProgramCommand(addr, data);
}

/*
  void sendEraseCommand(uint64_t) sends command erase command and waits until operation is done.
*/
void MT25Q::sendEraseCommand(uint8_t eraseCmd, uint64_t addr)
{
  sendGeneralCommand(MT25Q_WRITE_ENABLE, NO_ADDRESS_COMMAND, NULL, 0, NULL, 0);
  sendGeneralCommand(eraseCmd, addr, NULL, 0, NULL, 0);

  // Wait until all write operations are finished
  isMemoryReady();
}

/*
  void eraseBytes(uint32_t) erases a whole Subsector (4KB) at a given address.
*/
void MT25Q::eraseBytes(uint32_t addr)
{
  sendEraseCommand(MT25Q_SUBSECTOR_ERASE, addr);
}

/*
  void eraseChip(void) erases whole chip and waits until operation is done.
*/
void MT25Q::eraseChip(void)
{
  sendEraseCommand(MT25Q_BULK_ERASE, NO_ADDRESS_COMMAND);
}

/*
  void updateBytes(uint32_t, const uint8_t*) updates page at given address.
  Because page must be erased before re-written and the min size to erase
  is a Subsector (4KB), rest of Subsector must be buffered and also re-written.
*/
void MT25Q::updateBytes(uint32_t addr, const uint8_t* data)
{
  sendReadCommand(addr & 0xFFFFF000, sectorBuffer, MT25Q_SUBSECTOR_SIZE);

  uint16_t pageAddr = addr & 0xF00;
  copy_n(data, MT25Q_PAGE_SIZE, &sectorBuffer[addr & 0xF00]);

  sendEraseCommand(MT25Q_SUBSECTOR_ERASE, addr & 0xFFFFF000);

  for(auto i = 0; i < MT25Q_SUBSECTOR_SIZE / MT25Q_PAGE_SIZE; i++)
  {
    uint32_t pageAddr = (addr & 0xFFFFF000) | (i << 8);
    writeBytes(pageAddr, &sectorBuffer[i * MT25Q_PAGE_SIZE]);
  }
}