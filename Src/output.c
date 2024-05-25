#include "main.h"
#include <stdlib.h>
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "output.h"

extern uint8_t disableBeep;
extern int16_t pulsPeriod;
uint8_t ok0, ok1;
uint16_t valRun;
float iPart[3];

uint8_t heatCondition(int16_t err, uint8_t alarm){
  uint8_t warn = 0;
  if(abs(err)<alarm) ok0 = 1;               // ����� �� �������� ��������
  if(ok0 && err>=alarm){ok0 = 2; warn=1;}   // �������������� !
  if(err <= -alarm){ok0 = 3; warn = 1;}     // �������� ! ��������� ����������
  return warn;
}

uint8_t humCondition(int16_t err, uint8_t alarm){
  uint8_t warn = 0;
  if(abs(err)<alarm) ok1 = 1;               // ����� �� �������� ��������
  if(ok1 && err>=alarm){ok1 = 2; warn=1;}   // ���� !
  if(err <= -alarm){ok1 = 3; warn = 1;}     // ������ !
  return warn;
}

int16_t UpdatePID(int16_t err, uint8_t cn, struct eeprom *t){
 uint16_t maxVal;
 float pPart, Ud;
  if(cn==2) maxVal=t->maxRun*200; else maxVal=MAXPULS; // maxRun = 2000 -> 10 ������; MAXPULS = 250 // 1100 -> 1.1 ���.
  pPart = (float) err * t->pkoff[cn];           // ������ ���������������� �����
//---- ������� ����������� pPart ---------------
  if (pPart < 0) pPart = 0;
  else if (pPart > maxVal) pPart = maxVal;      // ������� �����������
//----------------------------------------------
  iPart[cn] += (float) t->pkoff[cn] / t->ikoff[cn] / 10 * err;  // ���������� ������������ �����
  Ud = pPart + iPart[cn];                       // ����� ���������� �� �����������
//---- ������� ����������� Ud ------------------
  if (Ud < 0) Ud = 0;
  else if (Ud > maxVal) Ud = maxVal;            // ������� �����������
  iPart[cn] = Ud - pPart;                       // "��������������" ��������
  return Ud;
};

uint16_t heater(int16_t err, struct eeprom *t){
  uint16_t result;
  // �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1]
  if(t->relayMode&1){
    if(err > 0) result = MAXPULS; else result=0;
  }
  else result = UpdatePID(err, 0, t);
  return result;
}

uint16_t humidifier(int16_t err, struct eeprom *t){
  uint16_t result, minR, maxR;
  static int8_t keep_up;
  // �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1] 4->�� ���.[1] ���������� �����
  if(t->relayMode&2){
    if(err > (t->hysteresis&3)) {result = MAXPULS; keep_up = 1;}// ���� (��������+hysteresis) �������� ����������� (MAXPULS=250)
    else if(err < 0) {result = 0; keep_up = -1;}                // ���� �������� ��������� �����������
    else if(keep_up>0) result = MAXPULS;                        // ���������� ���������
    else result = 0;                                            // ���������� �������
  }
  else if(t->relayMode==4){
    minR = t->minRun * 20;                                      // minRun -> 5*20=100    -> 0.5���.
    maxR = t->maxRun * 200;                                     // maxRun -> 10*200=2000 -> 10 ���.
    valRun = UpdatePID(err, 2, t);                              // ����������� ������������ ���. ���������
    if(valRun < minR) valRun = minR;                            // ����������� ������
    else if(valRun > maxR) valRun = maxR;                       // ������������ ������
    if(valRun > t->period) valRun = t->period;                  // ����. ������� �� ������ ��������� ����.������� (period = 60 -> 60 ���. = 1 ���.)
    if(err<=0) valRun = 0;                                      // ���������� ������� �� 2 ������ ���� �������
  }
  else result = UpdatePID(err, 1, t);
  return result;
}

uint8_t RelayPos(int16_t err, uint8_t cn, uint8_t hysteresis){
 uint8_t x=UNCHANGED;
  if(err > hysteresis) x = ON;          // ���� (��������-offSet) ��������
  if(err < 0) x = OFF;                  // ���� �������� ���������
  return x;
}

// ����������� ����� ������  0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
uint8_t extra_2(struct eeprom *t, struct rampv *ram){
  uint8_t byte = UNCHANGED;
  int16_t err;
  // 0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
  switch (t->extendMode){
  	// ���. ����� -> ������
    case 0: err = ram->errors + ram->warning + ram->fuses + ALARM;
            if(err && disableBeep==0) byte = ON; else byte = OFF;
      break;
    // ���. ����� -> ����.
    case 1: if(VENTIL) byte = ON; else byte = OFF; break;
    // ���. ����� -> ������������� ������
    case 2: 
            err = t->spT[0] - ram->pvT[0];
            if(err >= t->extOn[0]) byte = ON;
            else if(err <= t->extOff[0]) byte = OFF;
  		break;
  	// ���. ����� -> ������������� ����������
    case 3: 
            err = ram->pvT[0] - t->spT[0];
            if(err >= t->extOn[0]) byte = ON;
            else if(err <= t->extOff[0]) byte = OFF;
  		break;
    // ���. ����� -> ������������� ��������
    case 4: 
            err = ram->pvT[1] - t->spT[1];
            if(err >= t->extOn[1]) byte = ON;
            else if(err <= t->extOff[1]) byte = OFF;
      break;
  	// ���. ����� -> ������������ ������ ����������
    case 5: if(HUMIDI) byte = ON; else byte = OFF; break;
  }
//  if(!(ram->fuses&0x04)) byte=OFF;    // �������������� ���. ����� �2
  return byte;
}

void OutPulse(int16_t err, struct eeprom *t)
{
  if(ok0==0||ok0==2){valRun = 0; return;};        // ���������� ������� �� 2 ������ ���� ���� �������� ��� ��������������
  valRun = UpdatePID(err,1,t);                    // ����������� ������������ ���. ���������
  if(valRun < t->minRun) valRun = t->minRun;
  else if(valRun > t->period) valRun = t->period; // ����. ������� �� ������ ��������� ����.�������
  if(err<=0) valRun = 0;                          // ���������� ������� �� 2 ������ ���� �������
}

