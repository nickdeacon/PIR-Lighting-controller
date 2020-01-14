/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef uint32_t  counter_type;
typedef uint16_t RFindex;
typedef uint32_t RFvalue;
typedef uint64_t tick_type;

typedef struct setupData {
	uint32_t lightOnDelay;
	uint32_t lightLevel;
	uint32_t rfCodeArray[16];
	}setupData;

typedef struct ZONES {
	tick_type zoneTimeout;
	tick_type zoneLedTimeout;
	uint16_t  zoneActive:1;
	uint16_t  ledActive:1;
	uint16_t	btnActivated:1;
	uint16_t	pirActivated:1;
	uint16_t 	espActivated:1;
	uint16_t 	mode:1;
	uint16_t 	EspInput:1;
	uint16_t 	lastEspInput:1;
	uint16_t	EspOverride:1;
	uint16_t 	UNUSED:7;
	uint32_t 	*zoneRelayPort;
	uint32_t 	zoneRelayPin;
	uint32_t 	*zoneLEDPort;
	uint32_t 	zoneLEDPin;
	
}ZONES;
typedef struct {
	GPIO_TypeDef *port;
	uint32_t mask;
} PIN_TypeDef;

static const PIN_TypeDef Key_array[]={
	{GPIOB, 1<<6},
	{GPIOB, 1<<5},
	{GPIOB, 1<<4},
	{GPIOB, 1<<7}	
};


static const PIN_TypeDef Relay_array[]={
	{GPIOC, 1<<14},
	{GPIOB, 1<<8},
	{GPIOB, 1<<9},
	{GPIOC, 1<<13}	
};

static const PIN_TypeDef Button_array[]={
	{GPIOA, 1<<3},
	{GPIOA, 1<<5},
	{GPIOA, 1<<6},
	{GPIOA, 1<<4},
	{GPIOA, 1<<11}
};
static const PIN_TypeDef Pass_Relay_array[]={
	{GPIOA, 1<<15},
	{GPIOF, 1<<1},
	{GPIOF, 1<<0},
	{GPIOB, 1<<3}
};

static const PIN_TypeDef led_array[]={
	{GPIOC, 1<<15},
	{GPIOA, 1<<2},
	{GPIOA, 1<<1},
	{GPIOA, 1<<0}
};

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void buttonInit(void);
void RFInit(void);
int RFTask(void);
void SaveLightLevel(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RELAY_K4_Pin GPIO_PIN_13
#define RELAY_K4_GPIO_Port GPIOC
#define RELAY_K1_Pin GPIO_PIN_14
#define RELAY_K1_GPIO_Port GPIOC
#define RFLED1_Pin GPIO_PIN_15
#define RFLED1_GPIO_Port GPIOC
#define PASS_RELAY4_Pin GPIO_PIN_0
#define PASS_RELAY4_GPIO_Port GPIOF
#define PASS_RELAY2_Pin GPIO_PIN_1
#define PASS_RELAY2_GPIO_Port GPIOF
#define RFLED4_Pin GPIO_PIN_0
#define RFLED4_GPIO_Port GPIOA
#define RFLED3_Pin GPIO_PIN_1
#define RFLED3_GPIO_Port GPIOA
#define RFLED2_Pin GPIO_PIN_2
#define RFLED2_GPIO_Port GPIOA
#define KEY_S1_Pin GPIO_PIN_3
#define KEY_S1_GPIO_Port GPIOA
#define KEY_S4_Pin GPIO_PIN_4
#define KEY_S4_GPIO_Port GPIOA
#define KEY_S2_Pin GPIO_PIN_5
#define KEY_S2_GPIO_Port GPIOA
#define KEY_S3_Pin GPIO_PIN_6
#define KEY_S3_GPIO_Port GPIOA
#define LIGHT_Pin GPIO_PIN_7
#define LIGHT_GPIO_Port GPIOA
#define SW8_Pin GPIO_PIN_1
#define SW8_GPIO_Port GPIOB
#define SW7_Pin GPIO_PIN_2
#define SW7_GPIO_Port GPIOB
#define SW6_Pin GPIO_PIN_10
#define SW6_GPIO_Port GPIOB
#define SW5_Pin GPIO_PIN_11
#define SW5_GPIO_Port GPIOB
#define SW4_Pin GPIO_PIN_12
#define SW4_GPIO_Port GPIOB
#define SW3_Pin GPIO_PIN_13
#define SW3_GPIO_Port GPIOB
#define SW2_Pin GPIO_PIN_14
#define SW2_GPIO_Port GPIOB
#define SW1_Pin GPIO_PIN_15
#define SW1_GPIO_Port GPIOB
#define LOCK_Pin GPIO_PIN_8
#define LOCK_GPIO_Port GPIOA
#define BTN_Pin GPIO_PIN_11
#define BTN_GPIO_Port GPIOA
#define DIN_Pin GPIO_PIN_12
#define DIN_GPIO_Port GPIOA
#define DIN_EXTI_IRQn EXTI4_15_IRQn
#define PASS_RELAY1_Pin GPIO_PIN_15
#define PASS_RELAY1_GPIO_Port GPIOA
#define PASS_RELAY3_Pin GPIO_PIN_3
#define PASS_RELAY3_GPIO_Port GPIOB
#define KEY3_Pin GPIO_PIN_4
#define KEY3_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_5
#define KEY2_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_6
#define KEY1_GPIO_Port GPIOB
#define KEY4_Pin GPIO_PIN_7
#define KEY4_GPIO_Port GPIOB
#define RELAY_K2_Pin GPIO_PIN_8
#define RELAY_K2_GPIO_Port GPIOB
#define RELAY_K3_Pin GPIO_PIN_9
#define RELAY_K3_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define NUM_ZONES 4
#define ZONE_MODE_DELAY 0
#define ZONE_ACTIVE 1
#define ZONE_INACTIVE 0
#define RF_MODE_NORMAL 0
#define RF_MODE_LEARN 1
#define RF_MODE_DISABLE 2
#define RF_IDLE 0
#define RF_LISTEN 1
#define RF_LISTEN_FOR_EDGE 2
#define RF_HAS_BIT 3
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
