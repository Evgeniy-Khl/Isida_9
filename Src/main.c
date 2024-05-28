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
  Program Size: Code=43228 RO-data=1844 RW-data=216 ZI-data=3656  
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
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
volatile uint16_t adc[3] = {0,0,0};    // � ��� ��� ������ ���, ������� ������ �� ���� ���������
volatile uint8_t flag = 0;             // ���� ��������� �������������� ���
char fileName[9]={0};
union Byte portOut;
union Byte portFlag;
union d4v crc;
union d4v file;

extern uint8_t ds18b20_amount, disableBeep, topOwner, topUser, botUser, ok0, ok1, psword, pvAeration;
extern int16_t buf;
extern uint16_t valRun;
extern struct rs485 rs485Data;
extern struct bluetooth bluetoothData;

// -------- ISIDA ------
uint8_t getButton=0, modules=0, setup, servis, waitset, waitkey=WAITCOUNT;
int8_t countsec=-10, countmin, displmode, cardOk=0;
/*
ext[0] ������ �����
ext[1] ������ ���������
ext[2] ������ ��2
ext[3] ������ ��������
*/
int16_t pwTriac0, pwTriac1, pulsPeriod, beepOn, alarmErr; 
uint16_t currAdc, humAdc, coolerAdc, pvCurrent, statPw[2]; 
uint32_t unixTime;
/* ----------------------------- BEGIN point value -------------------------------------- */
union pointvalue{
  uint8_t pvdata[20];
  struct rampv {
    uint8_t model;       // 1 ���� ind=0  ������ �������
    uint8_t node;        // 1 ���� ind=1  ������� ����� �������
    int16_t pvT[MAX_DEVICES]; // 6 ���� ind=2-ind=9   �������� [MAX_DEVICES=3] �������� �����������
    uint8_t pvRH;        // 1 ���� ind=8  �������� ������� ������������� ���������
    uint8_t pvCO2;       // 1 ���� ind=9  �������� ������� CO2 (250*20=5000 / 20*20=400)
    uint8_t nextTurn;    // 1 ���� ind=10 �������� ������� �� ������ �������� ������
    uint8_t fan;         // 1 ���� ind=11 �������� �������� ����������� �����������
    uint8_t flap;        // 1 ���� ind=12 ��������� �������� 
    uint8_t power;       // 1 ���� ind=13 �������� ���������� �� ����
    uint8_t fuses;       // 1 ���� ind=14 �������� ��������� 
    uint8_t errors;      // 1 ���� ind=15 ������
    uint8_t warning;     // 1 ���� ind=16 ��������������
    uint8_t yearMonth;   // 1 ���� ind=17 YYMM = 2405
    uint8_t dayHour;     // 1 ���� ind=18 DDHH = 2209
    uint8_t minSec;      // 1 ���� ind=19 MMSS = 0728
  } pv;// ------------------ ����� 20 bytes -------------------------------
} upv;
struct rampv *p_rampv = &upv.pv;
uint8_t *p_ramdata = upv.pvdata;
/* ----------------------------- BEGIN EEPROM ------------------------------------------- */
union serialdata{
  uint8_t data[EEP_DATA];
  struct eeprom {
    int16_t spT[2];     // 4 ���� ind=0-ind=3   ������� ����������� spT[0]->����� ������; spT[1]->������� ������
    //spRH[0]->���������� HIH-5030/AM2301 
    //spRH[1]->������� ��������� ������ HIH-5030/AM2301 (����� 0111 1111 6,7 ��� ��� ���������� ������� HIH-5030,AM2301);
    int8_t  spRH[2];    // 2 ���� ind=4;ind=5
    uint8_t state;      // 1 ���� ind=6;        ��������� ������ (����. ���. ����������, � �.�.)
    uint8_t extendMode; // 1 ���� ind=7;        ����������� ����� ������  0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
    uint8_t relayMode;  // 1 ���� ind=8;        �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1] 4->�� ���.[1] ���������� �����
    uint8_t programm;   // 1 ���� ind=9;        ������ �� ���������
    uint8_t minRun;     // 1 ���� ind=10        ���������� ���������� ������� �����������
    uint8_t maxRun;     // 1 ���� ind=11        ���������� ���������� ������� �����������
    uint8_t period;     // 1 ���� ind=12        ���������� ���������� ������� �����������
    uint8_t timer[2];   // 2 ���� ind=13;ind=14 [0]-������.��������e [1]-�����.��������e
    uint8_t alarm[2];   // 2 ���� ind=15;ind=16 ������ 5 = 0.5 ��.C
    uint8_t extOn[2];   // 2 ���� ind=17;ind=18 �������� ��� ���. ���������������� ������
    uint8_t extOff[2];  // 2 ���� ind=19;ind=20 �������� ��� ����. ���������������� ������
    uint8_t air[2];     // 2 ���� ind=21;ind=22 ������ ������������� air[0]-�����; air[1]-������; ���� air[1]=0-���������
    uint8_t spCO2;      // 1 ���� ind=23;       ������� �������� ��� ���������� ������������ ��2
    uint8_t koffCurr;   // 1 ���� ind=24;       ��������� ����. �� ���� ���������  (150 ��� AC1010)
    uint8_t hysteresis; // 1 ���� ind=25;       ���������� ������ ���������� ����� 0x03; ���������� ������������� HIH-5030 ����� 0x40; AM2301 ����� 0x80;
    uint8_t zonaFlap;   // 1 ���� ind=26;       ����� ����������� � ������
    uint8_t turnTime;   // 1 ���� ind=27;       ����� �������� ������� ������ � ��������
    uint8_t waitCooling;// 1 ���� ind=28        ����� �������� ������ ������ ����������
    uint8_t pkoff[2];   // 2 ���� ind=29;ind=30 ���������������� �����.
    uint8_t ikoff[2];   // 2 ���� ind=31;ind=32 ������������ �����.
    uint8_t identif;    // 1 ���� ind=33;       ������� ����� �������
    uint8_t ip[4];      // 4 ���� ind=34;ind=35;ind=36;ind=37; IP MQTT broker [192.168.100.100]
    uint8_t nothing0;   // 1 ���� ind=38;       �� ������������ ! YYMM = 2405
    uint8_t nothing1;   // 1 ���� ind=39;       �� ������������ ! DDHH = 2209
  } sp;                 // ------------------ ����� 40 bytes -------------------------------
} eep;
struct eeprom *p_eeprom = &eep.sp;
uint8_t *p_eepdata = eep.data;
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
uint8_t sethorizon(uint8_t timer0, uint8_t turnTime, struct rampv *ram);
int8_t chkflap(uint8_t cmd, uint8_t *pvF);
uint8_t readCO2(struct rampv *ram);
//------
void saveservis(struct eeprom *t);
void saveset(struct eeprom *t);
uint8_t reset(uint16_t memAddr, uint8_t *data);
//void checkkey(struct eeprom *t, int16_t pvT0);
void checkkey(struct eeprom *t, struct rampv *ram);
//void pushkey(void);
void chkdoor(struct eeprom *t, struct rampv *ram);
void init(struct eeprom *t, struct rampv *ram);
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
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void rs485Callback(uint8_t *p_ramdata, uint8_t *p_eepdata);
uint8_t SD_write (const char* flname, struct eeprom *t, struct rampv *ram);
uint8_t bluetoothName(uint8_t number);
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
  sysTick_Init();   // ������������� �������� SysTick Reload Register (LOAD) ��� ��������� ���������� ������ ������������ (1mS)

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
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  COOLER = 1;     // �������� ����������
  set_Output();   // ������ � ���������� 74HC595D
	HAL_TIM_Base_Start_IT(&htim4);	/* ------  ������ 1��.   ������ 1000 ��.  ----*/
	HAL_TIM_Base_Start_IT(&htim3);	/* ------  ������ 200��.  ������ 5 ��.  --������ 10��.  ������ 100 ��.--*/
  HAL_ADCEx_Calibration_Start(&hadc1);            // ���������a ���
  HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2);
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);  // LED Off
  //--------- �������� ���������� ��� ������� ������ ������������� ��������� --------
//  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 3);  // �������� ���
//  while(flag==0);
//  flag = 0;
  //----------- ������ ������ ��������. ��������� ���� ������ -----------------------
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 3);  // �������� ���
  while(flag==0);
  flag = 0;
  currAdc = adc[0]; humAdc = adc[1]; coolerAdc = adc[2];
  adc[0] = 0; adc[1] = 0;
  //******************************************************************************  
  HAL_RTC_WaitForSynchro(&hrtc);                      // ϳ��� ���������, �����������, ��������, ������� ��������� �� �������  
  if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1) == 0){     // ���������� �� ���� ��� ���� ��������� �� �
   // �� �, �� ������ ����� ��������� ���� � ���
    setDataAndTime(24,RTC_MONTH_JUNE,0x01,RTC_WEEKDAY_SATURDAY,0,0,0,RTC_FORMAT_BIN);//2024,RTC_MONTH_JUNE,01,RTC_WEEKDAY_SATURDAY  00:00:00
    writeDateToBackup(RTC_BKP_DR1);       // � �������� �� backup ������� ����
  }
  else {
    readBackupToDate(RTC_BKP_DR1);        // �������� ��������� ����
    writeDateToBackup(RTC_BKP_DR1);       // �������� ����������� ����
  }
  HAL_RTCEx_SetSecond_IT(&hrtc);          // Sets Interrupt for second
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  sprintf(fileName,"%02u_%02u_%02u",sDate.Year,sDate.Month,sDate.Date);
  //******************************************************************************
  for(int8_t i=0;i<8;i++) {setChar(i,SIMBL_BL); PointOn(i); LedOn(i,3);}// "BL"+�����
  SendDataTM1638();
  SendCmdTM1638(0x8F);  // Transmit the display control command to set maximum brightness (8FH)
  //---- �������� EEPROM ---------------
  tmpbyte = eep_read(0x0000, eep.data);
  if(tmpbyte){              // ����������� ���� ���������� EEPROM "EEP-x" (HAL_ERROR=0x01U, HAL_BUSY=0x02U, HAL_TIMEOUT=0x03U)
      while(1){
          HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);    // Beeper On
          HAL_Delay(200);
          HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);  // Beeper Off
          HAL_Delay(200); 
      }
  }
  // *** -------- ����� � ��������� ���������� ------------- ***
  if (eep.sp.identif == 0) eep_initial(0x0000, eep.data);

  init(&eep.sp, &upv.pv);   // ������������� ���������� ����� � ������������ �������
  temperature_check(&upv.pv);
  upv.pv.pvT[0] = eep.sp.spT[0]-10;//?????????????????????????????????????????????????????
  upv.pv.pvT[1] = eep.sp.spT[1]-10;//?????????????????????????????????????????????????????
  crc.data[2] = 0x0D; crc.data[3] = 0x0A; // "[0x0D]->\r=<CR>; [0x0A]->\n=<LF>"
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  getButton = waitkey/4;
	while (1){
  //----------------------------------- ������ ����� ���������� ��� ������ �� ����� ��� � ������� DMA� -------------------------------------------

  /*** ------------------------------------------- BEGIN ������ TIM3 6 ��. ----------------------------------------------------------------------- ***/
      if (getButton>waitkey/4) checkkey(&eep.sp, &upv.pv);  // ���������� waitkey=WAITCOUNT=16 ������������ ����� ����� �������� �� ������

  /*** ------------------------------------------- BEGIN ������ TIM4 1 ��. ----------------------------------------------------------------------- ***/
      if(CHECK){   // ------- ����� ������� --------------------------------------------------------------
        CHECK=0; DISPLAY=1; ALARM=0; upv.pv.errors=0; upv.pv.warning=0;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 3);
        while(flag==0);
        flag = 0;
        currAdc = adcTomV(adc[0]);      // Channel 8 (Port B0) � ��.
        humAdc  = adcTomV(adc[1]);      // Channel 9 (Port B1) � ��.
//        if(HIH5030) humAdc  = adcTomV(adc[1]);// Channel 9 (Port B1) � ��.
        coolerAdc = adcTomV(adc[2]/10); // Channel 4 (Port A4) � 1550��./10 = 155 ������
        
        adc[0] = 0; adc[1] = 0; adc[2] = 0;

    //-------------------- ��������� �� ������ ����� ���� ------------
        if (((sTime.Hours+sTime.Minutes+sTime.Seconds)<=4)){
          writeDateToBackup(RTC_BKP_DR1);             // �������� ����������� ����
          sprintf(fileName,"%02u_%02u_%02u",sDate.Year,sDate.Month,sDate.Date);
        }
    //-------------------- ����������� � ��������� --------------------

//        temperature_check(&upv.pv);
        if(AM2301) am2301_Read(&upv.pv, eep.sp.spRH[0]);
        else if(HIH5030){ 
           if(humAdc > 500){                                                // humAdc = 500 mV ��� RH=0%
              upv.pv.pvRH = mVToRH(humAdc, eep.sp.spRH[0], upv.pv.pvT[0]);  // ������ ������������� ��������� % (��.,���������,�����������)
           }
           else upv.pv.pvRH = 200;
        }
        else upv.pv.pvRH = 0;
    //-------------------- ��������� ������; "���������� � ����������"; "���������� � ���������" --
        chkdoor(&eep.sp, &upv.pv);
        if((eep.sp.state&0x18)==0x08) eep.sp.state|= sethorizon(eep.sp.timer[0], eep.sp.turnTime, &upv.pv);  // ��������� � �������������� ���������
    //-------------------- ����� ������� ���������� -------------------
        if(modules&1) {
          tmpbyte = chkcooler(eep.sp.state);
          if(tmpbyte<0) upv.pv.errors |= 0x40;        // ����� ������ �����
          else if(tmpbyte>0) upv.pv.warning |= 0x20;  // ������� ����������� �����������
        }
        if(modules&2) {
          tmpbyte = chkhorizon(eep.sp.state);
          if(tmpbyte<0) upv.pv.errors |= 0x40; // ����� ������ ��������
          else if(tmpbyte>0) upv.pv.warning |= 0x40;  // ��� �������� ������
        }
        if(modules&4) {
          tmpbyte = readCO2(&upv.pv);
          if(tmpbyte<0) upv.pv.errors |= 0x20;        // ����� ������ CO2
        }
        if(modules&8) {
          tmpbyte = chkflap(0,&upv.pv.flap);
          if(tmpbyte<0) upv.pv.errors |= 0x20;        // ����� ������ ��������
        }
    //-------------------- ������ �������� � ������ -------------------
        if (eep.sp.state&1){
      // ---------------------- ����������� --------------------------------
          if(HAL_GPIO_ReadPin(OVERHEAT_GPIO_Port, OVERHEAT_Pin)==GPIO_PIN_RESET) upv.pv.errors |= 0x10;  // �������� ��������� !!!
          else if((upv.pv.warning&0x20)==0){    // ��� �������� ����������� ����������� !!!
            if(upv.pv.pvT[0] < 850){
              int16_t err = eep.sp.spT[0] - upv.pv.pvT[0];
              if(heatCondition(err, eep.sp.alarm[0])) upv.pv.warning |= 0x01;  // ���������� �� �����������
              pwTriac0 = heater(err, &eep.sp);
              if(pwTriac0) {HEATER = 1; /*set_Pulse(250);*/}  // HEATER On  32 mks
            }
            else upv.pv.errors |= 0x01;   // ������ ������� ����������� !!!
          }
          upv.pv.power = pwTriac0 / 2;
          if(upv.pv.power > 100) upv.pv.power = 100;
          statPw[0] += upv.pv.power;      // ������ ����������
      // ---------------------- ����������� --------------------------------
          if(ok0&1){  // ���������� ���������� ��� ��������� � ��������������
            int16_t err;
            if(HIH5030||AM2301){  // ��������� ����������� ������ ���������
              err = eep.sp.spRH[1] - upv.pv.pvRH;
              pwTriac1 = humidifier(err, &eep.sp);
            }
            else {
              if(upv.pv.pvT[1] < 850){
              err = eep.sp.spT[1] - upv.pv.pvT[1];
              pwTriac1 = humidifier(err, &eep.sp);
              }
              else {upv.pv.errors |= 0x02; pwTriac1 = 0;}   // ������ ������� ��������� !!!
            }
            if(humCondition(err, eep.sp.alarm[1])) upv.pv.warning |= 0x02; // ���������� �� ���������
            if(pwTriac1 && (upv.pv.fuses&0x01)==0) HUMIDI = 1;  // HUMIDIFIER On
            tmpbyte = pwTriac1 / 2;
            if(tmpbyte > 100) tmpbyte = 100;
            statPw[1] += tmpbyte;         // ������ ����������
          }
      // ---------------------- ���������� ---------------------------------
          if(upv.pv.warning&0x20) tmpbyte = ON;     // ������� ����������� �����������
          else {
            if(upv.pv.pvT[0] < 850){
              int16_t err = upv.pv.pvT[0] - eep.sp.spT[0];
              if(err>=eep.sp.extOff[0]) tmpbyte = ON; else if(err<=0) tmpbyte = OFF; // ��� err >=0.2���. -> ON; ��� err <=0���. -> OFF;
            }
            else upv.pv.errors |= 0x01;             // ������ ������� �����������! (��� 51, 53)
          }
          if(upv.pv.fuses&0x02) tmpbyte = OFF;      // �������������� ���. ����� �1
          switch (tmpbyte){
            case ON:  FLAP = ON;  upv.pv.flap = eep.sp.zonaFlap&0x3F+37;  if(modules&8) chkflap(SETFLAP,  &upv.pv.flap); break;// ��������� �������� zonaFlap&0x3F=63+37=100%
            case OFF: FLAP = OFF; upv.pv.flap = FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &upv.pv.flap); break;// ��������� ��������; ����� ����� ���������� �������������� ������ ����
          }
      //----------------------- ����������� ����������� ������ -------------
          if(ok0){
              if(HIH5030||AM2301) {if(ds18b20_amount>1) {if(abs(upv.pv.pvT[0]-upv.pv.pvT[1])>eep.sp.zonaFlap) upv.pv.warning |=0x08;}} // ������� ������� ����������.
              else {if(ds18b20_amount>2) {if(abs(upv.pv.pvT[0]-upv.pv.pvT[2])>eep.sp.zonaFlap) upv.pv.warning |=0x08;}};// ������� ������� ����������.
          }
          if(!(HIH5030||AM2301) && (upv.pv.pvT[1]-upv.pv.pvT[0])>20){
            if(upv.pv.pvT[1] < 850){
              upv.pv.warning =0x10;               // ������������ ������������ ��������! (��� 38, 40)
              pwTriac0 = 100; upv.pv.power = 50;  // �������� ���������� 50%
            }
            else upv.pv.errors |= 0x02;           // ������ ������� ���������! (��� 52, 53)
          }
      //----------------------- ������� ������ ���������� ������ -----------
          if(eep.sp.koffCurr){
          // ��������� mV � mA ->100mV/1A+1.65V. (5A->0.5+1.65=2.15; 10A->2.65; 20A->3.65) ��������� �������� 110k/68k �� 2,65 � �������� 10� = 1� = 1000 ��
            pvCurrent = currAdc/10 * eep.sp.koffCurr; pvCurrent /= 100;
            if(upv.pv.power==100 && countsec>=0 && currAdc<100) upv.pv.errors|=0x08;  // ���� ��� < 1,0 �. -> ���������� ���� ����������� 
          }
          else currAdc = 0;
          // ������ ��������� ��������� �������� (����� � ���� ����� ����� ������� ��������)
          int16_t newErr = abs(eep.sp.spT[0]-upv.pv.pvT[0]);
          if((upv.pv.warning & 3)&&(newErr-alarmErr)>2) disableBeep=0;  // ���� ��� ��������� ������ ���������� ������������ ������ ����� ����������
          if(countsec>59){
            ++countmin; countsec=0; if (disableBeep) disableBeep--;
            
            if(cardOk) My_LinkDriver(".txt");  // ������ �� SD ���� ������ �������� � ������
            
            if(!(eep.sp.state&0x18)) rotate_trays(eep.sp.timer[0], eep.sp.timer[1], &upv.pv);   // ����������� ������ ���� ������ ���.
            if(upv.pv.pvCO2>0) CO2_check(eep.sp.spCO2, eep.sp.spCO2, upv.pv.pvCO2);             // �������� ������������ ��2
            else if(eep.sp.air[1]>0) aeration_check(eep.sp.air[0], eep.sp.air[1]);              // ������������� ����������� ������ ���� air[1]>0
            statPw[0]/=60; statPw[1]/=60;     // ������ ������ ��������
//              upv.pv.cost0=statF2(0, statPw[0]); upv.pv.cost1=statF2(1, statPw[1]); // ������ ������ ��������
            statPw[0]=0; statPw[1]=0;         // ������ ������ ��������
            if(countmin>59){
              countmin = 0; /*upv.pv.date = sDate.Date; upv.pv.hours = sTime.Hours;*/
//                eep.sp.EnergyMeter += (summPower/100);// ��������� � ��.
//              EEPSAVE=1; waitset=1;
            }
          }
      // ---------------------- ��������������� ----------------------------
          tmpbyte = extra_2(&eep.sp, &upv.pv);
          switch (tmpbyte){
              case ON:  EXTRA = ON;  break;
              case OFF: EXTRA = OFF; break;
          }
        }
    //--------------------- ������ ��������� ---------------------------
        else if((eep.sp.state&7)==0){
            if(eep.sp.relayMode==4) valRun = 0;                                         // ��������� ���������� ���������� ������������
            if(servis){                                                                 // ������� ��������� �����
              switch (servis){
                 case 1: pwTriac0=MAXPULS; portOut.value = 0x01; break;                 // �����������
                 case 2: pwTriac1=MAXPULS; portOut.value = 0x02; break;                 // �����������
                 case 3: portOut.value = 0x04; upv.pv.flap = eep.sp.zonaFlap&0x3F+37;   //zonaFlap&0x3F=63+37=100%
                         if(modules&8) chkflap(SETFLAP, &upv.pv.flap); break;           // �������������, ����������� 90���.
                 case 4: portOut.value = 0x08; break;                                   // �������������� �����
                 case 5: portOut.value = 0x10; break;                                   // ����� �����
                 default: portOut.value = 0; upv.pv.flap=FLAPCLOSE;
                          if(modules&8) chkflap(DATAREAD, &upv.pv.flap);// ��� ���������, ����������� 0���.
              }
            }
            else {
               upv.pv.power=OFF; portOut.value &= 0x10; upv.pv.flap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &upv.pv.flap); VENTIL = OFF;
               if(currAdc>1000){upv.pv.errors|=0x04;}   // ���� ���� ���� > 1000 mV ������ ���������!
               if(countsec>59){
                countsec=0;
//                if(cardOk) SD_write(fileName, p_eeprom, p_rampv); else My_LinkDriver();  // ????????????????????????????????????????????
                if(eep.sp.state&0x80) rotate_trays(eep.sp.timer[0], eep.sp.timer[1], &upv.pv); // ������� ������ ��� ����������� ������
               }
            }
        }
    //--------------------- ���� ����� ��������� ��������� -------------
        if(waitset){
          if(--waitset==0){
            if(EEPSAVE){
              eep_write(0x0000, eep.data, EEP_DATA);    // ������ � ����������������� ������
              if(file.data[0]){                         // ��������� ���� ��� ����� *.eep
                if(file.data[0]==2){
                  sprintf(fileName,"%02u_%02u_%02u",sDate.Year,sDate.Month,sDate.Date);
                  writeDateToBackup(RTC_BKP_DR1);       // � �������� �� backup ������� ����
                }
                if(cardOk) My_LinkDriver(".eep");       // ���� ��������� SD ������ � ���� *.eep
                file.data[0] = 0;                       // ������� ����
              }
            }
            if(servis==7){upv.pv.node = eep.sp.identif; bluetoothName(eep.sp.identif);}   // ��������� Broadcast name 
            servis=0;setup=0;displmode=0;psword=0;buf=0;topUser=TOPUSER;botUser=BOTUSER;} // ������������ � ��������� ������, ����� ������ 
        }
        if(TURN && eep.sp.timer[1]){if(--upv.pv.nextTurn==0) { upv.pv.nextTurn=eep.sp.timer[0]; TURN = OFF;}} // ������ ��� sp[1].timer>0 -> ������������ �����
        
          unixTime = timestamp(); //  ������� � unixTime
    // -------------------- Bluetooth ----------------------------------
        if(HAL_GPIO_ReadPin(Bluetooth_STATE_GPIO_Port, Bluetooth_STATE_Pin)){ // ���� ���� ����������� �� Bluetooth
          crc.val[0] = 0;
          for(uint8_t i=0;i<20;i++){
            crc.val[0] += upv.pvdata[i];
            crc.val[0] ^= (crc.val[0]>>2);
          }
          for(uint8_t i=0;i<40;i++){
            crc.val[0] += eep.data[i];
            crc.val[0] ^= (crc.val[0]>>2);
          }
          HAL_UART_Transmit(&huart1,(uint8_t*)upv.pvdata,20,0x1000);// �������� upv
          HAL_UART_Transmit(&huart1,(uint8_t*)eep.data,40,0x1000);  // �������� eep
          HAL_UART_Transmit(&huart1,(uint8_t*)crc.data,4,0x1000);   // �������� crc, "\r=<CR>, \n=<LF>"
        }
      }
  /*** ============================================= END ������ TIM4 1 ��. ==================================================================== ***/

  /*** --------------------------------------------------- ��������� -------------------------------------------------------------------------- ***/
      if(DISPLAY){
        DISPLAY = 0;
        if(setup) display_setup(&eep.sp);
        else if(servis) display_servis(&upv.pv);
        else display(&eep.sp, &upv.pv);
        ledOut(eep.sp.state, upv.pv.warning, upv.pv.fuses, eep.sp.programm); SendDataTM1638(); set_Output(); // ������ � ���������� TM1638 � 74HC595D
      }
  /*** ======================================================================================================================================== ***/
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      if(coolerAdc < 250) COOLER=0; else if(coolerAdc > 300) COOLER = 1;  // ����������� 40� = 2,50V; 55C = 3.0V
      if(pwTriac0 < 0) {pwTriac0=0; HEATER = 0; LEDOFF = 1; /*set_Pulse(500);*/}  // HEATER Off  64 mks
      if(pwTriac1 < 0) {pwTriac1=0; HUMIDI = 0; LEDOFF = 1;}  // HUMIDIFIER Off
      if(eep.sp.relayMode==4){                                 // ���������� ����� ������ ������
        if(pulsPeriod < 0){
          pulsPeriod = eep.sp.period;
          if(pwTriac1 && (upv.pv.fuses&0x01)==0) {pwTriac1=valRun; HUMIDI = 1; LEDOFF = 1;}  // HUMIDIFIER On
        }
      }
      // LEDOFF = 1 ��� �������� ���������� ����������� pwTriac0, pwTriac1
      if(LEDOFF) {LEDOFF = 0; ledOut(eep.sp.state, upv.pv.warning, upv.pv.fuses, eep.sp.programm); SendDataTM1638(); set_Output();}  // ������ � ���������� TM1638 � 74HC595D
      if(beepOn < 0) {beepOn=0; HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);}  // Beeper Off
  /* ------------------------------------------------------------------------------------------------------------------ */
      if(bluetoothData.ind == 0){
        bluetoothData.timeOut=0;
        HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2); // ������ ������ (100 ���� ��� 9600 b/s ����� �������� ������ 0,1 ���.)
      }
      else if(bluetoothData.timeOut > 5) {
        // ������ �������� ������ 500 mS.
        HAL_UART_AbortReceive_IT(&huart1); // ��������� ������ (https://www.translatorscafe.com/unit-converter/ru-RU/calculator/data-transfer-time/)
        bluetoothData.ind = 0; // ������� �������� ������� �����
        bluetoothData.timeOut = 0;
      }
  // ---------------------------- RS485 -----------------------------
      if(rs485Data.timeOut > 40){ // 200 mS
        rs485Data.ind = 0; // ������� �������� ������� �����  
        rs485Data.timeOut=0;
        HAL_UART_Receive_IT(&huart3,(uint8_t*)rs485Data.RXBuffer,16); // ������ ������ 16 byte (96 ���� ��� 9600 b/s ����� �������� ������ 0,101 ���.)
      }
  /* ================================================================================================================== */
      
      if(bluetoothData.ind == 0){
        bluetoothData.timeOut=0;
        HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2); // ������ ������
      }
      else if(bluetoothData.timeOut >= 10) {
        // ������ �������� ������ 10 mS.
        HAL_UART_AbortReceive_IT(&huart1); // ��������� ������
        bluetoothData.ind = 0; // ������� �������� ������� �����
        bluetoothData.timeOut = 0;
//        HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2); // ������ ������
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
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
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
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
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
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
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_3;
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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
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
  htim3.Init.Prescaler = 23999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 14;
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
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  HAL_GPIO_WritePin(GPIOB, OUT_RCK_Pin|DISPL_STB_Pin|SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DE485_Pin|Beeper_Pin, GPIO_PIN_RESET);

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

  /*Configure GPIO pin : SD_CS_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){  
  if(hadc->Instance == ADC1) flag = 1;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  if(huart==&huart1) bluetoothCallback();
  else if(huart==&huart3) rs485Callback(p_ramdata, p_eepdata);
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
