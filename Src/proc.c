#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv
#include "proc.h"
#include "module.h"


const float A4=1.8, A5=0.81, A6=0.01;  // порядок a=0.9 (A1=2a; A2=a^2; A3=(1-a)^2)
const float A1=1.6, A2=0.64, A3=0.04;  // порядок a=0.8 (A1=2a; A2=a^2; A3=(1-a)^2)
const float VREF = 3.3f;

uint8_t pvAeration;
extern RTC_DateTypeDef sDate;
extern RTC_TimeTypeDef sTime;
extern SPI_HandleTypeDef hspi2;
extern int8_t countsec;
extern uint8_t ok0, ok1, ext[], cmdmodule, modules, disableBeep;
extern int16_t beepOn;
float stold[2][2];

void rotate_trays(uint8_t timer0, uint8_t timer1, struct rampv *ram){ // симистричный таймер
  uint8_t result;
  if(--ram->nextTurn==0){
    if(TURN) {ram->nextTurn=timer0; result = OFF;}
    else if(timer1) {ram->nextTurn=timer1; result = ON;} 
    else {ram->nextTurn=timer0; result = ON;}
    cmdmodule=NEW_TURN;
    if(ram->fuses&0x08) result = OFF;      // ПРЕДОХРАНИТЕЛЬ реле поворотов
    switch (result){
      case ON:  TURN = ON; break;
      case OFF: TURN = OFF; break;
    }
  }
}

void CO2_check(uint16_t spCO20, uint16_t spCO21, uint16_t pvCO20){ // ПРОВЕРКА концентрации СО2 производиться каждую минуту
 uint16_t sp;
  sp=(int)spCO20*100;
  if(pvCO20>sp) VENTIL = ON;
  sp=(int)spCO21*100;
  if(pvCO20<sp) VENTIL = OFF;
  if(ext[2]==255) pvCO20 = 0;// если модуль СО2 НЕПОДКЛЮЧЕН обнуление на случай отключения от компьютера
}

void aeration_check(uint8_t air0, uint8_t air1){ // ПРОВЕТРИВАНИЕ
  if(VENTIL){if(--pvAeration==0) {pvAeration=air0; VENTIL = OFF;}}
  else {if(--pvAeration==0) {pvAeration=air1; VENTIL = ON;}}
}

uint8_t sethorizon(uint8_t timer0, uint8_t turnTime, struct rampv *ram){ // установка в горизонт
 uint8_t state=0;
 static uint8_t counter=0;
  if(counter) --counter;                                   // ожижание прохода лотков TIMETURN
  if(counter==0){
     if (TURN){                                            // если лотки в ВЕРХНЕМ положении то сразу в горизонт
        ram->nextTurn=timer0; state = 0x10; TURN = OFF;     // ГОРИЗОНТ УСТАНОВЛЕН
        cmdmodule=SETHORIZON;                              // команда дополнительному модулю
     }
     else {
      if((ram->fuses&0x08)==0) TURN = ON;      // ПРЕДОХРАНИТЕЛЬ реле поворотов 
      counter=turnTime;
     }                   // если лотки в НЕЖНЕМ положении то разгрузка лотков
  }
  return state;
}

uint16_t adcTomV(uint16_t curr){
  float res;
  if(curr < 125) curr = 0;                          // меньще 100 mV
  else {res = curr * VREF / 4096; curr = res*1000;} // в mV диапазон от 100 mV до 3300 mV (VREF = 3.3f;)
  return curr;
}


uint8_t statF2(uint8_t n, uint16_t statPw){
 float val;
  val = A4 * stold[n][0] - A5 * stold[n][1] + A6 * statPw;
  stold[n][1] = stold[n][0];
  stold[n][0] = val;
  return val;
}

void chkdoor(struct eeprom *t, struct rampv *ram){
 static uint16_t counter;
  uint8_t doorState = HAL_GPIO_ReadPin(Door_GPIO_Port, Door_Pin);// концевик дверей
//  doorState = 1;  //?????????????????????????????????????????????????????????
  if(doorState){  // Дверь ЗАкрыта
     ram->fuses &= 0x7F;  // Состояние дверей
     if(t->state&4){      //-- если "подгототка к ВКЛЮЧЕНИЮ" то включить камеру --
        t->state &=0xFB; t->state |=0x01; counter = 0; countsec=-5; ok0=0; ok1=0; ram->flap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &ram->flap);
        if(t->extendMode==1) EXTRA = OFF; // доп. канал (extendMode==1->ВЕНТИЛЯЦИЯ)
      }
     else if((t->state&3)==3) {beeper_ON(DURATION/2); if(++counter>300) {t->state &= 0xF9; counter = 0;}}//-- если превышено ожидание то снимаем "подгототка к ОТКЛЮЧЕНИЮ"
   }
  else { // Дверь ОТкрыта
     ram->fuses |= 0x80;  // Состояние дверей
     if((t->state&7)==3)  //-- если "подгототка к ОТКЛЮЧЕНИЮ" то отключить камеру --
      {
       t->state &=0xFC; t->state |=0x04; counter = 0; ram->power=0; HUMIDI = OFF; FLAP = ON; ram->flap=FLAPOPEN; if(modules&8) chkflap(SETFLAP, &ram->flap);
       if(t->extendMode==1) EXTRA = ON; // доп. канал (extendMode==1->ВЕНТИЛЯЦИЯ)
      }
     else if((t->state&7)==1) beeper_ON(DURATION*5);//-- если камера ВКЛ. то вкл. тревогу.
     else if(t->state&4) {if(++counter>t->waitCooling*10) {beeper_ON(DURATION); ALARM = 1;}}//-- если превышено ожидание то вкл. тревогу.(Режим "подгототка к ВКЛЮЧЕНИЮ")
   }
}

#define SYSCLOCK 72000000U  // Определение системной тактовой частоты
void sysTick_Init(void){
  // устанавливает значение SysTick Reload Register (LOAD) для генерации прерывания каждую миллисекунду
  // Она использует макрос MODIFY_REG для изменения соответствующих битов LOAD регистра SysTick.
  // если SYSCLOCK равно 72 МГц, то значение LOAD будет установлено на 71999
  MODIFY_REG(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk, SYSCLOCK / 1000 - 1);
  // сбрасывает текущее значение SysTick Counter Register (VAL) в 0, чтобы начать счет заново.
  CLEAR_BIT(SysTick->VAL, SysTick_VAL_CURRENT_Msk);
  // настраивает регистр управления SysTick (CTRL). 
  // Бит CLKSOURCE устанавливается, чтобы использовать системную тактовую частоту в качестве источника тактирования для SysTick. 
  // Бит ENABLE устанавливается для включения SysTick. Бит TICKINT устанавливается для разрешения генерации прерывания SysTick.
  SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}

void beeper_ON(uint16_t duration){
  HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
  beepOn = duration;
}

//---- запись в микросхему 74HC595D ----
void set_Output(void){
  HAL_SPI_Transmit(&hspi2,(uint8_t*)&portOut, 1, 5000);
  RCK_H();
  RCK_L();
}

void set_Pulse(uint16_t val){
  PB5_H();
  for (uint16_t i = 0; i < val; i++) __NOP();
  PB5_L();
}
