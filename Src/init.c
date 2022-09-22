#include "main.h"
#include "TM1638.h"
#include "ds18b20.h"
#include "displ.h"
#include "module.h"
#include "global.h"   // здесь определена структура eeprom
#include "am2301.h"
#include "proc.h"
//#include "FatFsAPI.h"

extern uint8_t ds18b20_amount, modules, pvTimer, pvAeration, topUser, card;
extern int16_t humAdc;
extern float PVold1, PVold2;

void init(struct eeprom *t, struct rampv *ram){
 int8_t i;
//  for(i=0;i<8;i++) {setChar(i,SIMBL_BL); PointOn(i);}  // only decimal points is on
//  SendDataTM1638();
//---------- Поиск датчиков ---------------------------------------------------------------------------------------------------
  ds18b20_port_init();
  ds18b20_count(MAX_DEVICES);     // проверяем наличие датчиков если error = 0 датчики найдены
  setChar(0,ds18b20_amount); setChar(1,SIMBL_d); setChar(2,0);  // "2d0"
//  if(ds18b20_amount > 2) checkSensor(); // проверяем на подключение датчика температуры скорлупы яйца
  if(t->HihEnable){                       // если разрешено ищем HIH-5030 в V humAdc 
    if(humAdc > 500){                     // humAdc => 500 mV для RH=0%
      HIH5030 = 1; setChar(2,1);          // если обнаружен HIH-5030 "2d1"
      PVold1 = PVold2 = humAdc;
    }
  }
  else {
//    AM2301 = am2301_Read(ram, t->spRH[0]);
    HAL_Delay(1000);
    am2301_port_init();
    AM2301 = am2301_Start();
    if(AM2301) setChar(2,1);              // если обнаружен AM2301 "2d1"
  }
  i = t->identif;
  setChar(3,SIMBL_n); setChar(4,i/10); setChar(5,i%10); // "n01"
  displ_3(VERSION,VERS,0); SendDataTM1638();            // Версия программы
//=====
  if(ds18b20_amount) ds18b20_Convert_T();
  else {// если датчики не обнаружены - останавливаем программу и ищем датчики
    while(ds18b20_amount == 0){
      HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
      HAL_Delay(200);
      HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);  // Beeper Off
      HAL_Delay(400); 
      ds18b20_count(MAX_DEVICES);
    }
    ds18b20_Convert_T();
  }    
  HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
  HAL_Delay(100);
  HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);  // Beeper Off
	HAL_Delay(1000);
//---------- Поиск модулей расширения -----------------------------------------------------------------------------------------
	modules = 0;
//  if(sd_check()) modules|=0x20;    // SD search
  if(rtc_check()) modules|=0x10;    // Real Time Clock search and availability of EEPROM
  if(module_check(ID_HALL)) {modules|=1; t->condition|=0x40;} else t->condition&=0xBF;  // если модуль обнаружен включаем контроль иначе контроль отключаем
  if(module_check(ID_HORIZON)) {modules|=2; t->condition|=0x20;} else t->condition&=0xDF; // если модуль обнаружен включаем контроль иначе контроль отключаем
  if(module_check(ID_CO2)) modules|=4;    // модуль CO2
  if(module_check(ID_FLAP)) modules|=8;   // модуль воздушных заслонок 
  setChar(0,SIMBL_u); setChar(1,modules/10); setChar(2,modules%10); // "u00"
//---------- Датчик тока ------------------------------------------------------------------------------------------------------
  i = 0;
  if(t->KoffCurr==0){
    i = 99;   // ОТКЛЮЧЕН мониторинг тока симистора !!!
    setChar(3,SIMBL_o); setChar(4,i/10); setChar(5,i%10); // "o99"
    i = 0;
    while (i<6){
      HAL_Delay(100);
      HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
      HAL_Delay(50);
      HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);  // Beeper Off
      i++;
    }
  }
  else {setChar(3,SIMBL_o); setChar(4,i/10); setChar(5,i%10);} // "o00"}
  setChar(6,2+0xA); setChar(7,DISPL_o);  // "2o"
  SendDataTM1638();
//------------------ Инициализация SD карты -----------------------------------------------------------------------------------
//  i = My_LinkDriver();     // инициализация SD карты
//  setChar(3,SIMBL_c); setChar(4,SIMBL_d); setChar(5,i); // "Sd0"
//  if(i) card = 0; else card = 1;
//  setChar(6,2+0xA); setChar(7,DISPL_o);  // "2o"
//  SendDataTM1638();
//=====
  while (i<2){
    HAL_Delay(200);
    HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
    HAL_Delay(50);
    HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);  // Beeper Off
    i++;
  }
  HAL_Delay(2000);
  
  ram->cellID  = t->identif;
  ram->pvTimer = t->timer[0];
  pvAeration   = t->air[0];
//**********????????????******************
  ram->pvT[0]=999;
  ram->pvT[1]=999;
  ram->pvT[2]=999;
  ram->pvT[3]=999;
  ram->pvRH = 999;
  ram->date  = 1;
  ram->hours = 23;
  ram->fuses = 0;
}
