#include "main.h"
#include <stdlib.h>
#include "global.h"   // здесь определена структура eeprom и структура rampv
#include "output.h"

extern uint8_t disableBeep;
extern int16_t pulsPeriod;
uint8_t ok0, ok1;
uint16_t valRun;
float iPart[3];

uint8_t heatCondition(int16_t err, uint8_t alarm, uint8_t extOn){
  uint8_t warn = 0;
  if(abs(err)<alarm) ok0 = 1;               // вышли на заданное значение
  if(ok0 && err>alarm){ok0 = 2; warn=1;}    // Сильное ПЕРЕОХЛАЖДЕНИЕ !
  if(err < -alarm)   ok0 = 3;               // ПЕРЕГРЕВ, разрешаем увлажнение
  alarm +=extOn;                            // с учетом воздушной заслонки для канала температуры
  if(err < -alarm) warn = 1;                // Сильный ПЕРЕГРЕВ !
  return warn;
}

uint8_t humCondition(int16_t err, uint8_t alarm, uint8_t extOn){
  uint8_t warn = 0;
  if(abs(err)<alarm) ok1 = 1;               // вышли на заданное значение
  if(ok1 && err>alarm){ok1 = 2; warn=1;}    // Сильно СУХО !
  if(err < -alarm)   ok1 = 3;               // ВЛАЖНО
  alarm +=extOn;                            // с учетом воздушной заслонки для канала увлажнения
  if(err < -alarm) warn = 1;                // Сильнo ВЛАЖНО !
  return warn;
}

int16_t UpdatePID(int16_t err, uint8_t cn, struct eeprom *t){
 int16_t maxVal;
 float pPart, Ud;
  if(cn) maxVal=t->maxRun; else maxVal=MAXPULS; // maxRun = 10000 = 10 секунд; 1100 -> 1.1 сек.
  pPart = (float) err * t->pkoff[cn];               // расчет пропорциональной части
//---- функция ограничения pPart ---------------
  if (pPart < 0) pPart = 0;
  else if (pPart > maxVal) pPart = maxVal;      // функция ограничения
//----------------------------------------------
  iPart[cn] += (float) t->pkoff[cn] / t->ikoff[cn] * 10 * err;  // приращение интегральной части
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
  uint16_t result;
  static int8_t direction;
  // релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
  if(t->relayMode&2){
    if(err > t->hysteresis) {result = MAXPULS; direction = 1;}  // ниже (заданной+hysteresis) включить увлажнитель
    else if(err < 0) {result = 0; direction = -1;}              // выше заданной отключить увлажнитель
    else if(direction>0) result = MAXPULS;                      // продолжаем увлажнять
    else result = 0;                                            // продолжаем осушать
  }
  else if(t->relayMode==4){
    valRun = UpdatePID(err, 1, t);                              // определение длительности ВКЛ. состояния
    if(valRun < t->minRun) valRun = t->minRun;                  // минимальный предел
    else if(valRun > t->maxRun) valRun = t->maxRun;             // максимальный предел
    if(valRun > t->period) valRun = t->period;                  // длит. впрыска не должна превыщать длит.переода
    if(err<=0) valRun = 0;                                      // отключение впрыска по 2 каналу если перелив
  }
  else result = UpdatePID(err, 0, t);
  return result;
}

uint8_t RelayPos(int16_t err, uint8_t cn, uint8_t hysteresis){
 uint8_t x=UNCHANGED;
  if(err > hysteresis) x = ON;          // ниже (заданной-offSet) включить
  if(err < 0) x = OFF;                  // выше заданной отключить
  return x;
}

uint8_t RelayNeg(int16_t err, uint8_t cn, int8_t extOn, int8_t extOff){
 uint8_t x=UNCHANGED;
  err += extOn;                         // err -> отрицательная величина
  if (err < 0) x = ON;                  // выше (заданной+offSet) включить
  if (err > extOn-extOff) x = OFF;      // ниже (заданной+0,2) отключить
  return x;
}

// расширенный режим работы  0-СИРЕНА; 1-ВЕНТ. 2-Форс НАГР. 3-Форс ОХЛЖД. 4-Форс ОСУШ. 5-Дубляж увлажнения
void extra_2(struct eeprom *t, struct rampv *ram){
  uint8_t byte = UNCHANGED;
  int16_t err;
  switch (t->extendMode){
  	// доп. канал -> ВЕНТ.
    case 1: if(VENTIL) byte = ON; else byte = OFF;
  		break;
    // доп. канал -> Форсированный нагрев
    case 2: 
            err = t->spT[0]-ram->pvT[0];
//            if(err > (t->ForceHeat+t->hysteresis)) byte = ON;
//            else if(err < t->ForceHeat) byte = OFF;
  		break;
  	// доп. канал -> Форсированное охлаждение
    case 3: 
            err = t->spT[0]-ram->pvT[0];
            byte = RelayNeg(err, 0, t->extOn[1],t->extOff[1]);
  		break;
    // доп. канал -> Форсированное осушение
    case 4: 
            err = t->spT[1]-ram->pvT[1];
            byte = RelayNeg(err, 1, t->extOn[1],t->extOff[1]);
      break;
  	// доп. канал -> Дублирование канала увлажнения
    case 5: 
            err = t->spT[1]-ram->pvT[1];
            byte = RelayPos(err, 1, t->hysteresis);
            if(!(ok0&1)) byte = OFF;  // отключение УВЛАЖНЕНИЯ при РАЗОГРЕВЕ и ПЕРЕОХЛАЖДЕНИИ
  		break;
    // доп. канал -> Сирена
    default: 
            err = ram->errors + ram->warning + ram->fuses + ALARM;
            if(err && disableBeep==0) byte = ON; else byte = OFF;
  }
  if(!(ram->fuses&0x04)) byte=OFF;    // ПРЕДОХРАНИТЕЛЬ доп. канал №2
  switch (byte){
      case ON:  EXT2 = ON;  break;
      case OFF: EXT2 = OFF; break;
  }
}

void OutPulse(int16_t err, struct eeprom *t)
{
  if(ok0==0||ok0==2){valRun = 0; return;};        // отключение впрыска по 2 каналу если идет разогрев или переохлаждение
  valRun = UpdatePID(err,1,t);                    // определение длительности ВКЛ. состояния
  if(valRun < t->minRun) valRun = t->minRun;
  else if(valRun > t->period) valRun = t->period; // длит. впрыска не должна превыщать длит.переода
  if(err<=0) valRun = 0;                          // отключение впрыска по 2 каналу если перелив
}

