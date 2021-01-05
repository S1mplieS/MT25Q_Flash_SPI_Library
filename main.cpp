#include "mbed.h"
#include "SPIFlash.h"
#include <cstdint>

void printPageBytes(uint8_t* dataBuffer);

int main()
{
  printf("\n\nProg Start\n");

  SPIFlash flashHandle(A6, A5, A4, A3);

  // Check if chip is available
  if(flashHandle.isAvailable() == -1)
  {
    printf("Flash Chip is not ready!\n");
    while(true){};
  }

  uint8_t mfrId, memType, capacity;
  flashHandle.getJdecId(&mfrId, &memType, &capacity);
  printf("Manufacturer ID: 0x%02X\nMemory Type: 0x%02X\nMemory Capacity: 0x%02X\n", mfrId, memType, capacity);

  uint8_t byteBuffer[1024];   // 4KB Buffer

  flashHandle.eraseSubsector(0x00);

  //flashHandle.readBytes(0x00, byteBuffer, 1024);
  //printPageBytes(byteBuffer);
//
  //uint8_t writeBuffer[256];
  //writeBuffer[255] = 0xA1;
  //writeBuffer[0] = 0x11;
  //flashHandle.writePage(0x0300, writeBuffer);
//
  //writeBuffer[255] = 0xAC;
  //writeBuffer[0] = 0x33;
  //flashHandle.writePage(0x00, writeBuffer);

  flashHandle.readBytes(0x00, byteBuffer, 1024);
  printPageBytes(byteBuffer);
} 

void printPageBytes(uint8_t* dataBuffer)
{
  printf("\n");
  for(uint8_t i = 0; i < 32; i++)
  {
    for(uint8_t j = 0; j < 32; j++)
    {
      printf("0x%02X ", dataBuffer[i * 32 + j]);
    }

    printf("\n");
  }
  printf("\n");
}