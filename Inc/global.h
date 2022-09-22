/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H
extern struct {
  uint8_t RXBuffer[2];
  uint8_t TXBuffer[2];
  uint8_t buf[60];
  uint8_t ind;
  uint8_t timeOut;
  uint8_t devOk;
} bluetoothData;

extern struct rampv {
    uint16_t cellID;              // 2 байт ind=0         сетевой номер прибора
    int16_t pvT[MAX_DEVICES];     // 8 байт ind=2-ind=9   значения [MAX_DEVICES] датчиков температуры
    int16_t pvRH;                 // 2 байт ind=10;ind=11 значение датчика относительной влажности
    int16_t pvCO2[3];             // 6 байт ind=12-ind=17 значения датчика CO2 ---------- ИТОГО 18 bytes ------------
    uint8_t pvTimer;              // 1 байт ind=18        значение таймера
    uint8_t pvTmrCount;           // 1 байт ind=19        не используется !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t pvFlap;               // 1 байт ind=20        положение заслонки 
    int8_t power;                 // 1 байт ind=21        мощность подаваемая на тены
    uint8_t fuses;                // 1 байт ind=22        короткие замыкания 
    uint8_t errors, warning;      // 2 байт ind=23;ind=24 ошибки и предупреждения
    uint8_t cost0, cost1;         // 2 байт ind=25;ind=26 затраты ресурсов
    uint8_t date, hours;          // 2 байт ind=27;ind=28 счетчики суток и часов
    uint8_t other0;               // 1 байт ind=29        не используется !!!!!!!!!!!!!!!!!!!!!!!!!
  } pv;// ------------------ ИТОГО 27 bytes -------------------------------

extern struct eeprom {
    int16_t spT[2];     // 4 байт ind=0-ind=3   Уставка температуры sp[0].spT->Сухой датчик; sp[1].spT->Влажный датчик
    int16_t spRH[2];    // 4 байт ind=4-ind=7   sp[0].spRH->ПОДСТРОЙКА HIH-5030; sp[1].spRH->Уставка влажности Датчик HIH-5030
    int16_t K[2];       // 4 байт ind=8-ind=11  пропорциональный коэфф.
    int16_t Ti[2];      // 4 байт ind=12-ind=15 интегральный коэфф.
    int16_t minRun;     // 2 байт ind=16;ind=17 импульсное управление насосом увлажнителя
    int16_t maxRun;     // 2 байт ind=18;ind=19 импульсное управление насосом увлажнителя
    int16_t period;     // 2 байт ind=20;ind=21 импульсное управление насосом увлажнителя
    int16_t TimeOut;    // 2 байт ind=22;ind=23 время ожидания начала режима охлаждения
    int16_t EnergyMeter;// 2 байт ind=24;ind=25   ----------- ИТОГО 26 bytes ------------
    int8_t timer[2];    // 2 байт ind=26;ind=27 [0]-отключ.состояниe [1]-включ.состояниe
    int8_t alarm[2];    // 2 байт ind=28;ind=29 дельта 5 = 0.5 гр.C
    int8_t extOn[2];    // 2 байт ind=30;ind=31 смещение для ВКЛ. вспомогательного канала
    int8_t extOff[2];   // 2 байт ind=32;ind=33 смещение для ОТКЛ. вспомогательного канала
    uint8_t air[2];     // 2 байт ind=34;ind=35 таймер проветривания air[0]-пауза; air[1]-работа; если air[1]=0-ОТКЛЮЧЕНО
    uint8_t spCO2;      // 1 байт ind=36;       опорное значение для управления концетрацией СО2
    uint8_t identif;    // 1 байт ind=37;       сетевой номер прибора
    uint8_t condition;  // 1 байт ind=38;       состояние камеры (ОТКЛ. ВКЛ. ОХЛАЖДЕНИЕ, и т.д.)
    uint8_t extendMode; // 1 байт ind=39;       расширенный режим работы  0-СИРЕНА; 1-ВЕНТ. 2-Форс НАГР. 3-Форс ОХЛЖД. 4-Форс ОСУШ. 5-Дубляж увлажнения
    uint8_t relayMode;  // 1 байт ind=40;       релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
    uint8_t programm;   // 1 байт ind=41;       работа по программе
    uint8_t Hysteresis; // 1 байт ind=42;       Гистерезис канала увлажнения
    uint8_t ForceHeat;  // 1 байт ind=43;       Форсированный нагрев 5 = 0.5 грд.С.
    uint8_t TurnTime;   // 1 байт ind=44;       время ожидания прохода лотков в секундах
    uint8_t HihEnable;  // 1 байт ind=45;       разрешение использования датчика влажности
    uint8_t KoffCurr;   // 1 байт ind=46;       маcштабный коэф. по току симистора  (160 для AC1010 или 80 для другого)
    uint8_t coolOn;     // 1 байт ind=47;       не используется !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t coolOff;    // 1 байт ind=48;       не используется !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t Zonality;   // 1 байт ind=49;       порог зональности в камере ----------- 24 bytes ------------
} sp;// ------------------ ИТОГО 50 bytes -------------------------------

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

//#define CANAL1	portOut.bitfield.a0  // симистор НАГРЕВАТЕЛЯ
//#define CANAL2	portOut.bitfield.a1  // реле или симистор УВЛАЖНЕНИЯ
//#define EXT1		portOut.bitfield.a2  // реле вспомогательного канала
//#define EXT2		portOut.bitfield.a3  // реле вспомогательного канала
//#define TURN		portOut.bitfield.a4  // поворот
//#define COOLER	portOut.bitfield.a5  // вентилятор охладителя

#endif /* __GLOBAL_H */
