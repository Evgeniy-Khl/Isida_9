#include "main.h"
#include <stdlib.h>
#include "global.h"   // здесь определена структура eeprom и структура rampv
#include "output.h"

extern uint8_t disableBeep;
extern int16_t pulsPeriod;
uint8_t ok0, ok1;
uint16_t valRun;
float iPart[3];

uint8_t heatCondition(int16_t err, uint8_t alarm){
  uint8_t warn = 0;
  if(abs(err)<alarm) ok0 = 1;               // вышли на заданное значение
  if(ok0 && err>=alarm){ok0 = 2; warn=1;}   // ПЕРЕОХЛАЖДЕНИЕ !
  if(err <= -alarm){ok0 = 3; warn = 1;}     // ПЕРЕГРЕВ ! Разрешаем увлажнение
  return warn;
}

uint8_t humCondition(int16_t err, uint8_t alarm){
  uint8_t warn = 0;
  if(abs(err)<alarm) ok1 = 1;               // вышли на заданное значение
  if(ok1 && err>=alarm){ok1 = 2; warn=1;}   // СУХО !
  if(err <= -alarm){ok1 = 3; warn = 1;}     // ВЛАЖНО !
  return warn;
}

int16_t UpdatePID(int16_t err, uint8_t cn, struct eeprom *t){
 uint16_t maxVal;
 float pPart, Ud;
  if(cn==2) maxVal=t->maxRun*200; else maxVal=MAXPULS; // maxRun = 2000 -> 10 секунд; MAXPULS = 250 // 1100 -> 1.1 сек.
  pPart = (float) err * t->pkoff[cn];           // расчет пропорциональной части
//---- функция ограничения pPart ---------------
  if (pPart < 0) pPart = 0;
  else if (pPart > maxVal) pPart = maxVal;      // функция ограничения
//----------------------------------------------
  iPart[cn] += (float) t->pkoff[cn] / t->ikoff[cn] / 10 * err;  // приращение интегральной части
  Ud = pPart + iPart[cn];                       // выход регулятора до ограничения
//---- функция ограничения Ud ------------------
  if (Ud < 0) Ud = 0;
  else if (Ud > maxVal) Ud = maxVal;            // функция ограничения
  iPart[cn] = Ud - pPart;                       // "антинасыщяющая" поправка
  return Ud;
};

uint16_t heater(int16_t err, struct eeprom *t){
  uint16_t result;
  // релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1]
  if(t->relayMode&1){
    if(err > 0) result = MAXPULS; else result=0;
  }
  else result = UpdatePID(err, 0, t);
  return result;
}

uint16_t humidifier(int16_t err, struct eeprom *t){
  uint16_t result, minR, maxR;
  static int8_t keep_up;
  // релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
  if(t->relayMode&2){
    if(err > (t->hysteresis&3)) {result = MAXPULS; keep_up = 1;}// ниже (заданной+hysteresis) включить увлажнитель (MAXPULS=250)
    else if(err < 0) {result = 0; keep_up = -1;}                // выше заданной отключить увлажнитель
    else if(keep_up>0) result = MAXPULS;                        // продолжаем увлажнять
    else result = 0;                                            // продолжаем осушать
  }
  else if(t->relayMode==4){
    minR = t->minRun * 20;                                      // minRun -> 5*20=100    -> 0.5сек.
    maxR = t->maxRun * 200;                                     // maxRun -> 10*200=2000 -> 10 сек.
    valRun = UpdatePID(err, 2, t);                              // определение длительности ВКЛ. состояния
    if(valRun < minR) valRun = minR;                            // минимальный предел
    else if(valRun > maxR) valRun = maxR;                       // максимальный предел
    if(valRun > t->period) valRun = t->period;                  // длит. впрыска не должна превыщать длит.переода (period = 60 -> 60 сек. = 1 мин.)
    if(err<=0) valRun = 0;                                      // отключение впрыска по 2 каналу если перелив
  }
  else result = UpdatePID(err, 1, t);
  return result;
}

uint8_t RelayPos(int16_t err, uint8_t cn, uint8_t hysteresis){
 uint8_t x=UNCHANGED;
  if(err > hysteresis) x = ON;          // ниже (заданной-offSet) включить
  if(err < 0) x = OFF;                  // выше заданной отключить
  return x;
}

// расширенный режим работы  0-СИРЕНА; 1-ВЕНТ. 2-Форс НАГР. 3-Форс ОХЛЖД. 4-Форс ОСУШ. 5-Дубляж увлажнения
uint8_t extra_2(struct eeprom *t, struct rampv *ram){
  uint8_t byte = UNCHANGED;
  int16_t err;
  // 0-СИРЕНА; 1-ВЕНТ. 2-Форс НАГР. 3-Форс ОХЛЖД. 4-Форс ОСУШ. 5-Дубляж увлажнения
  switch (t->extendMode){
  	// доп. канал -> Сирена
    case 0: err = ram->errors + ram->warning + ram->fuses + ALARM;
            if(err && disableBeep==0) byte = ON; else byte = OFF;
      break;
    // доп. канал -> ВЕНТ.
    case 1: if(VENTIL) byte = ON; else byte = OFF; break;
    // доп. канал -> Форсированный нагрев
    case 2: 
            err = t->spT[0] - ram->pvT[0];
            if(err >= t->extOn[0]) byte = ON;
            else if(err <= t->extOff[0]) byte = OFF;
  		break;
  	// доп. канал -> Форсированное охлаждение
    case 3: 
            err = ram->pvT[0] - t->spT[0];
            if(err >= t->extOn[0]) byte = ON;
            else if(err <= t->extOff[0]) byte = OFF;
  		break;
    // доп. канал -> Форсированное осушение
    case 4: 
            err = ram->pvT[1] - t->spT[1];
            if(err >= t->extOn[1]) byte = ON;
            else if(err <= t->extOff[1]) byte = OFF;
      break;
  	// доп. канал -> Дублирование канала увлажнения
    case 5: if(HUMIDI) byte = ON; else byte = OFF; break;
  }
//  if(!(ram->fuses&0x04)) byte=OFF;    // ПРЕДОХРАНИТЕЛЬ доп. канал №2
  return byte;
}

void OutPulse(int16_t err, struct eeprom *t)
{
  if(ok0==0||ok0==2){valRun = 0; return;};        // отключение впрыска по 2 каналу если идет разогрев или переохлаждение
  valRun = UpdatePID(err,1,t);                    // определение длительности ВКЛ. состояния
  if(valRun < t->minRun) valRun = t->minRun;
  else if(valRun > t->period) valRun = t->period; // длит. впрыска не должна превыщать длит.переода
  if(err<=0) valRun = 0;                          // отключение впрыска по 2 каналу если перелив
}

