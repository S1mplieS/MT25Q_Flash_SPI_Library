#include "mbed.h"
#include "MT25Q.h"
#include <cstdint>

static uint8_t tempPageD[MT25Q_SUBSECTOR_SIZE];

int main()
{
  printf("\n---- Flash Memory Test Start ----\n");
  MT25Q flashMemory(PE_6, PE_5, PE_12, D3); // Change Pins before use, Order: (MOSI, MISO, CLK, CS)

  if(!flashMemory.isAvailable())
  {
    printf("[ERROR] Flash memory not available!\n");
    while(true){}
  }
  
  printf("[INFO] Flash memory is available!\n");

  uint8_t tempPageA[MT25Q_PAGE_SIZE];
  flashMemory.eraseChip();
  flashMemory.readBytes(0x00, tempPageA, MT25Q_PAGE_SIZE);

  bool wasEreased = true;
  for(auto i = 0; i < MT25Q_PAGE_SIZE; i++)
  {
    if(tempPageA[i] != 0xFF)
    {
      wasEreased = false;
      printf("[ERROR] Memory was not ereased correctly! Memory test failed!\n");
      break;
    }
  }

  if(wasEreased)
  {
    printf("[INFO] Memory was ereased correctly!\n");
  }

  uint8_t tempPageB[MT25Q_PAGE_SIZE] = {"This is a test sentence!"};
  flashMemory.writeBytes(0x5000, tempPageB);
  flashMemory.readBytes(0x5000, tempPageA, MT25Q_PAGE_SIZE);

  bool isEqual = true;
  for(auto i = 0; i < MT25Q_PAGE_SIZE; i++)
  {
    if(tempPageA[i] != tempPageB[i])
    {
      isEqual = false;
      printf("[ERROR] Memory was not written or read (or both) correctly! Memory test failed!\n");
      break;
    }
  }

  if(isEqual)
  {
    printf("[INFO] Memory was written and read successfully!\n");
  }

  uint8_t tempPageC[MT25Q_PAGE_SIZE] = {"Another but different test sentence!"};
  
  flashMemory.updateBytes(0x5000, tempPageC);
  flashMemory.readBytes(0x5000, tempPageD, MT25Q_SUBSECTOR_SIZE);

  isEqual = true;
  for(auto i = 0; i < MT25Q_SUBSECTOR_SIZE; i++)
  {
    uint8_t compareValue = i >= MT25Q_PAGE_SIZE ? 0xFF : tempPageC[i];
    if(tempPageD[i] != compareValue)
    {
      isEqual = false;
      printf("[ERROR] Memory was not updated or read (or both) correctly! Memory test failed!\n");
      break;
    }
  }

  if(isEqual)
  {
    printf("[INFO] Memory was updated successfully!\n");
  }
}
