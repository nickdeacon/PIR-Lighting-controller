/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
	extern volatile tick_type systemTick;
	
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TICKS_PER_SECOND 20000 //timer 3 running at 10,000Hz
//#define __DEBUG_ENABLE // uncomment for debug messages over serial port
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
	setupData sData, *pData;
	ZONES zones[NUM_ZONES];
	uint32_t rfReceiveCode;
	int flashtest;
	uint8_t rfMode;
	int currLightLevel;
	tick_type rfTimeout=0;
	uint32_t rfCodeArray[3];
	int receivedCodes;
	int currZone;
	int lightTimer;
	uint8_t needSaving;
	uint8_t saveLightLevel;
	extern uint16_t rfStatus;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
	extern void buttonTask(void);
	extern int flashRead(void);
	extern void flashWrite(void);
	extern int findCodeZone(int,int);
	extern int findFirstSlot(int);
	extern int rfGetStatus(void);
	extern void rfSetStatus(int);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/*int fputc(int ch, FILE *f)
{
		HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,0xffff);
		return ch;
	}	*/


void activateZone(int zone) {
	
		if(zones[zone].mode==ZONE_MODE_DELAY) {
			if(currLightLevel<sData.lightLevel){
				zones[zone].zoneActive=ZONE_ACTIVE;
				zones[zone].zoneTimeout=systemTick+lightTimer;
				
			}
			else
				zones[zone].zoneActive=0;
		}
		else {
				if(zones[zone].zoneActive)
					zones[zone].zoneActive=0;
				else
					zones[zone].zoneActive=1;
				
		}
		zones[zone].btnActivated=0;
		zones[zone].pirActivated=0;
		zones[zone].espActivated=0;
}
	void espUpdate()
	{
		
		for(int x=0;x<4;x++) {
			zones[x].EspInput=HAL_GPIO_ReadPin(Pass_Relay_array[x].port,Pass_Relay_array[x].mask);
			
				if(zones[x].EspInput)
					zones[x].EspOverride=1;
				else
					zones[x].EspOverride=0;
			
					
		}
	}
	
	void SaveLightLevel(void) {
		
				//BTN has been pushed so transfer current light level to ROM
				saveLightLevel=1;
				HAL_ADC_Start_IT(&hadc);
					
	}
	
	
	
/*	void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
		//HAL_ADC_Stop(hadc);
		currLightLevel=HAL_ADC_GetValue(hadc);
		if(saveLightLevel){
				sData.lightLevel=currLightLevel;
				needSaving=1;
				saveLightLevel=0;
		}
		
	}*/

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
		
	tick_type rfDisableTimeout;
	tick_type secTick=TICKS_PER_SECOND;
	tick_type adcTimer = TICKS_PER_SECOND *10;
	int Zone;
	int index;
	
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim3); //start timer 3
	__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE );  //must enable the timer 3 overflow for the interrupt to be generated
	needSaving=0;
	HAL_ADC_Start_IT(&hadc);
	buttonInit();
	RFInit();
	pData=&sData;
	
	HAL_ADC_Start(&hadc);
	
	for(int x=0;x<16;x++)
		sData.rfCodeArray[x]=0;
	for(int x=0;x<NUM_ZONES;x++){
		zones[x].zoneTimeout=0;
		zones[x].zoneActive=ZONE_INACTIVE;
		zones[x].ledActive=ZONE_INACTIVE;
		zones[x].zoneLedTimeout=0;
		zones[x].btnActivated=0;
		zones[x].pirActivated=0;
		zones[x].espActivated=0;
		HAL_GPIO_WritePin(Relay_array[x].port, Relay_array[x].mask,0);
		HAL_GPIO_WritePin(Key_array[x].port, Key_array[x].mask,1);
	}
	
	zones[0].mode=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15);
	zones[1].mode=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14);
	zones[2].mode=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13);
	zones[3].mode=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12);
	
	flashRead();
	if(sData.lightLevel==0xffffffff)
		sData.lightLevel=0x3ff; //max value
	
	if(sData.lightOnDelay==0xffffffff || sData.lightOnDelay==0){
		//read the on board switches for delay in minutes
		lightTimer=1;
		lightTimer^=HAL_GPIO_ReadPin(SW8_GPIO_Port,SW8_Pin);
		lightTimer=(lightTimer<<1)+1;
		lightTimer^=HAL_GPIO_ReadPin(SW7_GPIO_Port,SW7_Pin);
		lightTimer=(lightTimer<<1)+1;
		lightTimer^=HAL_GPIO_ReadPin(SW6_GPIO_Port,SW6_Pin);
		lightTimer=(lightTimer<<1)+1;
		lightTimer^=HAL_GPIO_ReadPin(SW5_GPIO_Port,SW5_Pin);
		lightTimer=(lightTimer<<1)+1;
		if(lightTimer==1)
			lightTimer=TICKS_PER_SECOND*5;
		else
			lightTimer=TICKS_PER_SECOND*60*(lightTimer);
	}
	else
		lightTimer=sData.lightLevel;
	
	
	#ifdef __DEBUG_ENABLE
		printf("\fSonoff CH4 - RF enabled security lighting system (c) Nick Deacon 2020\n\r");
	#endif
	saveLightLevel=0;
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(systemTick>secTick) {
			for(int x=0;x<4;x++) {
				if(zones[x].ledActive) {
					HAL_GPIO_TogglePin(led_array[x].port,led_array[x].mask);
				}
			}
			secTick=systemTick+TICKS_PER_SECOND/2; 
		}
		if(systemTick>adcTimer) {
			currLightLevel=HAL_ADC_GetValue(&hadc);
			if(saveLightLevel){
				sData.lightLevel=currLightLevel;
				needSaving=1;
				saveLightLevel=0;
			}
			HAL_ADC_Start(&hadc);
			adcTimer=systemTick+(TICKS_PER_SECOND*10);
		}
		buttonTask();
		rfReceiveCode= RFTask();
		if(rfReceiveCode>0) {
			#ifdef __DEBUG_ENABLE
				printf("rf code received %X\n\r",rfReceiveCode);
			#endif
			RFInit();
			switch(rfMode){
				case RF_MODE_NORMAL:
					
					for(Zone=0;Zone<4;Zone++){
						if(findCodeZone(rfReceiveCode,Zone)>0) {
							if(!zones[Zone].ledActive) {
								zones[Zone].pirActivated=ZONE_ACTIVE;
								HAL_GPIO_WritePin(led_array[Zone].port,led_array[Zone].mask,1);
								zones[Zone].ledActive=1;
								zones[Zone].zoneLedTimeout=systemTick+TICKS_PER_SECOND/2;
								
							}
							
							#ifdef __DEBUG_ENABLE
								printf("button %i pressed\r\n",Zone);
							#endif
							
						}
					}
					break;
				case RF_MODE_LEARN:
						rfCodeArray[receivedCodes]=rfReceiveCode;
						receivedCodes++;
						if(receivedCodes==3){
							
							if(rfCodeArray[0]==rfCodeArray[1]&& rfCodeArray[0]==rfCodeArray[2]){
								//store new code
								if(!findCodeZone(rfCodeArray[0],currZone)) { //check that the code is already stored
									index=findFirstSlot(currZone);
									if(index>=0) {
										#ifdef __DEBUG_ENABLE
											printf("code saving to zone %i\n\r",currZone);
										#endif
										sData.rfCodeArray[index]=rfCodeArray[0];
										needSaving=1;
										HAL_GPIO_WritePin(led_array[currZone].port,led_array[currZone].mask,1);
										zones[currZone].ledActive=1;
										zones[currZone].zoneLedTimeout=systemTick+TICKS_PER_SECOND*2;
										//disable further rf for 5 sec
										rfMode=RF_MODE_DISABLE;
										rfDisableTimeout=systemTick+(TICKS_PER_SECOND*2);
										receivedCodes=0;
										rfCodeArray[0]=rfCodeArray[1]=rfCodeArray[2]=0;
									}
								receivedCodes=0;
								
								}
							}
							else {
								rfCodeArray[0]=rfCodeArray[1];
								rfCodeArray[1]=rfCodeArray[2];
								receivedCodes=2;
								
							}
							
						}
						
						break;
				}
		}
		if(rfMode==RF_MODE_LEARN && systemTick>rfTimeout){
				rfMode=RF_MODE_NORMAL; //timeout return to normal mode
				receivedCodes=0;
		}
		int rfStatus=rfGetStatus();
		if(rfStatus!=RF_IDLE) {
			
			//set outputs to relays
			
			for(int x=0;x<NUM_ZONES;x++) {
				
				if(zones[x].btnActivated==ZONE_ACTIVE || zones[x].pirActivated==ZONE_ACTIVE || zones[x].espActivated==ZONE_ACTIVE) {
					activateZone(x);
				}
												
				if(zones[x].ledActive) { 
					if(systemTick>zones[x].zoneLedTimeout){
						HAL_GPIO_WritePin(led_array[x].port,led_array[x].mask,0);
						zones[x].ledActive=0;
					}
				}
				if(zones[x].zoneActive && zones[x].mode==ZONE_MODE_DELAY){
					if(systemTick>zones[x].zoneTimeout) {
						zones[x].zoneActive=0;
						
					}
				}
				if(!zones[x].EspOverride)
					HAL_GPIO_WritePin(Relay_array[x].port, Relay_array[x].mask,zones[x].zoneActive);
				else
					HAL_GPIO_WritePin(Relay_array[x].port, Relay_array[x].mask,1);
			}			
			if(rfMode==RF_MODE_DISABLE) { 
				if(systemTick>rfDisableTimeout) {
					rfMode=RF_MODE_NORMAL;
				}
			}
			
			else {
				if(needSaving) {
					__disable_irq();
					flashWrite();
					__enable_irq();
					needSaving=0;
				}	
				else {
					//lowest priority task - update to and from the esp wifi chip
					espUpdate();
					//echo button S6 to KEY1 output to esp. this allows for long press to put esp into link mode
					HAL_GPIO_WritePin(Key_array[0].port,Key_array[0].mask,HAL_GPIO_ReadPin(BTN_GPIO_Port,BTN_Pin));
				}
			}
		}
		else
			rfSetStatus(RF_LISTEN);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_8;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1200-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim3.Init.Period = 1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RELAY_K4_Pin|RELAY_K1_Pin|RFLED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RFLED4_Pin|RFLED3_Pin|RFLED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, KEY3_Pin|KEY2_Pin|KEY1_Pin|KEY4_Pin 
                          |RELAY_K2_Pin|RELAY_K3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RELAY_K4_Pin RELAY_K1_Pin RFLED1_Pin */
  GPIO_InitStruct.Pin = RELAY_K4_Pin|RELAY_K1_Pin|RFLED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PASS_RELAY4_Pin PASS_RELAY2_Pin */
  GPIO_InitStruct.Pin = PASS_RELAY4_Pin|PASS_RELAY2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : RFLED4_Pin RFLED3_Pin RFLED2_Pin */
  GPIO_InitStruct.Pin = RFLED4_Pin|RFLED3_Pin|RFLED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : KEY_S1_Pin KEY_S4_Pin KEY_S2_Pin KEY_S3_Pin 
                           LOCK_Pin BTN_Pin PASS_RELAY1_Pin */
  GPIO_InitStruct.Pin = KEY_S1_Pin|KEY_S4_Pin|KEY_S2_Pin|KEY_S3_Pin 
                          |LOCK_Pin|BTN_Pin|PASS_RELAY1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SW8_Pin SW7_Pin SW6_Pin SW5_Pin 
                           SW4_Pin SW3_Pin SW2_Pin SW1_Pin 
                           PASS_RELAY3_Pin */
  GPIO_InitStruct.Pin = SW8_Pin|SW7_Pin|SW6_Pin|SW5_Pin 
                          |SW4_Pin|SW3_Pin|SW2_Pin|SW1_Pin 
                          |PASS_RELAY3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DIN_Pin */
  GPIO_InitStruct.Pin = DIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : KEY3_Pin KEY2_Pin KEY1_Pin KEY4_Pin 
                           RELAY_K2_Pin RELAY_K3_Pin */
  GPIO_InitStruct.Pin = KEY3_Pin|KEY2_Pin|KEY1_Pin|KEY4_Pin 
                          |RELAY_K2_Pin|RELAY_K3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
