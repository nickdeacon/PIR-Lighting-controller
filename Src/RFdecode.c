//RFdecode.c

#include "stm32f0xx_hal.h"
#include "RFdecode.h"
#include "main.h"

extern setupData sData;

uint32_t rfData;
extern uint32_t rfReceiveCode;
uint16_t rfStatus;
int16_t rfBitCount;
uint32_t pulseLen, quietLen;


void RFInit() {
	rfData=0;
	rfBitCount=0;
	rfStatus=RF_LISTEN;
	
	
}

int rfGetStatus()
{
	return rfStatus;
}

void rfSetStatus(int stat)
{
	rfStatus=stat;
}

int RFTask(void){
	
	if(rfStatus==RF_HAS_BIT){
		rfData=rfData<<1;
		if(pulseLen>quietLen)
			rfData++;
		rfBitCount++;
		if(rfBitCount<RF_MAX_BITS)
			rfStatus=RF_LISTEN_FOR_EDGE;
		else {
			rfStatus=RF_IDLE;
			rfBitCount=0;
			return rfData&0xffffff;
		}	
	}
	
	return 0;
}



int findCodeZone(int code, int zone)
{
	zone=zone<<2;
	for(int x=zone;x<zone+4;x++) {
		if(sData.rfCodeArray[x]==code)
				return 1;
	}
	return 0;
	
}


int findFirstSlot(int zone) {
int index;	
	index=zone<<2;
	for(int x=index;x<index+4;x++) {
		if(sData.rfCodeArray[x]==0 || sData.rfCodeArray[x]==0xffffffff)
				return x;
	}
	return -1;
}
