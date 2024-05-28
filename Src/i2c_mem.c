// ��� ��� ��� AT24C32N �����, ��� �������� ����� (A0, A1 � A2), ���������� �������� ������ �������. �� 0x50 �� 0x57.
/* ��������� ������ ���� AT24C08 */
/**
  ******************************************************************************
  * 0x0000 - 0x0027 - 40 byte
  * 0x0028 - 0x002F - 8  byte ������
  *  4 ���� ������� ����������� sp[0].spT->����� ������; sp[1].spT->������� ������
  *  1 ���� ������� ��������� ������ HIH-5030/AM2301
  *  1 ���� ��������� �������� 
  *  1 ���� �������
  *  1 ���� ���������� (����� 0b00001111) �����;
  * 0x0030 - 0x003F - 16 byte -> 1,2 ���� 1 ��������� / 240 byte �� 30 ����
  * 0x0120 - 0x012F - 16 byte -> 1,2 ���� 2 ��������� / 240 byte �� 30 ����
  * 0x0210 - 0x021F - 16 byte -> 1,2 ���� 3 ��������� / 240 byte �� 30 ����
  * 0x0300 - 0x030F - 16 byte -> 1,2 ���� 4 ��������� / 240 byte �� 30 ����
  * 0x03F0 - 0x03FF - 16 byte ������
  ******************************************************************************  
  */
#include "main.h"
#include "displ.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv

extern I2C_HandleTypeDef EEPROM_I2C_PORT;

struct {
  uint8_t eepAddr;
  uint8_t sizeAddr;
  uint8_t pageSize;
} eepMem;

HAL_StatusTypeDef ret_stat;

void dspl_error(uint8_t status)
{
  setChar(0,SIMBL_E); setChar(1,SIMBL_E); setChar(2,SIMBL_P);      // "EEP"
  setChar(3,SIMBL_MINUS); setChar(4,status); setChar(5,SIMBL_BL);  // "-x "
  SendDataTM1638();
  HAL_Delay(5000);
}

void eep_write(uint16_t memAddr, uint8_t *data, uint8_t amount){
	int16_t writebyte;
	uint8_t i;
	uint8_t *begData = data;
	uint16_t begMemAddr = memAddr;
  EEPSAVE = 0;
  eepMem.eepAddr = (0x50 << 1);  // HAL expects address to be shifted one bit to the left
  eepMem.sizeAddr = I2C_MEMADD_SIZE_8BIT;
  eepMem.pageSize = 16;          // AT24C04A ��� AT24C08A. The 4K/8K EEPROM is capable of 16-byte page writes
  /* ------------ display ------------------ */
//  sprintf(writing0, "Start");//"Starting the test - writing to the memory...\r\n"
//  sprintf(writing1, "writing");
//  mem_display(1000, (char*)writing0, (char*)writing1);
  /* --------------------------------------- */
  setChar(7, SIMBL_T_M_B); SendDataTM1638();
  
	if (amount-eepMem.pageSize > 0) writebyte = eepMem.pageSize;
	else writebyte = amount;
	while (amount > 0){
		ret_stat = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, eepMem.eepAddr, memAddr, eepMem.sizeAddr, (uint8_t*)data, writebyte, HAL_MAX_DELAY);
		if(ret_stat) dspl_error(ret_stat);
		/* ------------ display ------------------ */
//		sprintf(writing0, "Waiting");//"OK, now waiting until device is ready...\r\n"
//		sprintf(writing1, "is ready");
//		mem_display(1000, (char*)writing0, (char*)writing1);
		/* --------------------------------------- */
		for(i=0;i<100;i++) { // wait...
		  ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepMem.eepAddr, 1, HAL_MAX_DELAY);
		  if(ret_stat == HAL_OK) break;
		}
		/* ------------ display ------------------ */
		if (i>99) {
//			sprintf(writing0, "Device");//"Done, now comparing...\r\n"
//			sprintf(writing1, "notReady");
//			mem_display(2000, (char*)writing0, (char*)writing1);
      for(i=0;i<8;i++) {setChar(i,SIMBL_M_Top); PointOn(i);} SendDataTM1638();// �������������+�����
      HAL_Delay(5000);
			return;
		}
		/* --------------------------------------- */
		memAddr += writebyte;
		data += writebyte;
		amount -= writebyte;
		if (amount-eepMem.pageSize > 0) writebyte = eepMem.pageSize;
		else writebyte = amount;
	}
	/* ----------------- NOW COMPARING... ----------------------------- */
	uint8_t temp[EEP_DATA];
	eep_read(begMemAddr, temp);
	/* ------------ display ------------------ */
//	sprintf(writing0, "Now");
//	sprintf(writing1, "comparing");
//	mem_display(1000, (char*)writing0, (char*)writing1);
	/* --------------------------------------- */
  
//	SSD1306_GotoXY(0,40);
	if(memcmp(begData, temp, EEP_DATA) == 0){
	/* ------------ display ------------------ */
//		SSD1306_Puts("passed!", &Font_11x18, SSD1306_COLOR_WHITE);
//    SSD1306_UpdateScreen();
	}
	else {
//    SSD1306_Puts("failed!", &Font_11x18, SSD1306_COLOR_WHITE);
//    SSD1306_UpdateScreen();
    for(i=0;i<8;i++) {setChar(i,SIMBL_MBott); PointOn(i);} // �������������+�����
    SendDataTM1638();
    HAL_Delay(5000);
  }
	/* --------------------------------------- */
}

uint8_t eep_read(uint16_t memAddr, uint8_t *data){
  eepMem.eepAddr = (0x50 << 1);  // HAL expects address to be shifted one bit to the left
  eepMem.sizeAddr = I2C_MEMADD_SIZE_8BIT;
  eepMem.pageSize = 16;          // AT24C04A ��� AT24C08A. The 4K/8K EEPROM is capable of 16-byte page writes
	/* ------------ display ------------------ */
//	sprintf(writing0, "Start");//"Device is ready, now reading...\r\n"
//	sprintf(writing1, "reading");
//	mem_display(1000, (char*)writing0, (char*)writing1);
	/* --------------------------------------- */
	ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepMem.eepAddr, 1, HAL_MAX_DELAY);
	if(ret_stat) {dspl_error(ret_stat); return ret_stat;}
	memAddr = 0x0000;
	ret_stat = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, eepMem.eepAddr, memAddr, eepMem.sizeAddr, (uint8_t*)data, EEP_DATA, HAL_MAX_DELAY);
	if(ret_stat) dspl_error(ret_stat);
  return ret_stat;
}

void eep_initial(uint16_t memAddr, uint8_t *data){
	/* ------------ display ------------------ */
//	sprintf(writing0, "Initial");//"Device is ready, now reading...\r\n"
//	sprintf(writing1, "EEPROM");
//	mem_display(1000, (char*)writing0, (char*)writing1);
	/* --------------------------------------- */
  for(int8_t i=0;i<8;i++) setChar(i,SIMBL_T_M_B);
  SendDataTM1638();

	const uint8_t source[EEP_DATA]={
	/* ---------------------- uint16_t ---------------------------------*/
	/* ������� ����������� */
	0x5E,0x01,// data[0,1];   spT[0]->����� ������ = 350 -> 35 ��.C
	0x2C,0x01,// data[2,3];   spT[1]->������� ������ = 300 -> 30 ��.C
	/* ������� ��������� */
	        0,// data[4];     spRH[0]->���������� HIH-5030/AM2301
	        0,// data[5];     spRH[1]->������� ��������� ������ HIH-5030 -> 0%
    	    0,// data[6];     state ��������� ������ (����. ���. ����������, � �.�.)
	        0,// data[7];     extendMode ����������� ����� ������  0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
	        2,// data[8];     relayMode �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1] 4-> ��� �� ���.[1]
	        0,// data[9];     programm ������ �� ���������
          5,// data[10];    minRun = 5  -> 0.5���.
         10,// data[11];    maxRun = 10 -> 10 ���.
         60,// data[12];    period = 60 -> 60 ���. = 1 ���.
         60,// data[13];    timer[0]  ������.��������e = 60 ���.
          0,// data[14];    timer[1]  �����.��������e  = 0 ���. (����������� �����)
          5,// data[15];    alarm[0]  5 = 0.5 ��.C
          5,// data[16];    alarm[1]  5 = 0.5 ��.C
          6,// data[17];    extOn[0]  �������� ��� ���. ���������������� ������ 1 = 5 -> 0.5 ��.C
          5,// data[18];    extOn[1]  �������� ��� ���. ���������������� ������ 2 = 5 -> 0.5 ��.C
          3,// data[19];    extOff[0] �������� ��� ����. ���������������� ������ 1 = 2 -> 0.2 ��.C
          2,// data[20];    extOff[1] �������� ��� ����. ���������������� ������ 2 = 2 -> 0.2 ��.C
         60,// data[21];    air[0]    ������ ������������� �����; = 60 ���.
          0,// data[22];    air[1]    ������ ������������� air[1]-������; ���� air[1]=0-���������
         40,// data[23];    spCO2     ������� �������� ��� ���������� ������������ ��2 MAX; = 40 -> 4000
        100,// data[24];    koffCurr ��������� ����. �� ���� ��������� = 1.0
          1,// data[25];    hysteresis ���������� ������ ���������� = 1 -> 0.1 ��.C
        255,// data[26];    zonaFlap ����� ����������� � ������ (����� 0xC0) = 2 -> 2 ��.C; FLAPOPEN (����� 0x3F) -> zonaFlap&0x3F+37 -> 63+37=100%  
         80,// data[27];    turnTime ����� �������� ������� ������ � �������� = 80 sec.
         60,// data[28];    timeOut ����� �������� ������ ������ ���������� = 60(*10) -> 600 ���. = 10 ���.
         25,// data[29];    pkoff[0] ���������������� �����. = 25
         10,// data[30];    pkoff[1] ���������������� �����. = 10
         90,// data[31];    ikoff[0] ������������ �����. = 90 -> 900
         90,// data[32];    ikoff[1] ������������ �����. = 90 -> 900
          1,// data[33];    identif ������� ����� �������
        192,// data[34];    ip[0] IP MQTT broker [192.168.100.100]
        168,// data[35];    ip[1]
        100,// data[36];    ip[2]
        100,// data[37];    ip[3]
          0,// data[38]; �� ������������ !!!!!!!!!!!!!!!!!!!!!!!!!
          0,// data[39]; �� ������������ !!!!!!!!!!!!!!!!!!!!!!!!!
	};        // ����� 40 bytes
	memcpy(data, source, EEP_DATA);   // ����������� ������ ������ � ������
	ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepMem.eepAddr, 1, HAL_MAX_DELAY);
	if(ret_stat) {dspl_error(ret_stat); return;}

	eep_write(memAddr, data, EEP_DATA);
}

void prog_initial(uint16_t memAddr, uint8_t *data){
  const uint8_t source[8]={
    0x5E,0x01,// data[0,1];   spT[0]->����� ������ = 350 -> 35 ��.C
    0x2C,0x01,// data[2,3];   spT[1]->������� ������ = 300 -> 30 ��.C
    0,// data[4];             spRH->  ������� ��������� ������ HIH-5030 -> 0%
    0,// data[5];             Flap->  ��������� �������� 0% -> �������
   60,// data[6];             timer-> ������.��������e = 60 ���.
   10,// data[7];             air->   ���������� (����� 0b00001111) �����; = 10 ���., (����� 0b11110000) ������������ ���� air=0-���������
  };
  uint8_t buff[16], i, xx=0x0030, yy;
  memcpy(buff, source, 8);      // ����������� 8 ����
  memcpy(&buff[8], source, 8);  // ����������� ��������� 8 ����
  for(yy=0;yy<4;yy++){          // 4 ���������
    for(i=0;i<20;i++){          // 20 ����� ���������
      eep_write(xx, buff, 16);
      xx +=16;
    }
    for(i=0;i<16;i++) buff[i] = 0;
    for(i=0;i<10;i++){            // 10 ����� �������
      eep_write(xx, buff, 16);
      xx +=16;
    }
  }
}

uint8_t rtc_check(void){
	uint8_t i=0;
	/* ------------ display ------------------ */
//	sprintf(writing0, "Looking");
//	sprintf(writing1, "EEPROM");
//	mem_display(500, (char*)writing0, (char*)writing1);
	/* --------------------------------------- */
  
	ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, 0xD0, 1, HAL_MAX_DELAY); // [0xD0] � ����� ���������� DS3231 real-time clock (RTC)
//	SSD1306_GotoXY(0,40);
	if (ret_stat == HAL_OK){
//	SSD1306_Puts("24C32", &Font_11x18, SSD1306_COLOR_WHITE);
  
	eepMem.eepAddr = (0x57 << 1);  // HAL expects address to be shifted one bit to the left
	eepMem.sizeAddr = I2C_MEMADD_SIZE_16BIT;
	eepMem.pageSize = 32;          // AT24C32A ��� AT24C64A. The 32K/64K EEPROM is capable of 32-byte page writes
	i = 1;
	}
//	else SSD1306_Puts("24C04", &Font_11x18, SSD1306_COLOR_WHITE);
	return i;
}

