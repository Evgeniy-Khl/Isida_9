#include "main.h"
#include "TM1638.h"
#include "ds18b20.h"
#include "displ.h"
#include "module.h"
#include "global.h"   // ����� ���������� ��������� eeprom
#include "am2301.h"
#include "proc.h"

extern uint8_t ds18b20_amount, modules, nextTurn, pvAeration, topUser, card;
extern int16_t humAdc;
extern float PVold1, PVold2;

void init(struct eeprom *t, struct rampv *ram){
 int8_t err;
//---------- ����� �������� --------------
  ds18b20_port_init();
  ds18b20_count(MAX_DEVICES);             // ��������� ������� �������� ���� error = 0 ������� �������
  setChar(0,ds18b20_amount); setChar(1,SIMBL_d); setChar(2,0);  // "2d0"
//  if(ds18b20_amount > 2) checkSensor(); // ��������� �� ����������� ������� ����������� �������� ����

  if(t->hysteresis&0x40){                 // ���� ��������� ���� HIH-5030 � V humAdc 
    if(humAdc > 500){                     // humAdc => 500 mV ��� RH=0%
      HIH5030 = 1; setChar(2,1);          // ���� ��������� HIH-5030 "2d1"
      PVold1 = PVold2 = humAdc;
    }
  }
//  else if(t->hysteresis&0x80) {
//    am2301_port_init();
//    //AM2301 = am2301_Read(ram, t->spRH[0]);
//    AM2301 = am2301_Start();
//    if(AM2301) setChar(2,2);              // ���� ��������� AM2301 "2d2"
//  }
  err = t->identif;
  setChar(3,SIMBL_n); setChar(4,err/10); setChar(5,err%10); // "n01"
  displ_3(VERSION,VERS,0); SendDataTM1638();                // ������ ���������
//======== ������� ���������� ============ 
  if(ds18b20_amount) ds18b20_Convert_T();
  else {// ���� ������� �� ���������� - ������������� ��������� � ���� �������
    while(ds18b20_amount == 0){
      HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
      HAL_Delay(200);
      HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);  // Beeper Off
      HAL_Delay(500); 
      ds18b20_count(MAX_DEVICES);
    }
    ds18b20_Convert_T();
  }
	HAL_Delay(1000);
//---------- ����� ������� ���������� --------
	modules = 0;
  if(rtc_check()) modules|=0x10;    // Real Time Clock search and availability of EEPROM
  if(module_check(ID_HALL))    {modules|=1; t->state|=0x40;} else t->state&=0xBF;  // ���� ������ ��������� �������� �������� ����� �������� ���������
  if(module_check(ID_HORIZON)) {modules|=2; t->state|=0x20;} else t->state&=0xDF;  // ���� ������ ��������� �������� �������� ����� �������� ���������
  if(module_check(ID_CO2))      modules|=4;   // ������ CO2
  if(module_check(ID_FLAP))     modules|=8;   // ������ ��������� �������� 
  setChar(0,SIMBL_u); setChar(1,modules/10); setChar(2,modules%10); // "u00"
//---------- ������������� ���������� ����� --------------------
  err=0;
  if(t->koffCurr==0)  err|=1;   // E01 - �������� ���������� ���� ���������
//==============================================================
  setChar(3,SIMBL_E); setChar(4,err/10); setChar(5,err); // "E00"
  setChar(6,DISPL_MINUS); setChar(7,DISPL_MINUS);  // "--"
  SendDataTM1638();
  err++;
  while (err){
    HAL_Delay(500);
    HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
    HAL_Delay(500);
    HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);  // Beeper Off
    err--;
  }
  HAL_Delay(1000);
  
  ram->node  = t->identif;
  ram->nextTurn = t->timer[0];
  pvAeration   = t->air[0];
//**********????????????******************
  ram->pvT[0]=1990;
  ram->pvT[1]=1990;
  ram->pvT[2]=1990;
  ram->pvRH =199;
  ram->fuses = 0;
}
