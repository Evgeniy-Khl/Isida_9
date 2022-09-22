#include "main.h"
//#include "power.h"
#include "global.h"   // здесь определена структура eeprom

#define ON          1
#define OFF         0
#define UNCHANGED   2

extern uint8_t Hih, errors, ok0, ok1, warning, valRun;
extern int16_t pvRH, pvT[];
//extern float PVold1, PVold2, iPart[3], stold[2][2];

