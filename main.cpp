#include "mbed.h"
#include "SPIFlash.h"
#include <chrono>
#include <cstdint>

void printPageBytes(uint8_t* dataBuffer);
void fillWithString(const char* dataString, int stringSize, uint8_t* dataBuffer);

int main()
{
  Timer timer;

  printf("\n\nProg Start\n");
  SPI spiPort(A6, A5, A4);
  SPIFlash flashHandle(&spiPort, A3);

  // Check if chip is available
  if(flashHandle.isAvailable() == -1)
  {
    printf("Flash Chip is not ready!\n");
    while(true){};
  }

  uint8_t mfrId, memType, capacity;
  flashHandle.getJdecId(&mfrId, &memType, &capacity);
  printf("Manufacturer ID: 0x%02X\nMemory Type: 0x%02X\nMemory Capacity: 0x%02X\n", mfrId, memType, capacity);

  uint8_t dataBuffer[256];

  timer.start();
  flashHandle.eraseChip();
  timer.stop();
  printf("Erase Chip took %llu ms!\n",chrono::duration_cast<chrono::milliseconds>(timer.elapsed_time()).count());

  timer.reset();
  fillWithString("SPI Flash Functionality Test", 30, dataBuffer);
  timer.start();
  flashHandle.writePage(0x00, dataBuffer);
  timer.stop();
  printf("Write Page took %llu ms!\n",chrono::duration_cast<chrono::milliseconds>(timer.elapsed_time()).count());

  timer.reset();
  fillWithString("An apple a day keeps the doctor away!", 37, dataBuffer);
  timer.start();
  flashHandle.writePage(0x0F00, dataBuffer);
  timer.stop();
  printf("Write Page took %llu ms!\n",chrono::duration_cast<chrono::milliseconds>(timer.elapsed_time()).count());

  timer.reset();
  timer.start();
  flashHandle.updatePage(0x00, dataBuffer);
  timer.stop();
  printf("Update Page took %llu ms!\n",chrono::duration_cast<chrono::milliseconds>(timer.elapsed_time()).count());

  timer.reset();
  timer.start();
  flashHandle.readBytes(0x00, dataBuffer, 256);
  timer.stop();
  printf("Read Page took %llu ms!\n",chrono::duration_cast<chrono::milliseconds>(timer.elapsed_time()).count());
  printPageBytes(dataBuffer);

  timer.reset();
  timer.start();
  flashHandle.readBytes(0x0F00, dataBuffer, 256);
  timer.stop();
  printf("Read Page took %llu ms!\n",chrono::duration_cast<chrono::milliseconds>(timer.elapsed_time()).count());
  printPageBytes(dataBuffer);
} 

void fillWithString(const char* dataString, int stringSize, uint8_t* dataBuffer)
{
  for(int i = 0; i < stringSize; i++)
  {
    dataBuffer[i] = dataString[i];
  }
}

void printPageBytes(uint8_t* dataBuffer)
{
  printf("\n");
  for(uint8_t i = 0; i < 16; i++)
  {
    for(uint8_t j = 0; j < 16; j++)
    {
      printf("\'%c\' ", (char)dataBuffer[i * 16 + j]);
    }

    printf("\n");
  }
  printf("\n");
}