#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv

extern uint8_t ds18b20_amount;

//--------------------------------------------------
__STATIC_INLINE void DelayMicro(__IO uint32_t micros){
  micros *= (SystemCoreClock / 1000000) / 8;
  while (micros--) ;  // Wait till done
}
//--------------------------------------------------
void am2301_port_init(void){
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);
  GPIOA->CRH |= GPIO_CRH_MODE12;        // вывод порта с номерами 12
  GPIOA->CRH |= GPIO_CRH_CNF12_0;       // выход с открытым коллектором
  GPIOA->CRH &= ~GPIO_CRH_CNF12_1;
}
//-----------------------------------------------
uint8_t am2301_Start(void){
  uint8_t status;
  GPIOA->BSRR = GPIO_BSRR_BR12;//низкий уровень (Сбросили 12 бит порта A )
  DelayMicro(5000);//задержка как минимум на 800 микросекунд
  GPIOA->BSRR = GPIO_BSRR_BS12;//высокий уровень (Установили 12 бит порта A )
  DelayMicro(60);//задержка на 60 микросекунд (wait for DHT respond 20-40uS)
  status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);//проверяем уровень (low-voltage-level response signal & keeps it for 80us)
  if(status) return 0;// status==1 датчик не ответил низким уровнем
  DelayMicro(80);//задержка на 80 микросекунд
  status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);//проверяем уровень (hi-voltage-level response signal & keeps it for 80us)
  return status;//вернём результат. status==0 если линия осталась в низком уровне. Иначе осталось 46 us до низкого уровня.
}
//--------------------------------------------------
uint8_t am2301_Read(struct rampv *ram, int8_t biasHum){
  uint8_t i, j, status=0, crc, tem[5];
  int16_t result;
  static uint8_t err;
  if(am2301_Start()){
    DelayMicro(60);// до низкого уровня осталось 46 us. Задержка на 60 микросекунд
    for(i=0; i<5; i++){
      tem[i]=0;
      for(j=0; j<8; j++){
        tem[i]<<= 1;
        status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);//проверяем уровень
        while(!status) {status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);}// ожидаем (hi-voltage-level)
        crc = 0;
        while(status) {
          DelayMicro(1); crc++;
          status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);
        }// ожидаем (low-voltage-level)
        
        if(crc > 23) tem[i]|= 1;  // data "1" 12<crc<33
      }
    }
    crc=tem[0]+tem[1]+tem[2]+tem[3];
    if(crc==tem[4]){
      result =((int)tem[0]*256+tem[1] + biasHum)/10;            // коррекция датчика влажности
      if(ds18b20_amount<3) ram->pvT[2] =(int)tem[2]*256+tem[3]; // DHT21
      if(result > 100) result=100; else if(result < 0) result=0;
      ram->pvRH = result;
      err = 0;
      status=1;
    }
    else if(++err>3) {status=0; ram->pvRH = 150; ram->errors |= 0x02;}
  }
  else {ram->pvRH = 199; ram->pvT[ds18b20_amount] = 1999; ram->errors |= 0x02;}
  return status;
}
//-----------------------------------------------
