/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H

extern struct rampv {
    uint8_t model;       // 1 байт ind=0  модель прибора
    uint8_t node;        // 1 байт ind=1  сетевой номер прибора
    int16_t pvT[MAX_DEVICES]; // 6 байт ind=2-ind=7   значения [MAX_DEVICES=3] датчиков температуры
    uint8_t pvRH;        // 1 байт ind=8  значение датчика относительной влажности
    uint8_t pvCO2;       // 1 байт ind=9  значения датчика CO2 (от 4 -> 400 до 50 -> 5000)
    uint8_t nextTurn;    // 1 байт ind=10 значение таймера до начала поворота лотков
    uint8_t fan;         // 1 байт ind=11 скорость вращения тихоходного вентилятора
    uint8_t flap;        // 1 байт ind=12 положение заслонки 
    uint8_t power;       // 1 байт ind=13 мощность подаваемая на тены
    uint8_t fuses;       // 1 байт ind=14 короткие замыкания 
    uint8_t errors;      // 1 байт ind=15 ошибки
    uint8_t warning;     // 1 байт ind=16 предупреждения
    uint8_t nothing0;    // 1 байт ind=17
    uint8_t nothing1;    // 1 байт ind=18
    uint8_t nothing2;    // 1 байт ind=19
  } pv;// ------------------ ИТОГО 20 bytes -------------------------------

extern struct eeprom {
    int16_t spT[2];     // 4 байт ind=0-ind=3   Уставка температуры sp[0].spT->Сухой датчик; sp[1].spT->Влажный датчик
    //spRH[0]->ПОДСТРОЙКА HIH-5030/AM2301; spRH[1]->Уставка влажности Датчик HIH-5030/AM2301
    int8_t  spRH[2];    // 2 байт ind=4;ind=5   
    uint8_t state;      // 1 байт ind=6;        состояние камеры (ОТКЛ. ВКЛ. ОХЛАЖДЕНИЕ, и т.д.)
    uint8_t extendMode; // 1 байт ind=7;        расширенный режим работы  0-СИРЕНА; 1-ВЕНТ. 2-Форс НАГР. 3-Форс ОХЛЖД. 4-Форс ОСУШ. 5-Дубляж увлажнения
    uint8_t relayMode;  // 1 байт ind=8;        релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
    uint8_t programm;   // 1 байт ind=9;        работа по программе
    uint8_t minRun;     // 1 байт ind=10        импульсное управление насосом увлажнителя
    uint8_t maxRun;     // 1 байт ind=11        импульсное управление насосом увлажнителя
    uint8_t period;     // 1 байт ind=12        импульсное управление насосом увлажнителя
    uint8_t timer[2];   // 2 байт ind=13;ind=14 [0]-отключ.состояниe [1]-включ.состояниe
    uint8_t alarm[2];   // 2 байт ind=15;ind=16 дельта 5 = 0.5 гр.C
    uint8_t extOn[2];   // 2 байт ind=17;ind=18 смещение для ВКЛ. вспомогательного канала
    uint8_t extOff[2];  // 2 байт ind=19;ind=20 смещение для ОТКЛ. вспомогательного канала
    uint8_t air[2];     // 2 байт ind=21;ind=22 таймер проветривания air[0]-пауза; air[1]-работа; если air[1]=0-ОТКЛЮЧЕНО
    uint8_t spCO2;      // 1 байт ind=23;       опорное значение для управления концетрацией СО2
    uint8_t koffCurr;   // 1 байт ind=24;       маштабный коэф. по току симистора  (150 для AC1010)
    uint8_t hysteresis; // 1 байт ind=25;       гистерезис канала увлажнения
    uint8_t zonaFlap;   // 1 байт ind=26;       порог зональности в камере
    uint8_t turnTime;   // 1 байт ind=27;       время ожидания прохода лотков в секундах
    uint8_t waitCooling;// 1 байт ind=28        время ожидания начала режима охлаждения
    uint8_t pkoff[2];   // 2 байт ind=29;ind=30 пропорциональный коэфф.
    uint8_t ikoff[2];   // 2 байт ind=31;ind=32 интегральный коэфф.
    uint8_t identif;    // 1 байт ind=33;       сетевой номер прибора
    uint8_t ip[4];      // 4 байт ind=34;ind=35;ind=36;ind=37; IP MQTT broker [192.168.100.100]
    uint8_t nothing0;   // 1 байт ind=38;       не используется !
    uint8_t nothing1;   // 1 байт ind=39;       не используется !
} sp;// ------------------ ИТОГО 40 bytes -------------------------------

extern union Byte portFlag;
extern union Byte portOut;
/* ---структура с битовыми полями -----*/
//extern struct byte {
//    unsigned a0: 1;
//    unsigned a1: 1;
//    unsigned a2: 1;
//    unsigned a3: 1;
//    unsigned a4: 1;
//    unsigned a5: 1;
//    unsigned a6: 1;
//    unsigned a7: 1;
//};
///* ---структура объединена с обычным байтом -----*/
//union Byte {
//    unsigned char value;
//    struct byte bitfield;
//}portOut;

//#define HEATER  portOut.bitfield.a0  // НАГРЕВАТЕЛЬ
//#define HUMIDI	portOut.bitfield.a1  // УВЛАЖНИТЕЛЬ
//#define FLAP		portOut.bitfield.a2  // Заслонка воздухообмена
//#define EXTRA		portOut.bitfield.a3  // Вспомогательный канал
//#define TURN		portOut.bitfield.a4  // Поворот лотков
//#define COOLER	portOut.bitfield.a5  // вентилятор охладителя

#endif /* __GLOBAL_H */
