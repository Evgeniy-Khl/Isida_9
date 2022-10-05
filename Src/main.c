/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "proc.h"
#include "module.h"
#include "ds18b20.h"
#include "hih.h"
#include "FatFsAPI.h"
#include "rtc.h"

//#include "stm32f1xx_hal_adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void bluetoothCallback(void);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
volatile uint16_t adc[2] = {0,0};      // у нас два канала АЦП, поэтому массив из двух элементов
volatile uint8_t flag = 0;             // флаг окончания преобразования АЦП
char fileName[15]={0};
union Byte portOut;
union Byte portFlag;

extern uint8_t ds18b20_amount, disableBeep, topOwner, topUser, botUser, ok0, ok1, psword, pvAeration;
extern int16_t buf, valRun;

struct {
  uint8_t RXBuffer[2];
  uint8_t TXBuffer[2];
  uint8_t buf[60];
  uint8_t ind;
  uint8_t timeOut;
  uint8_t devOk;
} bluetoothData;

extern struct {
  uint8_t eepAddr;
  uint8_t sizeAddr;
  uint8_t pageSize;
} eepMem;

// -------- ISIDA ------
uint8_t getButton=0, modules=0, setup, servis, waitset, waitkey=WAITCOUNT;
int8_t countsec=-10, countmin, displmode, cardOk=0;
/*
ext[0] модуль Холла
ext[1] модуль ГОРИЗОНТА
ext[2] модуль СО2
ext[3] модуль заслонок
*/
int16_t pwTriac0, pwTriac1, pulsPeriod, beepOn, alarmErr; 
uint16_t currAdc, humAdc, statPw[2]; 
uint32_t summCurr=0, summPower=0, UnixTime;
/* ----------------------------- BEGIN point value -------------------------------------- */
union pointvalue{
  uint8_t pvdata[30];
  struct rampv {
   uint16_t cellID;               // 2 байт ind=0         сетевой номер и тип прибора
    int16_t pvT[MAX_DEVICES];     // 8 байт ind=2-ind=9   значения [MAX_DEVICES] датчиков температуры
    int16_t pvRH;                 // 2 байт ind=10;ind=11 значение датчика относительной влажности
    int16_t pvCO2[3];             // 6 байт ind=12-ind=17 значения датчика CO2 ---------- ИТОГО 18 bytes ------------
    uint8_t pvTimer;              // 1 байт ind=18        значение таймера
    uint8_t pvTmrCount;           // 1 байт ind=19        не используется !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t pvFlap;               // 1 байт ind=20        положение заслонки 
    uint8_t power;                // 1 байт ind=21        мощность подаваемая на тены
    uint8_t fuses;                // 1 байт ind=22        короткие замыкания 
    uint8_t errors, warning;      // 2 байт ind=23;ind=24 ошибки и предупреждения
    uint8_t cost0, cost1;         // 2 байт ind=25;ind=26 затраты ресурсов
    uint8_t date, hours;          // 2 байт ind=27;ind=28 счетчики суток и часов
    uint8_t other0;               // 1 байт ind=29        не используется !!!!!!!!!!!!!!!!!!!!!!!!!
  } pv;// ------------------ ИТОГО 30 bytes -------------------------------
} upv;
struct rampv *p_rampv = &upv.pv;

/* ----------------------------- BEGIN EEPROM ------------------------------------------- */
union serialdata{
  uint8_t data[EEP_DATA];
  struct eeprom {
    int16_t spT[2];     // 4 байт ind=0-ind=3   Уставка температуры sp[0].spT->Сухой датчик; sp[1].spT->Влажный датчик
    int16_t spRH[2];    // 4 байт ind=4-ind=7   sp[0].spRH->ПОДСТРОЙКА HIH-5030; sp[1].spRH->Уставка влажности Датчик HIH-5030
    int16_t K[2];       // 4 байт ind=8-ind=11  пропорциональный коэфф.
    int16_t Ti[2];      // 4 байт ind=12-ind=15 интегральный коэфф.
    int16_t minRun;     // 2 байт ind=16;ind=17 импульсное управление насосом увлажнителя
    int16_t maxRun;     // 2 байт ind=18;ind=19 импульсное управление насосом увлажнителя
    int16_t period;     // 2 байт ind=20;ind=21 импульсное управление насосом увлажнителя
    int16_t TimeOut;    // 2 байт ind=22;ind=23 время ожидания начала режима охлаждения
    int16_t EnergyMeter;// 2 байт ind=24;ind=25 счетчик элктрической энергии  ----------- ИТОГО 26 bytes ------------
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
    uint8_t KoffCurr;   // 1 байт ind=46;       маштабный коэф. по току симистора  (160 для AC1010 или 80 для другого)
    uint8_t coolOn;     // 1 байт ind=47;       не используется !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t coolOff;    // 1 байт ind=48;       не используется !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t Zonality;   // 1 байт ind=49;       порог зональности в камере ----------- 24 bytes ------------
  } sp;                 // ------------------ ИТОГО 50 bytes -------------------------------
} eep;
struct eeprom *p_eeprom = &eep.sp;
/* ------------------------------ END EEPROM -------------------------------------------- */
#include "output.h"
#include "displ.h"
void temperature_check(struct rampv *ram);
void am2301_port_init(void);
uint8_t am2301_Start(void);
uint8_t am2301_Read(struct rampv *ram, uint8_t biasHum);
void display(struct eeprom *t, struct rampv *ram);
void display_setup(struct eeprom *t);
void display_servis(struct rampv *ram);
void displ_3(int16_t val, int8_t mode, int8_t blink);
void rotate_trays(uint8_t timer0, uint8_t timer1, struct rampv *ram);
uint8_t sethorizon(uint8_t timer0, uint8_t TurnTime, struct rampv *ram);
int8_t chkflap(uint8_t cmd, uint8_t *pvF);
uint8_t readCO2(struct rampv *ram);
//------
void saveservis(struct eeprom *t);
void saveset(struct eeprom *t);
uint8_t reset(uint16_t memAddr, uint8_t *data);
void checkkey(struct eeprom *t, int16_t pvT0);
//void pushkey(void);
void chkdoor(struct eeprom *t, struct rampv *ram);
void init(struct eeprom *t, struct rampv *ram);
uint8_t SD_write (const char* flname, struct eeprom *t, struct rampv *ram);
uint8_t bluetoothName(uint8_t number);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  int8_t tmpbyte;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  sysTick_Init();   // 1mS
  eepMem.eepAddr = (0x50 << 1);  // HAL expects address to be shifted one bit to the left
  eepMem.sizeAddr = I2C_MEMADD_SIZE_8BIT;
  eepMem.pageSize = 16;          // AT24C04A или AT24C08A. The 4K/8K EEPROM is capable of 16-byte page writes
  portOut.value = 0;
  portFlag.value = 0;
  topOwner=MAXOWNER;
  topUser=TOPUSER;
  botUser=BOTUSER;
  CHECK = 1;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  set_Output();
	HAL_TIM_Base_Start_IT(&htim4);	/* ------  таймер 1Гц.   период 1000 мс.  ----*/
	HAL_TIM_Base_Start_IT(&htim3);	/* ------  таймер 6Гц.   период 166 мс.  ----*/
  HAL_ADCEx_Calibration_Start(&hadc1);            // калибровкa АЦП
  HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2);
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);  // LED Off
  //--------- ХОЛОСТОЕ ВЫПОЛНЕНИЕ при котором каналы распологаются правильно --------
//  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 2);  // стартуем АЦП
//  while(flag==0);
//  flag = 0;
  //----------- Теперь каналы сдвинуты. Последний стал первым -----------------------
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 2);  // стартуем АЦП
  while(flag==0);
  flag = 0;
  currAdc = adc[0]; humAdc = adc[1];
  adc[0] = 0; adc[1] = 0;
  //******************************************************************************  
  HAL_RTC_WaitForSynchro(&hrtc);                      // Після увімкнення, пробудження, скидання, потрібно викликати цю функцію  
  if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1) == 0){     // Перевіряємо чи дата вже була збережена чи ні
   // як ні, то задамо якусь початкову дату і час
    setDataAndTime(22,RTC_MONTH_OCTOBER,0x01,RTC_WEEKDAY_SATURDAY,0,0,0,RTC_FORMAT_BIN);//2022,RTC_MONTH_OCTOBER,01,RTC_WEEKDAY_SATURDAY  00:00:00
    writeDateToBackup(RTC_BKP_DR1);       // і запишемо до backup регістрів дату
  }
  else {
    readBackupToDate(RTC_BKP_DR1);        // выполним коррекцию даты
    writeDateToBackup(RTC_BKP_DR1);       // сохраним обновленную дату
  }
  HAL_RTCEx_SetSecond_IT(&hrtc);          // Sets Interrupt for second
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  sprintf(fileName,"%02u_%02u_%02u.txt",sDate.Year,sDate.Month,sDate.Date);
  //******************************************************************************
  for(int8_t i=0;i<8;i++) {setChar(i,SIMBL_BL); PointOn(i); LedOn(i,3);}// "BL"+точки
  SendDataTM1638();
  SendCmdTM1638(0x8F);  // Transmit the display control command to set maximum brightness (8FH)
  //---- проверка EEPROM ---------------
  tmpbyte = eep_read(0x0000, eep.data);
  if(tmpbyte){              // бесконечный цикл НЕИСПРАВНА EEPROM "EEP-x" (HAL_ERROR=0x01U, HAL_BUSY=0x02U, HAL_TIMEOUT=0x03U)
      while(1){
          HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);    // Beeper On
          HAL_Delay(200);
          HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);  // Beeper Off
          HAL_Delay(200); 
      }
  }
  if (eep.sp.identif == 0 || eep.sp.identif > 30) eep_initial(0x0000, eep.data);// сброс к заводским настройкам

  init(&eep.sp, &upv.pv);   // инициализация аппаратной части и подключаемых модулей
  temperature_check(&upv.pv);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  getButton = waitkey/4;
	while (1)
	{
  //----------------------------------- Теперь будем опрашивать три канала на одном АЦП с помощью DMA… -------------------------------------------

  /* ------------------------------------------- BEGIN таймер TIM3 6 Гц. ----------------------------------------------------------------------- */
      if (getButton>waitkey/4) checkkey(&eep.sp, upv.pv.pvT[0]);  // клавиатура
  /* -------------------------------------------- END таймер TIM3 6 Гц. ------------------------------------------------------------------------ */

  /* ------------------------------------------- BEGIN таймер TIM4 1 Гц. ----------------------------------------------------------------------- */
      if(CHECK){   // ------- новая секунда --------------------------------------------------------------
        CHECK=0; DISPLAY=1; ALARM=0; upv.pv.errors=0; upv.pv.warning=0; upv.pv.pvTmrCount = countsec;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 2);
        while(flag==0);
        flag = 0;
        currAdc = adcTomV(adc[0]);                  // Channel 8 (Port B0) в мВ.
//        if(HIH5030) humAdc  = adcTomV(adc[1]);      // Channel 9 (Port B1) в мВ.
        humAdc  = adcTomV(adc[1]);
        adc[0] = 0; adc[1] = 0;
// ----------- ************************************** --------------------------------------------------
        //-- перевіримо чи настав інший день --------------
        if (((sTime.Hours+sTime.Minutes+sTime.Seconds)<=4)){  //??????????????????????????????????
          writeDateToBackup(RTC_BKP_DR1);             // сохраним обновленную дату
          sprintf(fileName,"%02u_%02u_%02u.txt",sDate.Year,sDate.Month,sDate.Date);
        }
        //-------------------------------------------------
        temperature_check(&upv.pv);
        if(AM2301) am2301_Read(&upv.pv, eep.sp.spRH[0]);
        else if(HIH5030){ 
           if(humAdc > 500){                          // humAdc => 500 mV для RH=0%
              upv.pv.pvRH = mVToRH(humAdc, eep.sp.spRH[0], upv.pv.pvT[0]);  // перевод в десятичное значение относительной влажности (%)
           }
           else upv.pv.pvRH = 255;
        }
  //---------------------- Состояние дверей; "подгототка к ОХЛАЖДЕНИЮ"; "подгототка к ВКЛЮЧЕНИЮ" --------------------------------------------------
        chkdoor(&eep.sp, &upv.pv);
        if((eep.sp.condition&0x18)==0x08) eep.sp.condition|= sethorizon(eep.sp.timer[0], eep.sp.TurnTime, &upv.pv);  // Установка в горизонтальное положение
  //------------------------------------------ Опрос модулей расширения ---------------------------------------------------------------------------
        if(modules&1) {
          tmpbyte = chkcooler(eep.sp.condition);
          if(tmpbyte<0) upv.pv.errors |= 0x40;        // Отказ модуля Холла
          else if(tmpbyte>0) upv.pv.warning |= 0x20;  // ОСТАНОВ тихоходного вентилятора
        }
        if(modules&2) {
          tmpbyte = chkhorizon(eep.sp.condition);
          if(tmpbyte<0) upv.pv.errors |= 0x40; // Отказ модуля ГОРИЗОНТ
          else if(tmpbyte>0) upv.pv.warning |= 0x40;  // НЕТ ПОВОРОТА лотков
        }
        if(modules&4) {
          tmpbyte = readCO2(&upv.pv);
          if(tmpbyte<0) upv.pv.errors |= 0x20;        // Отказ модуля CO2
        }
        if(modules&8) {
          tmpbyte = chkflap(0,&upv.pv.pvFlap);
          if(tmpbyte<0) upv.pv.errors |= 0x20;        // Отказ модуля заслонок
        }
  //------------------------------------------ КАМЕРА ВКЛЮЧЕНА в работу ---------------------------------------------------------------------------
        if (eep.sp.condition&1){
          // --------------------------------------- НАГРЕВАТЕЛЬ -----------------------------------------------------------------
          if(HAL_GPIO_ReadPin(OVERHEAT_GPIO_Port, OVERHEAT_Pin)==GPIO_PIN_RESET) upv.pv.errors |= 0x10;  // ПЕРЕГРЕВ СИМИСТОРА !!!
          else if((upv.pv.warning&0x20)==0){    // НЕТ ОСТАНОВА тихоходного вентилятора !!!
            if(upv.pv.pvT[0] < 850){
              int16_t err = eep.sp.spT[0] - upv.pv.pvT[0];
              if(heatCondition(err, eep.sp.alarm[0], eep.sp.extOn[0])) upv.pv.warning |= 0x01;  // ОТКЛОНЕНИЕ по температуре
              pwTriac0 = heater(err, &eep.sp);
              if(pwTriac0) HEATER = 1;  // HEATER On
            }
            else upv.pv.errors |= 0x01;   // ОШИБКА ДАТЧИКА температуры !!!
          }
          upv.pv.power = pwTriac0 / 10;
          if(upv.pv.power > 100) upv.pv.power = 100;
          statPw[0] += upv.pv.power;      // расчет эконометра
          // --------------------------------------- УВЛАЖНИТЕЛЬ -----------------------------------------------------------------
          if(ok0&1){  // отключение УВЛАЖНЕНИЯ при РАЗОГРЕВЕ и ПЕРЕОХЛАЖДЕНИИ
            if(HIH5030||AM2301){  // подключен электронный датчик влажности
              int16_t err = eep.sp.spRH[1] - upv.pv.pvRH;
              if(humCondition(err, eep.sp.alarm[1], eep.sp.extOn[1])) upv.pv.warning |= 0x02; // ОТКЛОНЕНИЕ по влажности
              // релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
              if(eep.sp.relayMode==4) valRun = humidifier(err, &eep.sp);
              else {
                pwTriac1 = humidifier(err, &eep.sp);
                if(pwTriac1 && (upv.pv.fuses&0x01)==0) HUMIDI = 1;  // HUMIDIFIER On
              }
            }
            else if(upv.pv.pvT[1] < 850){
              int16_t err = eep.sp.spT[1] - upv.pv.pvT[1];
              if(humCondition(err, eep.sp.alarm[1], eep.sp.extOn[1])) upv.pv.warning |= 0x02; // ОТКЛОНЕНИЕ по влажности
              // релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
              if(eep.sp.relayMode==4) valRun = humidifier(err, &eep.sp);
              else {
                pwTriac1 = humidifier(err, &eep.sp);
                if(pwTriac1 && (upv.pv.fuses&0x01)==0) HUMIDI = 1;  // HUMIDIFIER On
              }
            }
            else upv.pv.errors |= 0x02;   // ОШИБКА ДАТЧИКА влажности !!!
            tmpbyte = pwTriac1 / 10;
            if(tmpbyte > 100) tmpbyte = 100;
            statPw[1] += tmpbyte;         // расчет эконометра
          }
          // --------------------------------------- ОХЛАЖДЕНИЕ -----------------------------------------------------------------
          if(upv.pv.warning&0x20) tmpbyte = ON;     // ОСТАНОВ тихоходного вентилятора
          else {
            if(upv.pv.pvT[0] < 850){
              int16_t err = eep.sp.spT[0] - upv.pv.pvT[0];
              tmpbyte = RelayNeg(err, 0, eep.sp.extOn[0],eep.sp.extOff[0]);// доп. канал -> охлаждение
            }
            else upv.pv.errors |= 0x01;             // ОШИБКА ДАТЧИКА температуры !!!
          }
          if(upv.pv.fuses&0x02) tmpbyte = OFF;      // ПРЕДОХРАНИТЕЛЬ доп. канал №1
          switch (tmpbyte){
            case ON:  FLAP = ON;  upv.pv.pvFlap = FLAPOPEN;  if(modules&8) chkflap(SETFLAP,  &upv.pv.pvFlap); break;// установка заслонки
            case OFF: FLAP = OFF; upv.pv.pvFlap = FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &upv.pv.pvFlap); break;// установка заслонки; сброс флага запрещения принудительной подачи воды
          }
          // --------------------------------------- ВСПОМОГАТЕЛЬНЫЙ -----------------------------------------------------------------
          extra_2(&eep.sp, &upv.pv);
  //---------------------------------------- ЗОНАЛЬНОСТЬ температуры камеры -----------------------------------------------------------------------
            if(ok0){
                if(HIH5030||AM2301) {if(ds18b20_amount>1) {if(abs(upv.pv.pvT[0]-upv.pv.pvT[1])>eep.sp.Zonality) upv.pv.warning |=0x08;}} // Большой перепад температур.
                else     {if(ds18b20_amount>2) {if(abs(upv.pv.pvT[0]-upv.pv.pvT[2])>eep.sp.Zonality) upv.pv.warning |=0x08;}};// Большой перепад температур.
            }
            if(!(HIH5030||AM2301) && (upv.pv.pvT[1]-upv.pv.pvT[0])>20){
              if(upv.pv.pvT[1] < 850){
                upv.pv.warning =0x10; 					// Неправильная конфигурация датчиков !!
                pwTriac0 = 500;
              }
              else upv.pv.errors |= 0x02;   // ОШИБКА ДАТЧИКА влажности !!!
            }
  //--------------------------------------- ПОВОРОТ ЛОТКОВ Статистика камеры ----------------------------------------------------------------------
            if(eep.sp.KoffCurr){
            // конверсия mV в mA ->100mV/1A+1.65V. (5A->0.5+1.65=2.15; 10A->2.65; 20A->3.65) добавляем делитель 110k/68k на 2,65 и получаем 10А = 1В = 1000 мВ
              currAdc *= eep.sp.KoffCurr; currAdc /= 100;
              summCurr += currAdc;                                        // суммирование тока симистора каждую секунду
              if(upv.pv.power==100 && countsec>=0 && currAdc<100) upv.pv.errors|=0x08;  // если ток < 1,0 А. -> НЕИСПРАВНА цепь НАГРЕВАТЕЛЯ 
            }
            else {currAdc = 0; summCurr = 0;}
            // АНАЛИЗ УХУДШЕНИЯ АВАРИЙНОЙ СИТУАЦИИ (важно в этом месте после анализа мощности)
            int16_t newErr = abs(eep.sp.spT[0]-upv.pv.pvT[0]);
            if((upv.pv.warning & 3)&&(newErr-alarmErr)>2) disableBeep=0;  // если при блокироке сирены продолжает увеличиватся ошибка сброс блокировки
            if(countsec>59){
              ++countmin; countsec=0; if (disableBeep) disableBeep--;
              if(cardOk) SD_write(fileName, p_eeprom, p_rampv); else My_LinkDriver();  // запись на SD если КАМЕРА ВКЛЮЧЕНА в работу
              if(!(eep.sp.condition&0x18)) rotate_trays(eep.sp.timer[0], eep.sp.timer[1], &upv.pv);  // выполняется только если Камера ВКЛ.
              if(upv.pv.pvCO2[0]>0) CO2_check(eep.sp.spCO2, eep.sp.spCO2, upv.pv.pvCO2[0]); // Проверка концентрации СО2
              else if(eep.sp.air[1]>0) aeration_check(eep.sp.air[0], eep.sp.air[1]);    // Проветривание выполняется только если air[1]>0
              statPw[0]/=60; statPw[1]/=60;     // расчет затрат ресурсов
              upv.pv.cost0=statF2(0, statPw[0]); upv.pv.cost1=statF2(1, statPw[1]); // расчет затрат ресурсов
              statPw[0]=0; statPw[1]=0;         // расчет затрат ресурсов
              summCurr *= 22; summCurr /= 6;    // summCurr*220V./60 sec.
              summPower += summCurr/60;         // суммируем в мВт.
              summCurr = 0;
              if(countmin>59){
                countmin = 0; upv.pv.date = sDate.Date; upv.pv.hours = sTime.Hours;
                eep.sp.EnergyMeter += (summPower/100);// суммируем в Вт.
                summPower = 0;
                EEPSAVE=1; waitset=1;
              }
            } 
        }
  //---------------------------------------------- КАМЕРА ОТКЛЮЧЕНА -------------------------------------------------------------------------------
        else if((eep.sp.condition&7)==0){
            if(eep.sp.relayMode==4) valRun = 0;                            // ОТКЛЮЧИТЬ импульсное управление увлажнителем
            if(servis){                                                   // включен СЕРВИСНЫЙ режим
              switch (servis){
                 case 1: pwTriac0=MAXPULS; portOut.value = 0x01; break;   // НАГРЕВАТЕЛЬ
                 case 2: pwTriac1=MAXPULS; portOut.value = 0x02; break;   // УВЛАЖНИТЕЛЬ
                 case 3: portOut.value = 0x04; upv.pv.pvFlap = FLAPOPEN; if(modules&8) chkflap(SETFLAP, &upv.pv.pvFlap); break; // ПРОВЕТРИВАНИЕ, СЕРВОПРИВОД 90грд.
                 case 4: portOut.value = 0x08; break;                     // ДОПОЛНИТЕЛЬНЫЙ КАНАЛ
                 case 5: portOut.value = 0x10; break;                     // ЛОТКИ ВВЕРХ
                 default: portOut.value = 0; upv.pv.pvFlap=FLAPCLOSE;
                          if(modules&8) chkflap(DATAREAD, &upv.pv.pvFlap);// ВСЕ ОТКЛЮЧЕНО, СЕРВОПРИВОД 0грд.
              }
            }
            else {
               upv.pv.power=OFF; portOut.value &= 0x10; upv.pv.pvFlap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &upv.pv.pvFlap); VENTIL = OFF;
               if(currAdc>1000){upv.pv.errors|=0x04;}   // если сила тока > 1000 mV ПРОБОЙ СИМИСТОРА!
               if(countsec>59){
                countsec=0; upv.pv.date = sDate.Date; upv.pv.hours = sTime.Hours;// ???????????????????????????????
                if(cardOk) SD_write(fileName, p_eeprom, p_rampv); else My_LinkDriver();  // ????????????????????????????????????????????
                if(eep.sp.condition&0x80) rotate_trays(eep.sp.timer[0], eep.sp.timer[1], &upv.pv); // Поворот лотков при ОТКЛЮЧЕННОЙ камере
               }
            }
        }
 //--------------------------------------------------------------------------------------------------------------------------------------------       
        if(waitset){
          if(--waitset==0){
            if(EEPSAVE) eep_write(0x0000, eep.data);                                      // запись в энергонезависимую память
            if(servis==7) bluetoothName(eep.sp.identif);                                  // коррекция Broadcast name 
            servis=0;setup=0;displmode=0;psword=0;buf=0;topUser=TOPUSER;botUser=BOTUSER;} // возвращяемся к основному экрану, сброс пароля 
        }
        if(TURN && eep.sp.timer[1]){if(--upv.pv.pvTimer==0) { upv.pv.pvTimer=eep.sp.timer[0]; TURN = OFF;}} // только при sp[1].timer>0 -> асиметричный режим
        // ---------------------------------------------------------------------------------------------------------------------------------------------------
        if(HAL_GPIO_ReadPin(Bluetooth_STATE_GPIO_Port, Bluetooth_STATE_Pin)){ // если есть подключение по Bluetooth
          HAL_UART_Transmit(&huart1,(uint8_t*)upv.pvdata,30,0x1000);
          HAL_UART_Transmit(&huart1,(uint8_t*)eep.data,50,0x1000);
//          for(tmpbyte=0;tmpbyte<60;tmpbyte++){bluetoothData.buf[tmpbyte]=tmpbyte;}
//          HAL_UART_Transmit(&huart1,(uint8_t*)bluetoothData.buf,60,0x1000);
          bluetoothData.TXBuffer[0] = 0x0D;  bluetoothData.TXBuffer[1] = 0x0A;  // "\r=<CR>-\n=<LF>"
          HAL_UART_Transmit(&huart1,(uint8_t*)bluetoothData.TXBuffer,2,0x1000);
        }
//        else if(rx_buffer_overflow) check_command();      // если установлен флаг - "комманда компьютера"
      }
      /* ---------------- ИНДИКАЦИЯ ------------------------------------------------- */
      if(DISPLAY){
        DISPLAY = 0;
        if(setup) display_setup(&eep.sp);
        else if(servis) display_servis(&upv.pv);
        else display(&eep.sp, &upv.pv);
        ledOut(eep.sp.condition, upv.pv.warning, upv.pv.fuses); SendDataTM1638(); set_Output();
      }
  /* -------------------------------------------- END таймер TIM4 1 Гц. ------------------------------------------------------------------------ */     
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      if(pwTriac0 < 0) {pwTriac0=0; HEATER = 0; LEDOFF = 1;}  // HEATER Off
      if(pwTriac1 < 0) {pwTriac1=0; HUMIDI = 0; LEDOFF = 1;}  // HUMIDIFIER Off
      if(eep.sp.relayMode==4){                                 // импульсный режим работы насоса
        if(pulsPeriod < 0){
          pulsPeriod = eep.sp.period;
          if(pwTriac1 && (upv.pv.fuses&0x01)==0) {pwTriac1=valRun; HUMIDI = 1; LEDOFF = 1;}  // HUMIDIFIER On
        }
      }
      if(LEDOFF) {LEDOFF = 0; ledOut(eep.sp.condition, upv.pv.warning, upv.pv.fuses); SendDataTM1638(); set_Output();}
      if(beepOn < 0) {beepOn=0; HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);}  // Beeper Off
      if(bluetoothData.ind == 0){
        bluetoothData.timeOut=0;
        HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2); // запуск приема
      }
      else if(bluetoothData.timeOut >= 10) {
        // ошибка таймаута больше 10 mS.
        HAL_UART_AbortReceive_IT(&huart1); // остановка приема
        bluetoothData.ind = 0; // признак ожидание первого байта
        bluetoothData.timeOut = 0;
//        HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2); // запуск приема
      }
	} // ---------------------------------- END while (1) ------------------------------------------------------------------------------------------
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_1LINE;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_LSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 2999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2399;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 23999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 2999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SD_CS_Pin|DE485_Pin|Beeper_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OUT_RCK_Pin|DISPL_STB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(cmdH_05_GPIO_Port, cmdH_05_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FUSE2_Pin FUSE3_Pin FUSE4_Pin FUSE5_Pin */
  GPIO_InitStruct.Pin = FUSE2_Pin|FUSE3_Pin|FUSE4_Pin|FUSE5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_CS_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OUT_RCK_Pin */
  GPIO_InitStruct.Pin = OUT_RCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(OUT_RCK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DISPL_STB_Pin */
  GPIO_InitStruct.Pin = DISPL_STB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(DISPL_STB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Bluetooth_STATE_Pin */
  GPIO_InitStruct.Pin = Bluetooth_STATE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Bluetooth_STATE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DE485_Pin */
  GPIO_InitStruct.Pin = DE485_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(DE485_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : AM2301_Pin */
  GPIO_InitStruct.Pin = AM2301_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(AM2301_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Beeper_Pin */
  GPIO_InitStruct.Pin = Beeper_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Beeper_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Door_Pin OVERHEAT_Pin OneW_2R_Pin OneWR_1_Pin */
  GPIO_InitStruct.Pin = Door_Pin|OVERHEAT_Pin|OneW_2R_Pin|OneWR_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : cmdH_05_Pin */
  GPIO_InitStruct.Pin = cmdH_05_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(cmdH_05_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){  
  if(hadc->Instance == ADC1) flag = 1;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  if(huart==&huart1) bluetoothCallback();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
