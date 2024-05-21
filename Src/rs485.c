#include "main.h"
#include "proc.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv

#define RX_BUFFER_SIZE 16
#define BREND     0x63  // 99
#define SOH       0xDD	// Начало блока
#define RESP		  0xEE	// Ответ блока
#define SPRG		  0xAA	// Начало блока программы
#define BROADCAST	0xBC	// Широковещательный вызов
#define COMMAND_LINK        81	// 0x51 Индикация
#define COMMAND_SP_WRITE	  51 	// 0x33 Запись опорных значений
#define COMMAND_NOM_WRITE	  53	// 0x35 Запись номера прибора и прочих установок
#define COMMAND_PRG_WRITE	  55	//      Запись в микросхему программы
#define COMMAND_PRG_READ	  85	//      Чтение из микросхемы программы
#define COMMAND_KOF_WRITE	  57	// 0x39 Запись коэфициентов
#define COMMAND_PUMP_WRITE	58	// 0x3A Запись парметров насоса
#define COMMAND_FLP_WRITE 	59	// 0x3B Запись параметров заслонки
#define COMMAND_START 	    87	//      Запись номера программы и старт новой инкубации
#define COMMAND_TURN 	      89	//      Начать поворот
#define COMMAND_MODULE 	    91	//      Команда модулю

extern uint8_t countsec;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern char buffTFT[];
extern UART_HandleTypeDef huart3;
extern uint8_t ds18b20_amount;

struct rs485 rs485Data;

/***************************************************************************************************
Output = Chr(DevNom)+Chr(COMMAND_*)+Chr(INDEX0)+Chr(INDEX1)+Chr(lnRxD1)+Chr(lnRxD2)+Chr(lnRxD3)+Chr(lnRxD4);
		+Chr(lnRxD5)+Chr(lnRxD6)+Chr(lnRxD7)+Chr(lnRxD8)+Chr(lnRxD9)+Chr(lnRxD10)+Chr(lnRxD11)+Chr(lnCRC)
****************************************************************************************************/
void rs485Callback(uint8_t *p_ramdata, uint8_t *p_eepdata){
  uint8_t i, item, identif, *p_ind;
  uint16_t word = 0;
  union d2v intVal;
//  set_Pulse(250);  //  32 mks

  for (i=0; i<RX_BUFFER_SIZE-1; i++) word+=rs485Data.RXBuffer[i];
  word %=255;
  if(word != rs485Data.RXBuffer[RX_BUFFER_SIZE-1]){HAL_UART_AbortReceive_IT(&huart3); rs485Data.timeOut=0; return;}   // если неверная CRC то выход.
  p_ind = p_eepdata+33; // указатель на eep->identif сетевой номер прибора
  identif = *p_ind;

  if(rs485Data.RXBuffer[0] == BROADCAST || rs485Data.RXBuffer[0] == identif){// Начало блока

    rs485Data.ind = 0;
    switch (rs485Data.RXBuffer[1]){
        case COMMAND_LINK:         // Есть связь с компьютером
          {
           rs485Data.TXBuffer[0] = SOH;    // Начало блока  item: 0
           rs485Data.TXBuffer[1] = BREND;  // Марка прибора item: 1
           rs485Data.TXBuffer[2] = identif;// сетевой номер прибора
           item = 3; p_ind = p_eepdata+6;   // state
           // state, extendMode, relayMode, programm
           for (i=0; i<4; i++){rs485Data.TXBuffer[item] = *p_ind++; item++;}  // item: 3-6  (4 byte)
           p_ind = p_eepdata;      // spT[0]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = *p_ind++; item++;}    // item: 7-8   (2 byte)
           p_ind = p_eepdata+4;    // spRH[0]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = *p_ind++; item++;}    // item: 9-10  (2 byte)
           p_ind = p_eepdata+13;   // timer[0]
           rs485Data.TXBuffer[item] = *p_ind++; item++;    // item: 11  (1 byte)
           p_ind = p_eepdata+15;   // alarm[0]
           rs485Data.TXBuffer[item] = *p_ind++; item++;    // item: 12  (1 byte)
           p_ind = p_eepdata+17;   // extOn[0]
           rs485Data.TXBuffer[item] = *p_ind++; item++;    // item: 13  (1 byte)
           p_ind = p_eepdata+19;   // extOff[0]
           rs485Data.TXBuffer[item] = *p_ind++; item++;    // item: 14  (1 byte)
           p_ind = p_eepdata+2;    // spT[1]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = *p_ind++; item++;}    // item: 15-16 (2 byte)
           p_ind = p_eepdata+5;    // spRH[1]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = *p_ind++; item++;}    // item: 17-18 (2 byte)
           p_ind = p_eepdata+14;   // timer[1]
           rs485Data.TXBuffer[item] = *p_ind++; item++;    // item: 19  (1 byte)
           p_ind = p_eepdata+16;   // alarm[1]
           rs485Data.TXBuffer[item] = *p_ind++; item++;    // item: 20  (1 byte)
           p_ind = p_eepdata+18;   // extOn[1]
           rs485Data.TXBuffer[item] = *p_ind++; item++;    // item: 21  (1 byte)
           p_ind = p_eepdata+20;   // extOff[1]
           rs485Data.TXBuffer[item] = *p_ind++; item++;    // item: 22  (1 byte)
           intVal.val = *(p_eepdata+29);   // pkoff[0]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 23-24  (2 byte)
           intVal.val = *(p_eepdata+31)*10;   // ikoff[0]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 25-26  (2 byte)
           intVal.val = *(p_eepdata+30);   // pkoff[1]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 27-28  (2 byte)
           intVal.val = *(p_eepdata+32)*10;   // ikoff[1]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 29-30  (2 byte)
           for (i=0; i<4; i++){rs485Data.TXBuffer[item] = 0; item++;}    // item: 31-34  (4 byte)
           intVal.val = *(p_eepdata+10);   // minRun
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 35-36  (2 byte)
           intVal.val = *(p_eepdata+11);   // maxRun
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 37-38  (2 byte)
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = 0; item++;}    // item: 39-40  (2 byte)
           intVal.val = *(p_eepdata+12);   // period
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 41-42  (2 byte)
           for (i=0; i<7; i++){rs485Data.TXBuffer[item] = 0; item++;}    // item: 43-49  (7 byte-> notUsed2,spCO2[3],air[2])
           //----------------
           rs485Data.TXBuffer[item] = ds18b20_amount; item++;  //  item: 50 /* ds18b20_amount */  (1 byte)
           rs485Data.TXBuffer[item] = sDate.Date; item++;      // item: 51 /* days */  (1 byte)
           rs485Data.TXBuffer[item] = sTime.Hours; item++;     // item: 52 /* hours */  (1 byte)
           p_ind = p_ramdata+13;  // power
           rs485Data.TXBuffer[item] = *p_ind; item++;    // item: 53  (1 byte)
           p_ind = p_ramdata+10;  // nextTurn
           rs485Data.TXBuffer[item] = *p_ind; item++;    // item: 54  (1 byte)
           p_ind = p_ramdata+12;  // flap
           rs485Data.TXBuffer[item] = *p_ind; item++;    // item: 55  (1 byte)
           p_ind = p_ramdata+15;  // errors, warning
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = *p_ind++; item++;}    // item: 56-57  (2 byte)
           p_ind = p_ramdata+14;  // fuses
           rs485Data.TXBuffer[item] = ~*p_ind; item++;    // item: 58  (1 byte)
           p_ind = p_ramdata+17;  // nothing0, nothing1
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = *p_ind++; item++;}    // item: 59-60 (2 byte -> cost0, cost1)
           for (i=0; i<5; i++){rs485Data.TXBuffer[item] = 255; item++;}         // item: 61-65 (5 byte -> ext[5])
           for (i=0; i<3; i++){rs485Data.TXBuffer[item] = 0; item++;}           // item: 66-68 (3 byte -> flap[3])
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = 0; item++;}           // item: 69-70 ( kWattHore )
           intVal.val = *(p_ramdata+8);   // pvRH
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 71-72
           p_ind = p_ramdata+2;           // pvT[3]
           for (i=0; i<6; i++){rs485Data.TXBuffer[item] = *p_ind++; item++;}          // item: 73-78
           intVal.val = *(p_ramdata+9);   // pvCO2[3]
           for (i=0; i<2; i++){rs485Data.TXBuffer[item] = intVal.data[i]; item++;}    // item: 79-80
           for (i=0; i<4; i++){rs485Data.TXBuffer[item] = 0; item++;}                 // item: 81-84 нулевые байты ( 4 байт)
           rs485Data.TXBuffer[item] = portOut.value; item++;             // item: 85   перадача состояния иполнительного оборудования
           for (i=0; i<8; i++){rs485Data.TXBuffer[item] = 0; item++;}    // item: 86-93 нулевые байты ( 8 байт)
           
           word=0;
           for (i=2; i<94; i++){word += rs485Data.TXBuffer[i];}
           rs485Data.TXBuffer[94] = word%256;
           rs485Data.TXBuffer[95] = word/256;
           HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_SET);     // управление RS 485
           HAL_UART_Transmit(&huart3,(uint8_t*)rs485Data.TXBuffer,96,0x1000);
           HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_RESET);   // управление RS 485
//           if(ext[2]==255){   // если модуль СО2 НЕПОДКЛЮЧЕН
//               cr.data[0] = rs485Data.RXBuffer[3]; cr.data[1] = rs485Data.RXBuffer[4]; CO2[0] = cr.word; // ПЕРЕДАЧА ЗАМЕРОВ CO2
//               if(ext[3]==255){p_ind = p_ramdata+20; *p_ind = rs485Data.RXBuffer[5];}                    // ПЕРЕДАЧА положения воздушной заслонки
//           }
          }; break;
        case COMMAND_NOM_WRITE:
          {
            rs485Data.TXBuffer[0] = RESP;	  // Ответ блока  (1 байт)
            rs485Data.TXBuffer[1] = identif;	  // Номер блока  (1 байт)
            rs485Data.TXBuffer[2] = COMMAND_NOM_WRITE;
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_SET);     // управление RS 485
            HAL_UART_Transmit(&huart3,(uint8_t*)rs485Data.TXBuffer,3,0x1000);
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_RESET);   // управление RS 485

            p_ind = p_eepdata+33; // указатель на eep->identif сетевой номер прибора
            *p_ind = rs485Data.RXBuffer[4];  // identif; (1 byte)
            p_ind = p_eepdata+6; // указатель на state состояние камеры (ОТКЛ. ВКЛ. ОХЛАЖДЕНИЕ, и т.д.)
            for (i=5; i<9; i++) *p_ind++ = rs485Data.RXBuffer[i];  // state; extendMode; relayMode; programm.  (4 byte)
            
            eep_write(0x0000, p_eepdata);
          }; break;
        case COMMAND_SP_WRITE:
          {
            rs485Data.TXBuffer[0] = RESP;	  // Ответ блока  (1 байт)
            rs485Data.TXBuffer[1] = identif;	  // Номер блока  (1 байт)
            rs485Data.TXBuffer[2] = COMMAND_SP_WRITE;
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_SET);     // управление RS 485
            HAL_UART_Transmit(&huart3,(uint8_t*)rs485Data.TXBuffer,3,0x1000);
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_RESET);   // управление RS 485

            item = rs485Data.RXBuffer[2];
            if(item){
              item = 4;                                                   //  (8 byte)
              p_ind = p_eepdata+2; // указатель на eep->spT[1]
              for (i=0; i<2; i++) *p_ind++ = rs485Data.RXBuffer[item++];  //  item: 4-5
              p_ind = p_eepdata+5; // указатель на eep->spRH[1]
              *p_ind = rs485Data.RXBuffer[item];  //  item: 6 -> spRH[1]  item: 7 -> не используется
              item = 8;
              p_ind = p_eepdata+14; // указатель на eep->timer[1]
              for (i=0; i<4; i++){
                *p_ind = rs485Data.RXBuffer[item++];  //  item: 8-11
                p_ind += 2;
              }
            }
            else {
              item = 4;                                                   //  (8 byte)
              p_ind = p_eepdata; // указатель на eep->spT[0]
              for (i=0; i<2; i++) *p_ind++ = rs485Data.RXBuffer[item++];  //  item: 4-5
              p_ind = p_eepdata+4; // указатель на eep->spRH[0]
              for (i=0; i<2; i++) *p_ind++ = rs485Data.RXBuffer[item++];  //  item: 6-7
              p_ind = p_eepdata+13; // указатель на eep->timer[0]
              for (i=0; i<4; i++){
                *p_ind = rs485Data.RXBuffer[item++];  //  item: 8-11
                p_ind += 2;
              }
            }
            
            eep_write(0x0000, p_eepdata);
          }; break;
        case COMMAND_KOF_WRITE:
          {
            rs485Data.TXBuffer[0] = RESP;	  // Ответ блока  (1 байт)
            rs485Data.TXBuffer[1] = identif;	  // Номер блока  (1 байт)
            rs485Data.TXBuffer[2] = COMMAND_KOF_WRITE;
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_SET);     // управление RS 485
            HAL_UART_Transmit(&huart3,(uint8_t*)rs485Data.TXBuffer,3,0x1000);
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_RESET);   // управление RS 485

            item = 3;                                                   //  (8 byte)
            p_ind = p_eepdata+29; // указатель на eep->pkoff[0]=25
            *p_ind++ = rs485Data.RXBuffer[item];  //  pkoff[0]
            // ---------- Ti[0]=900 ---------------
            item = 5;
            intVal.data[0] = rs485Data.RXBuffer[item++];
            intVal.data[1] = rs485Data.RXBuffer[item];
            p_ind = p_eepdata+31;   // указатель на eep->ikoff[0]=90
            *p_ind = intVal.val/10; //  ikoff[0]
            item = 7;
            p_ind = p_eepdata+30; // указатель на eep->pkoff[1]=20
            *p_ind++ = rs485Data.RXBuffer[item];  //  pkoff[1]
            // ---------- Ti[1]=900 ---------------
            item = 9;
            intVal.data[0] = rs485Data.RXBuffer[item++];
            intVal.data[1] = rs485Data.RXBuffer[item];
            p_ind = p_eepdata+32; // указатель на eep->ikoff[1]=900
            *p_ind =  intVal.val/10; //  ikoff[1]
            
            eep_write(0x0000, p_eepdata);
          }; break;
        case COMMAND_PUMP_WRITE:
          {
            rs485Data.TXBuffer[0] = RESP;	  // Ответ блока  (1 байт)
            rs485Data.TXBuffer[1] = identif;	  // Номер блока  (1 байт)
            rs485Data.TXBuffer[2] = COMMAND_PUMP_WRITE;
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_SET);     // управление RS 485
            HAL_UART_Transmit(&huart3,(uint8_t*)rs485Data.TXBuffer,3,0x1000);
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_RESET);   // управление RS 485

            // ----------- minRun=100 -----------------
            item = 4;
            intVal.data[0] = rs485Data.RXBuffer[item++];
            intVal.data[1] = rs485Data.RXBuffer[item];
            p_ind = p_eepdata+10; // указатель на eep->minRun
            *p_ind =  intVal.val/20; // minRun=5
            // ----------- maxRun=600 -----------------
            item = 6;
            intVal.data[0] = rs485Data.RXBuffer[item++];
            intVal.data[1] = rs485Data.RXBuffer[item];
            p_ind = p_eepdata+11; // указатель на eep->maxRun
            *p_ind =  intVal.val/20; // maxRun=30
            // ----------- period=3000 -----------------
            item = 10;
            intVal.data[0] = rs485Data.RXBuffer[item++];
            intVal.data[1] = rs485Data.RXBuffer[item];
            p_ind = p_eepdata+12; // указатель на eep->period
            *p_ind =  intVal.val/200; // period=15
            
            eep_write(0x0000, p_eepdata);
          }; break;
        case COMMAND_FLP_WRITE:
          {
            rs485Data.TXBuffer[0] = RESP;	  // Ответ блока  (1 байт)
            rs485Data.TXBuffer[1] = identif;	  // Номер блока  (1 байт)
            rs485Data.TXBuffer[2] = COMMAND_FLP_WRITE;
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_SET);     // управление RS 485
            HAL_UART_Transmit(&huart3,(uint8_t*)rs485Data.TXBuffer,3,0x1000);
            HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_RESET);   // управление RS 485
            
//            eep_write(0x0000, p_eepdata);
//            if(modules&8)       // если подключен блок управления воздушными заслонками
//             {
//               pvFlap = rx_buffer[4];
//               chkflap(rx_buffer[5]);
//             }
          }; break;          
       }
  }
  else {HAL_UART_AbortReceive_IT(&huart3); rs485Data.timeOut=0;} // прерывания асинхронной операции приема данных и прекращения приема данных по интерфейсу UART.
//  set_Pulse(500);  //  64 mks
  return;
}
