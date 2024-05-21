#ifndef __PROC_H
#define __PROC_H

#define RCK_H() HAL_GPIO_WritePin(OUT_RCK_GPIO_Port, OUT_RCK_Pin, GPIO_PIN_SET)
#define RCK_L() HAL_GPIO_WritePin(OUT_RCK_GPIO_Port, OUT_RCK_Pin, GPIO_PIN_RESET)

#define PB5_H() HAL_GPIO_WritePin(OVERHEAT_GPIO_Port, OVERHEAT_Pin, GPIO_PIN_SET)
#define PB5_L() HAL_GPIO_WritePin(OVERHEAT_GPIO_Port, OVERHEAT_Pin, GPIO_PIN_RESET)

void checkSensor(void);
void CO2_check(uint16_t spCO20, uint16_t spCO21, uint16_t pvCO20);
void aeration_check(uint8_t air0, uint8_t air1);
uint16_t adcTomV(uint16_t curr);
uint8_t statF2(uint8_t n, uint16_t statPw);
void sysTick_Init(void);
void beeper_ON(uint16_t duration);
void set_Output(void);          // запись в микросхему 74HC595D
void set_Pulse(uint16_t val);     // вспомогательный импульс
#endif /* __PROC_H */

