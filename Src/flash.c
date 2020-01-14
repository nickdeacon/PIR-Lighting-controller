//flash.c

#include "stm32f0xx_hal.h"
#include "main.h"

#define FLASH_PAGE 0x08007000 //for stm32f030c6t6 on sonoff ch4 pro
//#define FLASH_SECTOR 31 //sector size is 1k (0x400)
//#define FLASH_PAGE 0x0800FC00 //for stm32f030c6r8 development board
//#define FLASH_SECTOR 63 //sector size is 1k (0x400)
//#define FLASH_FKEY1 0x45670123
//#define FLASH_FKEY2 0xCDEF89AB

extern setupData sData, *pData;

void flashWrite(){
__IO uint32_t addr;	
	
	
	HAL_FLASH_Unlock();
	FLASH_PageErase(FLASH_PAGE);
	CLEAR_BIT(FLASH->CR,(FLASH_CR_PER));
	addr=FLASH_PAGE;
	CLEAR_BIT(FLASH->CR,(FLASH_CR_PG));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,addr,sData.lightLevel);

	CLEAR_BIT(FLASH->CR,(FLASH_CR_PG));
	addr+=sizeof(uint32_t);
	for(int x=0;x<16;x++){
		if(sData.rfCodeArray[x]!=0xffffffff){
			CLEAR_BIT(FLASH->CR,(FLASH_CR_PG));
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,addr,sData.rfCodeArray[x]);
		}
		addr+=sizeof(uint32_t);
	}
	HAL_FLASH_Lock();
}	
	
	
	
	
	

		

void flashRead(void){
	uint32_t *pd = (uint32_t*)FLASH_PAGE;
				
        
        //*pData = *pd;                    //Attempt to read flash mem
        
				sData.lightLevel=*pd;
				pd++;
				for(int x=0;x<16;x++) {
					sData.rfCodeArray[x]=*pd;
					pd++;
				}					

        
}

