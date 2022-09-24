/*
 * TM1638.h
*/

#ifndef __TM1638_H
#define __TM1638_H

#include "main.h"

#define STB_H() HAL_GPIO_WritePin(DISPL_STB_GPIO_Port, DISPL_STB_Pin, GPIO_PIN_SET)
#define STB_L() HAL_GPIO_WritePin(DISPL_STB_GPIO_Port, DISPL_STB_Pin, GPIO_PIN_RESET)

void SendCmdTM1638(uint8_t cmd);
void SendDataTM1638(void);
void ReadKeyTM1638(void);
void LedOn(uint8_t pos, uint8_t led);
void LedInverse(uint8_t pos, uint8_t led);
void LedOff(uint8_t pos, uint8_t led);
void AllLedOff(void);
void PointOn(uint8_t pos);
void PointInverse(uint8_t pos);
void PointOff(uint8_t pos);

void setChar(uint8_t pos, uint8_t nuber);

#endif /* __TM1638_H */
