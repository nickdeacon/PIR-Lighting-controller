//button.c
//Task based button reading
//Scans button array for button up and button down changes
// (c) Nick Deacon 2020
#include "button.h"
#include "main.h"

extern setupData sData;
extern volatile tick_type systemTick;
extern int rfMode;
extern tick_type rfTimeout;
extern uint32_t rfCodeArray[3];
extern int receivedCodes;
extern int currZone;
extern ZONES zones[NUM_ZONES];
int currBtn;
extern uint8_t needSaving;


struct ButtonPress {
	int lastState:1;
	int currState:1;
	int dblClickState:1;
	int buttonAction:1;
	int longClickState:1;
	int unused:27;
	tick_type dblClickTimeout;
	tick_type longClickTimeout;
	tick_type bounceTimeout;
	uint32_t *btnPort;
	uint32_t btnPin;
}btn[NUM_BUTTONS];

void buttonInit(){
	for(int x=0;x<NUM_ZONES;x++) {
		btn[x].lastState=1;
		btn[x].buttonAction=0;
		btn[x].dblClickState=0;
		btn[x].bounceTimeout=0;
		btn[x].longClickState=0;
	}
	
	currBtn=0;
}

void buttonTask(){
	int ind;
	
		if(rfMode==RF_MODE_LEARN)
			return; //disable all buttons while in learning mode
		int btn_res=oneBtnTask(currBtn);
		if(btn_res){
			//action to be taken
			if(currBtn<4) {
				currZone=currBtn;
				switch(btn_res) {
					case 1:
						// single click
						zones[currZone].btnActivated=ZONE_ACTIVE;
						break;
					case 2:
						// double click learn mode
						rfMode=RF_MODE_LEARN;
						rfCodeArray[0]=rfCodeArray[1]=rfCodeArray[2]=0;
						receivedCodes=0;
						rfTimeout=systemTick+(10 * TICKS_PER_SECOND);
						break;
					case 3:
						//long click erase zone rfcodes
						ind=currBtn<<2;
						for(int x=ind;x<ind+4;x++){
							sData.rfCodeArray[x]=0xffffffff;
						
						}
						needSaving=1;
					
						break;
				}
			}
			else {
				//push button on pcb
				if(btn_res==1) {
					SaveLightLevel();
					
				}
				
			}
		}
		currBtn++;
		if (currBtn==NUM_ZONES+1)
			currBtn=0;
}
int oneBtnTask(int ind) {
	
	
	if(systemTick<btn[ind].bounceTimeout)
			return 0; //ignore keybounce
	
	btn[ind].currState=HAL_GPIO_ReadPin(Button_array[ind].port,Button_array[ind].mask);
	if(btn[ind].currState != btn[ind].lastState){
		//button state has changed
		if(!btn[ind].currState){
			//button down
			if(!btn[ind].buttonAction) {
				btn[ind].longClickTimeout=systemTick+LONG_CLICK_DURATION; //mark end of long click
				btn[ind].dblClickTimeout=systemTick+DOUBLE_CLICK_PERIOD;
			}
		}
		else {
			//button released
			
			if(systemTick<btn[ind].dblClickTimeout) {
					
					if(btn[ind].buttonAction) {
						//double click
						btn[ind].dblClickState=1;
					}
					
			}
			btn[ind].buttonAction=1;
			btn[ind].bounceTimeout=systemTick+KEY_BOUNCE_PERIOD;
			//btn[ind].longClickState=0;
		}
		btn[ind].lastState=btn[ind].currState;
	}
	else {
			
			if(btn[ind].buttonAction && systemTick>btn[ind].dblClickTimeout){
				//check for button press type
				btn[ind].buttonAction=0;
				if(systemTick>btn[ind].longClickTimeout){
					//long click
					btn[ind].buttonAction=0;
					btn[ind].dblClickState=0;	
					//btn[ind].longClickState=0;
					return 3;
				}	
					
				if(btn[ind].dblClickState){
					//double click
					btn[ind].buttonAction=0;
					btn[ind].dblClickState=0;	
					//btn[ind].longClickState=0;
					return 2;
				}
				else {
					//normal click
					btn[ind].buttonAction=0;
					btn[ind].dblClickState=0;	
					//btn[ind].longClickState=0;
					return 1;
				}
					
			}
	}			
		
	
	return 0;
}
