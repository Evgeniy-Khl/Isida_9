/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "TM1638.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define FUSE2_Pin GPIO_PIN_0
#define FUSE2_GPIO_Port GPIOA
#define FUSE3_Pin GPIO_PIN_1
#define FUSE3_GPIO_Port GPIOA
#define FUSE4_Pin GPIO_PIN_2
#define FUSE4_GPIO_Port GPIOA
#define FUSE5_Pin GPIO_PIN_3
#define FUSE5_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_4
#define SD_CS_GPIO_Port GPIOA
#define SD_SCK_Pin GPIO_PIN_5
#define SD_SCK_GPIO_Port GPIOA
#define SD_MISO_Pin GPIO_PIN_6
#define SD_MISO_GPIO_Port GPIOA
#define SD_MOSI_Pin GPIO_PIN_7
#define SD_MOSI_GPIO_Port GPIOA
#define ADC_CURR_Pin GPIO_PIN_0
#define ADC_CURR_GPIO_Port GPIOB
#define ADC_HUM_Pin GPIO_PIN_1
#define ADC_HUM_GPIO_Port GPIOB
#define OUT_RCK_Pin GPIO_PIN_12
#define OUT_RCK_GPIO_Port GPIOB
#define DISPL_STB_Pin GPIO_PIN_14
#define DISPL_STB_GPIO_Port GPIOB
#define Bluetooth_STATE_Pin GPIO_PIN_8
#define Bluetooth_STATE_GPIO_Port GPIOA
#define Blutooth_TX_Pin GPIO_PIN_9
#define Blutooth_TX_GPIO_Port GPIOA
#define Blutooth_RX_Pin GPIO_PIN_10
#define Blutooth_RX_GPIO_Port GPIOA
#define DE485_Pin GPIO_PIN_11
#define DE485_GPIO_Port GPIOA
#define AM2301_Pin GPIO_PIN_12
#define AM2301_GPIO_Port GPIOA
#define Beeper_Pin GPIO_PIN_15
#define Beeper_GPIO_Port GPIOA
#define Door_Pin GPIO_PIN_3
#define Door_GPIO_Port GPIOB
#define OVERHEAT_Pin GPIO_PIN_4
#define OVERHEAT_GPIO_Port GPIOB
#define cmdH_05_Pin GPIO_PIN_5
#define cmdH_05_GPIO_Port GPIOB
#define mem_SCL_Pin GPIO_PIN_6
#define mem_SCL_GPIO_Port GPIOB
#define mem_SDA_Pin GPIO_PIN_7
#define mem_SDA_GPIO_Port GPIOB
#define OneW_2R_Pin GPIO_PIN_8
#define OneW_2R_GPIO_Port GPIOB
#define OneWR_1_Pin GPIO_PIN_9
#define OneWR_1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define VERSION       1       // версия ПО
#define EEPROM_I2C_PORT hi2c1
#define EEP_DATA      50      // ВСЕГО 50 bytes
#define MAX_DEVICES   4       // максимальное количество микросхем DS18b20 подключенных 1 Wire шине
#define WAITCOUNT     16      // максимальная пауза перед реакцией на кнопку
#define MAXOWNER 	    12      // максимальная количество меню для пользователя
#define BOTUSER 	    16      // начальный пункт меню для специалиста
#define TOPUSER 	    18      // максимальная количество меню для специалиста кроме ШИМ
#define PULSMENU 	    21      // максимальная количество меню для специалиста включая ШИМ
#define BOTKOFF 	    26      // начальный пункт меню коэфициентов
#define TOPKOFF 	    31      // конечный пункт меню коэфициентов

#define ON          1
#define OFF         0
#define UNCHANGED   2
#define FLAPOPEN    100     // значение при котором зслонка полностью открыта
#define FLAPCLOSE   0       // значение при котором зслонка полностью закрыта
#define DATAREAD    0xA1    // Read Scratchpad
#define SETFLAP     0xA2    // аварийное полное открытие заслонки
#define SETHORIZON  0xA3    // команда установки в горизонт
#define NEW_TURN    0xA5    // команда установки проверки прохода поворота
#define TUNING      170
#define SENSOREGG   171
#define DURATION    100
#define COUNT       500
#define MAXPULS     1100    // 1.1 сек.

#define ID_HALL         0xF1    // идентификатор блока
#define ID_HORIZON      0xF3    // идентификатор блока
#define ID_CO2          0xF5    // идентификатор блока
#define ID_FLAP         0xF7    // идентификатор блока
#define RESETCMD        0xC1    // Generate Reset Pulse

//#define CANAL1_ON(x)  x |= 1<<0  // симистор НАГРЕВАТЕЛЯ
//#define CANAL1_OFF(x) x &= ~(1<<0)
//#define CANAL1_IN(x)  x &= (1<<0)
//#define CANAL2_ON(x)  x |= 1<<1  // реле или симистор УВЛАЖНЕНИЯ
//#define CANAL2_OFF(x) x &= ~(1<<1)
//#define CANAL2_IN(x)  x &= (1<<1)
//#define EXT1_ON(x)    x |= 1<<2  // реле вспомогательного канала
//#define EXT1_OFF(x)   x &= ~(1<<2)
//#define EXT2_ON(x)    x |= 1<<3  // реле вспомогательного канала
//#define EXT2_OFF(x)   x &= ~(1<<3)
//#define TURN_ON(x)    x |= 1<<4  // поворот
//#define TURN_OFF(x)   x &= ~(1<<4)
//#define TURN_IN(x)    x &= (1<<4)
//#define COOLER_ON(x)  x |= 1<<5  // вентилятор охладителя
//#define COOLER_OFF(x) x &= ~(1<<5)  // вентилятор охладителя
//#define ALL_OFF(x)    x &= ~((1<<0)|(1<<1)|(1<<2)|(1<<3))

#define SIMBL_A     20
#define SIMBL_B     21
#define SIMBL_C     22
#define SIMBL_d     23
#define SIMBL_E     24
#define SIMBL_F     25
#define SIMBL_H     26
#define SIMBL_t     27
#define SIMBL_P     28
#define SIMBL_TOPo  29
#define SIMBL_Pe    30
#define SIMBL_TOPn  31
#define SIMBL_n     32
#define SIMBL_o     33
#define SIMBL_c     34
#define SIMBL_TOPu  35
#define SIMBL_u     36
#define SIMBL_MINUS 37
#define SIMBL_M_Top 38
#define SIMBL_MBott 39
#define SIMBL_T_M_B 40
#define SIMBL_BL    41

#define DISPL_A     42
#define DISPL_B     43
#define DISPL_C     44
#define DISPL_d     45
#define DISPL_E     46
#define DISPL_F     47
#define DISPL_H     48
#define DISPL_t     49
#define DISPL_P     50
#define DISPL_TOPo  51
#define DISPL_Pe    52
#define DISPL_TOPn  53
#define DISPL_n     54
#define DISPL_o     55
#define DISPL_c     56
#define DISPL_TOPu  57
#define DISPL_u     58
#define DISPL_MINUS 59
#define DISPL_M_Top 60
#define DISPL_MBott 61
#define DISPL_T_M_B 62
#define DISPL_BL    63

#define DEN           20  // denominator - делитель для вывода на дисплей в секундах 0,0
#define MINRELAYMODE  0   // релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->ШИ по кан.[1]
#define MAXRELAYMODE  4
/* ---структура с битовыми полями -----*/
struct byte {
    unsigned a0: 1;
    unsigned a1: 1;
    unsigned a2: 1;
    unsigned a3: 1;
    unsigned a4: 1;
    unsigned a5: 1;
    unsigned a6: 1;
    unsigned a7: 1;
};
 
union Byte {
    unsigned char value;
    struct byte bitfield;
};

#define CHECK   portFlag.bitfield.a0  // Start of all checks
#define ALARM   portFlag.bitfield.a1  // Alarm flag
#define VENTIL 	portFlag.bitfield.a2  // Ventilation flag
#define EEPSAVE portFlag.bitfield.a3  // Save in EEPROM flag
#define HIH5030	portFlag.bitfield.a4  // exist HIH5030 flag
#define AM2301	portFlag.bitfield.a5  // exist AM2301 flag
#define LEDOFF  portFlag.bitfield.a6  // 
#define DISPLAY portFlag.bitfield.a7  // 

#define HEATER  portOut.bitfield.a0  // НАГРЕВАТЕЛЬ
#define HUMIDI	portOut.bitfield.a1  // УВЛАЖНИТЕЛЬ
#define FLAP		portOut.bitfield.a2  // Заслонка воздухообмена
#define EXT2		portOut.bitfield.a3  // Вспомогательный канал
#define TURN		portOut.bitfield.a4  // Поворот лотков

//typedef struct
//{
//  uint8_t Hours;            /*!< Specifies the RTC Time Hour. This parameter must be a number between Min_Data = 0 and Max_Data = 23 */
//  uint8_t Minutes;          /*!< Specifies the RTC Time Minutes. This parameter must be a number between Min_Data = 0 and Max_Data = 59 */
//  uint8_t Seconds;          /*!< Specifies the RTC Time Seconds. This parameter must be a number between Min_Data = 0 and Max_Data = 59 */
//} MY_TimeTypeDef;

//typedef struct
//{
//  uint8_t WeekDay;  /*!< Specifies the RTC Date WeekDay (not necessary for HAL_RTC_SetDate). This parameter can be a value of @ref RTC_WeekDay_Definitions */
//  uint8_t Month;    /*!< Specifies the RTC Date Month (in BCD format). This parameter can be a value of @ref RTC_Month_Date_Definitions */
//  uint8_t Date;     /*!< Specifies the RTC Date. This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
//  uint8_t Year;     /*!< Specifies the RTC Date Year.  This parameter must be a number between Min_Data = 0 and Max_Data = 99 */
//} MY_DateTypeDef;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
