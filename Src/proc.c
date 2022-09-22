#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv
#include "proc.h"
#include "module.h"


const float A4=1.8, A5=0.81, A6=0.01;  // порядок a=0.9 (A1=2a; A2=a^2; A3=(1-a)^2)
const float A1=1.6, A2=0.64, A3=0.04;  // порядок a=0.8 (A1=2a; A2=a^2; A3=(1-a)^2)
const float VREF = 3.3f;

uint8_t pvAeration;
extern MY_DateTypeDef sDate;
extern MY_TimeTypeDef sTime;
extern SPI_HandleTypeDef hspi2;
extern int8_t countsec;
extern uint8_t ok0, ok1, ext[], cmdmodule, modules, disableBeep;
extern int16_t alarmErr, beepOn;
float stold[2][2];

void rotate_trays(uint8_t timer0, uint8_t timer1, struct rampv *ram){ // симистричный таймер
  uint8_t result;
  if(--ram->pvTimer==0){
    if(TURN) {ram->pvTimer=timer0; result = OFF;}
    else if(timer1) {ram->pvTimer=timer1; result = ON;} 
    else {ram->pvTimer=timer0; result = ON;}
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

uint8_t sethorizon(uint8_t timer0, uint8_t TurnTime, struct rampv *ram){ // установка в горизонт
 uint8_t state=0;
 static uint8_t counter=0;
  if(counter) --counter;                                   // ожижание прохода лотков TIMETURN
  if(counter==0){
     if (TURN){                                            // если лотки в ВЕРХНЕМ положении то сразу в горизонт
        ram->pvTimer=timer0; state = 0x10; TURN = OFF;     // ГОРИЗОНТ УСТАНОВЛЕН
        cmdmodule=SETHORIZON;                              // команда дополнительному модулю
     }
     else {
      if((ram->fuses&0x08)==0) TURN = ON;      // ПРЕДОХРАНИТЕЛЬ реле поворотов 
      counter=TurnTime;
     }                   // если лотки в НЕЖНЕМ положении то разгрузка лотков
  }
  return state;
}

uint16_t adcTomV(uint16_t curr){
  float res;
  if(curr < 125) curr = 0;                          // меньще 100 mV
  else {res = curr * VREF / 4096; curr = res*1000;} // в mV диапазон от 100 mV до 3300 mV
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
  doorState = 1;  //?????????????????????????????????????????????????????????
  if(doorState){  // Дверь ЗАкрыта
     ram->fuses &= 0x7F;  // Состояние дверей
     if(t->condition&4){      //-- если "подгототка к ВКЛЮЧЕНИЮ" то включить камеру --
        t->condition &=0xFB; t->condition |=0x01; counter = 0; countsec=-5; ok0=0; ok1=0; ram->pvFlap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &ram->pvFlap);
        if (t->extendMode==1) EXT2 = OFF; // доп. канал (extendMode==1->ВЕНТИЛЯЦИЯ)
      }
     else if((t->condition&3)==3) {beeper_ON(DURATION/2); if(++counter>300) {t->condition &= 0xF9; counter = 0;}}//-- если превышено ожидание то снимаем "подгототка к ОТКЛЮЧЕНИЮ"
   }
  else { // Дверь ОТкрыта
     ram->fuses |= 0x80;  // Состояние дверей
     if((t->condition&7)==3)  //-- если "подгототка к ОТКЛЮЧЕНИЮ" то отключить камеру --
      {
       t->condition &=0xFC; t->condition |=0x04; counter = 0; ram->power=0; HUMIDI = OFF; FLAP = ON; ram->pvFlap=FLAPOPEN; if(modules&8) chkflap(SETFLAP, &ram->pvFlap);
       if (t->extendMode==1) EXT2 = ON; // доп. канал (extendMode==1->ВЕНТИЛЯЦИЯ)
      }
     else if((t->condition&7)==1) beeper_ON(DURATION*5);//-- если камера ВКЛ. то вкл. тревогу.
     else if(t->condition&4) {if(++counter>t->TimeOut) {beeper_ON(DURATION); ALARM = 1;}}//-- если превышено ожидание то вкл. тревогу.(Режим "подгототка к ВКЛЮЧЕНИЮ")
   }
}

#define SYSCLOCK 72000000U
void sysTick_Init(void){
  MODIFY_REG(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk, SYSCLOCK / 1000 - 1);
  CLEAR_BIT(SysTick->VAL, SysTick_VAL_CURRENT_Msk);
  SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}

void beeper_ON(uint16_t duration){
  HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
  beepOn = duration;
}

void set_Output(void){
  HAL_SPI_Transmit(&hspi2,(uint8_t*)&portOut, 1, 5000);
  RCK_H();
  RCK_L();
}

//----------- Функція встановлення дати і часу ----------------------------------------
void setDataAndTime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday, 
             uint8_t hour, uint8_t min,  uint8_t sec){
    /**Initialize RTC and set the Time and Date**/
  sTime.Hours = hour;
  sTime.Minutes = min;
  sTime.Seconds = sec;
  sDate.WeekDay = weekday;
  sDate.Month = month;
  sDate.Date = day;
  sDate.Year = year;
}

