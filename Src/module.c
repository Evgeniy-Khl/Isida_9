#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv
#include "module.h"
#include "ds18b20.h"
#include "proc.h"

uint8_t outbuffer[4], inbuffer[4], cmdmodule;
//--------------------------------------------------
__STATIC_INLINE void DelayMicro(__IO uint32_t micros){
  micros *= (SystemCoreClock / 1000000) / 8;
  while (micros--) ;  // Wait till done
}
//--------------------------------------------------
void module_port_init(void){
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);
  GPIOB->CRH |= GPIO_CRH_MODE8;
  GPIOB->CRH |= GPIO_CRH_CNF8_0;
  GPIOB->CRH &= ~GPIO_CRH_CNF8_1;
}
//-----------------------------------------------
uint8_t module_Reset(void){
  uint8_t status;
  GPIOB->BSRR =GPIO_BSRR_BR8;//низкий уровень (Сбросили 8 бит порта B )
  DelayMicro(485);//задержка как минимум на 480 микросекунд
  GPIOB->BSRR =GPIO_BSRR_BS8;//высокий уровень (Установили 8 бит порта B )
  DelayMicro(65);//задержка как минимум на 60 микросекунд
  status = (GPIOB->IDR & GPIO_IDR_IDR8 ? 1 : 0);//проверяем уровень
  DelayMicro(500);//задержка как минимум на 480 микросекунд
  //(на всякий случай подождём побольше, так как могут быть неточности в задержке)
  return (status ? 1 : 0);//вернём результат
}
//--------------------------------------------------
uint8_t module_ReadBit(void){
  uint8_t bit = 0;
  GPIOB->BSRR =GPIO_BSRR_BR8;//низкий уровень
  DelayMicro(2);
  GPIOB->BSRR =GPIO_BSRR_BS8;//высокий уровень
  DelayMicro(13);
  bit = (GPIOB->IDR & GPIO_IDR_IDR8 ? 1 : 0);//проверяем уровень
  DelayMicro(45);
  return bit;
}
//-----------------------------------------------
uint8_t module_ReadByte(void){
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++)
  data += module_ReadBit() << i;
  return data;
}
//-----------------------------------------------
void module_WriteBit(uint8_t bit){
  GPIOB->BSRR =GPIO_BSRR_BR8;//низкий уровень
  DelayMicro(bit ? 3 : 65);
  GPIOB->BSRR =GPIO_BSRR_BS8;//высокий уровень
  DelayMicro(bit ? 65 : 3);
}
//-----------------------------------------------
void module_WriteByte(uint8_t dt){
  for (uint8_t i = 0; i < 8; i++){
    module_WriteBit(dt >> i & 1);
    //Delay Protection
    DelayMicro(5);
  }
}
//--------------------------------------------------
int8_t module_check(uint8_t fc){
 int8_t i, byte;
  outbuffer[3] = dallas_crc8(outbuffer, 3);// контрольная сумма для ПЕРВЫХ 3 байт
  for (i=0; i<5; i++)
   {
    module_Reset();             // 1 Wire Bus initialization
    module_WriteByte(fc);      // Load Family code
    for (byte=0; byte<4; byte++) module_WriteByte(outbuffer[byte]);
    DelayMicro(COUNT);          // ожидаем пока модуль обработает информацию
    for (byte=0; byte<4; byte++) inbuffer[byte] = module_ReadByte();// Read 4 byte
    byte = module_ReadByte();  // Read CRC byte #5
    if(byte == dallas_crc8(inbuffer,4)) break;
    GPIOB->BSRR =GPIO_BSRR_BR8;//низкий уровень (Сбросили 8 бит порта B)
    DelayMicro(200);            //задержка как минимум на 200 микросекунд
    GPIOB->BSRR =GPIO_BSRR_BS8;//высокий уровень (Установили 8 бит порта B)
    DelayMicro(800);            //задержка как минимум на 800 микросекунд
   };
  if(i>4) byte=0; else byte=1;
  return byte;
}

int8_t chkcooler(uint8_t state){ // проверка вращения тихоходного вентилятора
 int8_t byte;
 static uint8_t countTry;
  outbuffer[0]=DATAREAD;        // Function Command
  outbuffer[1]=0x00;            // Data 1
  outbuffer[2]=0x00;            // Data 2
  byte = module_check(ID_HALL); // идентификатор блока
  if(byte)                      // если блок ответил ...
   {
     byte = inbuffer[0];
     countTry = 0;
     if((state&0x41)==0x41){    // если ВКЛЮЧЕН мониторинг тихоходного вентилятора
       if(byte<1) byte = 1;     // слабое вращение вентилятора
      }
   }
  else if(++countTry>5) byte = -1; // Отказ модуля Холла
  return byte;
}

int8_t chkhorizon(uint8_t state){ // проверка прохода через горизонт
 int8_t byte;
 static uint8_t countTry;
  outbuffer[0]=DATAREAD;          // Function Command
  outbuffer[1]=cmdmodule;         // Data 1
  if(state&0x20) outbuffer[2]=1;  // Data 2 ВКЛЮЧЕН мониторинг поворота лотков
  else outbuffer[2]=0;            // Data 2 ОТКЛЮЧЕН мониторинг поворота лотков
  byte = module_check(ID_HORIZON);// идентификатор блока
  if(byte){                        // если блок ответил ...
     byte = 0;//inbuffer[0]; ???????????????????????????????????????????????????
     countTry = 0;
     switch(cmdmodule){
         case NEW_TURN:
            if(inbuffer[1]==2){                             // 2-> поворот НЕ прошел
               cmdmodule=0;
               if((state&0x21)==0x21) byte = 1;             // если ВКЛЮЧЕН камера и мониторинг поворота лотков
               if((state&0xA0)==0xA0) beeper_ON(DURATION);  // если ВКЛЮЧЕН ТОЛЬКО ПОВОРОТ ЛОТКОВ
            } 
            else if(inbuffer[1]==3) {cmdmodule=0;}          // 3-> поворот прошел
            break;
         case SETHORIZON:
            if((state&0x18)==0) cmdmodule=0;                // отмена установки в горизонт
            break;
         default: cmdmodule=0;
     }
  }
  else if(++countTry>5) byte = -1; // Отказ модуля ГОРИЗОНТ
  return byte;
}

int8_t readCO2(struct rampv *ram){ // чтение модуля СО2
 int8_t byte;
 static uint8_t countTry;
  outbuffer[0]=DATAREAD;       // Function Command
  outbuffer[1]=0x00;           // Data 1
  outbuffer[2]=0x00;           // Data 2
  byte = module_check(ID_CO2); // идентификатор блока
  if(byte)                     // если блок ответил ...
   {
     byte = inbuffer[0];       // колич. обслуж. камер(0x30) + номер камеры 
     byte = inbuffer[0]&0x03;  // номер камеры
     //inbuffer[1]; не используется
     ram->pvCO2[byte] = inbuffer[2]+inbuffer[3]*256;// значение CO2
     countTry = 0;
   }
  else if(++countTry>5) {byte = -1; for (byte=0;byte<3;byte++) ram->pvCO2[byte]=0;}// Отказ модуля CO2
  return byte;
}

int8_t chkflap(uint8_t cmd, uint8_t *pvF){ // проверка положения воздушной заслонки
 int8_t byte;
 static uint8_t countTry;
  outbuffer[0]=cmd;             // Function Command
  outbuffer[1]=*pvF;            // Data 1
  outbuffer[2]=0x00;            // Data 2
  byte = module_check(ID_FLAP); // идентификатор блока
  if(byte)                      // если блок ответил ...
   {
     *pvF = inbuffer[0];
     countTry = 0;
   }
  else if(++countTry>5) {byte = -1; *pvF = 0;} // Отказ дополнительного модуля
  return byte;
}
