#include "main.h"
#include <stdlib.h>
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "keypad.h"
#include "proc.h"

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern int8_t countsec, getButton, displmode;
extern uint8_t ok0, ok1, keyBuffer[], setup, waitset, waitkey, modules, servis;
extern int16_t alarmErr;
uint8_t disableBeep, topOwner, topUser, botUser, psword, keynum;
int16_t buf;
/*
void pushkey(void){
 uint8_t xx, keykod;
  getButton = 0;
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  ReadKeyTM1638();
  keykod=0;
  for (uint8_t i=0;i<4;i++){
  xx =  keyBuffer[i]; 
  xx <<= i; 
  keykod |= xx;
  if (keyBuffer[i]==0x01) LedInverse(i,1);
  else if (keyBuffer[i]==0x10) LedInverse(i+4,1);
  }
  SendDataTM1638();  
}
*/
//void checkkey(struct eeprom *t, int16_t pvT0){
void checkkey(struct eeprom *t, struct rampv *ram){
  uint8_t xx, keykod;
  ReadKeyTM1638();  
  keykod=0;
  for (uint8_t i=0;i<4;i++){xx =  keyBuffer[i]; xx <<= i; keykod |= xx;}   // ���������� ������� ������
  if ((keynum == keykod)&&keynum) {xx = 1; getButton=0;}  // ��� ���������� �������� ��������� ������� ������ (getButton=0) �������������� �������� �� ������������ ������
  else {keynum = keykod; xx= 0; waitkey=WAITCOUNT; getButton=waitkey/4;}
  if (xx){
    DISPLAY=1;
    if (setup){                // ����� �������������� ������� � ����������
        waitset=10;            // ���������� ����� ���������
        switch (keykod)
          {
           case KEY_1:
             {
              if (++setup>topOwner) setup=1;           // ���� ������������
              switch (setup)
                {
                 case 1:  buf=t->spT[0]; break;         // �1 ������� �����������
                 case 2:  if(HIH5030||AM2301) buf=t->spRH[1]; else buf=t->spT[1]; break;// �2 ������� ���������
                 case 3:  buf=t->timer[0]; break;       // �3 ����� ������������ ���������
                 case 4:  buf=t->timer[1]; break;       // �4 ����� ����������� ��������� (���� �� 0 �� ��� �������)
                 case 5:  buf=t->alarm[0]; break;       // �5 ������� �� ������ 1
                 case 6:  buf=t->alarm[1]; break;       // �6 ������� �� ������ 2
                 case 7:  buf=t->extOn[0]; break;       // �7 �������� ��� ���. ���������������� ������ 1
                 case 8:  buf=t->extOff[0]; break;      // �8 �������� ��� ����. ���������������� ������ 1
                 case 9:  buf=t->extOn[1]; break;       // �9 �������� ��� ���. ���������������� ������ 2
                 case 10: buf=t->extOff[1]; break;      // �10 �������� ��� ����. ���������������� ������ 2
                 case 11: if(modules&4) buf=t->spCO2; else buf=t->air[0]; break;// �11 MAX CO2 / ����� ����� ��������������
                 case 12: if(modules&4) buf=t->spCO2; else buf=t->air[1]; break;// �12 MIN CO2 / ������������ ������������
                } 
             } break;
           case KEY_2:
             {
              buf++; EEPSAVE=1; if (waitkey) waitkey--;
              switch (setup)
               {
                case 1:  t->spT[0]=buf; break;                                                                   // �1  ������� �����������
                case 2:  if(HIH5030||AM2301) t->spRH[1]=buf&0x7F; else t->spT[1]=buf; break;                     // �2  ������� ���������
                case 3:  if(buf<1) buf=1; t->timer[0]=buf; break;                                                // �3  ����� ������������ ���������
                case 4:  t->timer[1]=buf; break;                                                                 // �4  ����� ����������� ��������� (�������)
                case 5:  buf&=0x1F; if(buf<1) buf=1; t->alarm[0]=buf; break;                                     // �5  ������� �� ������ 1
                case 6:  buf&=0xFF; if(buf<1) buf=1; t->alarm[1]=buf; break;                                     // �6  ������� �� ������ 2
                case 7:  buf&=0x1F; if(buf<t->extOff[0]) buf=t->extOff[0]; t->extOn[0]=buf; break;               // �7  �������� ��� ���. ���������������� ������ 1
                case 8:  if(buf>t->extOn[0]) buf=t->extOn[0]; else if (buf<1) buf=1; t->extOff[0]=buf; break;    // �8  �������� ��� ����. ���������������� ������ 1
                case 9:  buf&=0x1F; if(buf<t->extOff[1]) buf=t->extOff[1]; t->extOn[1]=buf; break;               // �9  �������� ��� ���. ���������������� ������ 2
                case 10: if(buf>t->extOn[1]) buf=t->extOn[1]; else if (buf<1) buf=1; t->extOff[1]=buf; break;    // �10 �������� ��� ����. ���������������� ������ 2
                case 11: 
                         if(buf<1) buf=1; buf&=0xFF;
                         if(modules&4) t->spCO2=buf; else t->air[0]=buf;
                  break;                                                                                         // �11 MAX CO2 / ����� ����� ��������������
                case 12:
                         if(buf<0) buf=0; buf&=0xFF;
                         if(modules&4) t->spCO2=buf; else t->air[1]=buf;
                  break;                                                                                         // �12 MIN CO2 / ������������ ������������
                //------------------------------------ �00 -------------------------------------------------------------------
                case 16: switch (buf){
                          case  3: topOwner=14; break;// ���������� ������� �������� ������� ���������
                          case 10: topUser=TOPKOFF; botUser=BOTKOFF; break; // ���������� ������� �������� ������������ cof[3];
                         };
                         EEPSAVE=0; waitset=20;
                  break;
                //--------------------------- ���� ����������� ---------------------------------------------------------
                case 17: if(buf) t->extendMode=1; else t->extendMode=0; break;           // ����������� ����� ������  0-������; 1-��������� ����������
                case 18: if(buf>MAXRELAYMODE) buf=MAXRELAYMODE; else if(buf<MINRELAYMODE) buf=MINRELAYMODE; t->relayMode=buf;
                         if(t->relayMode==4) topUser=PULSMENU; else topUser=TOPUSER; break;//�������� ����� ������
                case 19: if(buf<1) buf=1; t->minRun=buf; break;   // ���������� 0.1-25.5 sec.;
                case 20: if(buf<1) buf=1; t->maxRun=buf; break;   // ���������� 1-255 ������ (4 ���.);
                case 21: if(buf<1) buf=1; t->period=buf; break;   // ���������� 1-255 ������ (4 ���.);
                
                case 26: if(buf>32) buf=32; else if (buf<-64) buf=-64; t->spRH[0]=buf; break; // ���������� ������� HIH-5030/AM2301
                case 27: xx = t->hysteresis>>6; t->hysteresis = buf&0x03; t->hysteresis |= xx<<6; break;// ����������
                case 28: buf&=0x3F; if(buf<1) buf=1; t->pkoff[0]=buf; break;         // ���������� 1 - 63;
                case 29: buf&=0x7F; if(buf<10) buf=10; t->ikoff[0]=buf; break;       // ���������� 10 - 127;
                case 30: buf&=0x3F; if(buf<1) buf=1; t->pkoff[1]=buf; break;         // ���������� 1 - 63;
                case 31: buf&=0x7F; if(buf<10) buf=10; t->ikoff[1]=buf; break;       // ���������� 10 - 127;
               }; 
             } break;
           case KEY_3:
             {
              ++setup;
              if (setup>topUser||setup<botUser) setup=botUser;// ���� �����������
              switch (setup)
                {
                 case 16: buf=0; break;               // �0 ����� ����������
                 case 17: buf=t->extendMode; break;   // �1 = ����������� ����� ������
                 case 18: buf=t->relayMode; break;    // �2 = ����� ���/����
                
                 case 19: buf=t->minRun; break;       // �3 = 5  -> 0.5���.
                 case 20: buf=t->maxRun; break;       // �4 = 10 -> 10 ���.
                 case 21: buf=t->period; break;       // �5 = 60 -> 60 ���. = 1 ���.
                 
                 case 26: buf=t->spRH[0]; break;      // �10 ���������� ������� HIH-4000
                 case 27: buf=t->hysteresis&3; break; // �11 ���������� ������ ���������� 
                 case 28: buf=t->pkoff[0]; break;     // �12 = 25
                 case 29: buf=t->ikoff[0]; break;     // �13 = 90
                 case 30: buf=t->pkoff[1]; break;     // �14 = 10
                 case 31: buf=t->ikoff[1]; break;     // �15 = 90
                }
             } break;
           case KEY_4:
             {
              buf--; EEPSAVE=1; if (waitkey) waitkey--;
              switch (setup)
               {
                case 1:  t->spT[0]=buf; break;                                                                   // �1  ������� �����������
                case 2:  if (HIH5030||AM2301) t->spRH[1]=buf; else t->spT[1]=buf; break;                         // �2  ������� ���������
                case 3:  if (buf<1) buf=1; t->timer[0]=buf; break;                                               // �3  ����� ������������ ���������
                case 4:  t->timer[1]=buf; break;                                                                 // �4  ����� ����������� ��������� (�������)
                case 5:  buf&=0x1F; if (buf<1) buf=1; t->alarm[0]=buf; break;                                    // �5  ������� �� ������ 1
                case 6:  buf&=0xFF; if (buf<1) buf=1; t->alarm[1]=buf; break;                                    // �6  ������� �� ������ 2
                case 7:  buf&=0x1F; if (buf<t->extOff[0]) buf=t->extOff[0]; t->extOn[0]=buf; break;              // �7  �������� ��� ���. ���������������� ������ 1
                case 8:  if(buf>t->extOn[0]) buf=t->extOn[0]; else if (buf<1) buf=1; t->extOff[0]=buf; break;    // �8  �������� ��� ����. ���������������� ������ 1
                case 9:  buf&=0x1F; if (buf<t->extOff[1]) buf=t->extOff[1]; t->extOn[1]=buf; break;              // �9  �������� ��� ���. ���������������� ������ 2
                case 10: if(buf>t->extOn[1]) buf=t->extOn[1]; else if (buf<1) buf=1; t->extOff[1]=buf; break;    // �10 �������� ��� ����. ���������������� ������ 2
                case 11: 
                         if(buf<1) buf=1; buf&=0xFF;
                         if(modules&4) t->spCO2=buf; else t->air[0]=buf;
                  break;                                                                                         // �11 MAX CO2 / ����� ����� ��������������
                case 12:
                         if(buf<0) buf=0; buf&=0xFF;
                         if(modules&4) t->spCO2=buf; else t->air[1]=buf;
                  break;                                                                                         // �12 MIN CO2 / ������������ ������������
                //------------------------------------ �00 -------------------------------------------------------------------
                case 16: switch (buf){
                          case  3: topOwner=14;            break;// ���������� ������� �������� ������� ���������
                          case 10: topUser=TOPKOFF; botUser=BOTKOFF; break; // ���������� ������� �������� ������������ cof[3];
                         };
                         EEPSAVE=0; waitset=20;
                  break;
                //--------------------------- ���� ����������� ---------------------------------------------------------
                case 17: if(buf) t->extendMode=1; else t->extendMode=0; break;             // ����������� ����� ������  0-������; 1-��������� ����������
                case 18: if(buf>MAXRELAYMODE) buf=MAXRELAYMODE; else if(buf<MINRELAYMODE) buf=MINRELAYMODE; t->relayMode=buf;
                         if(t->relayMode==4) topUser=PULSMENU; else topUser=TOPUSER; break;//�������� ����� ������
                case 19: if(buf<1) buf=1; t->minRun=buf; break;   // ���������� 0.1-25.5 sec.;
                case 20: if(buf<1) buf=1; t->maxRun=buf; break;   // ���������� 1-255 ������ (4 ���.);
                case 21: if(buf<1) buf=1; t->period=buf; break;   // ���������� 1-255 ������ (4 ���.);
                
                case 26: if(buf>32) buf=32; else if (buf<-64) buf=-64; t->spRH[0]=buf; break; // ���������� ������� HIH-5030
                case 27: xx = t->hysteresis>>6; t->hysteresis = buf&0x03; t->hysteresis |= xx<<6; break; // ���������� ���������� 3
                case 28: buf&=0x3F; if(buf<1)  buf=1;  t->pkoff[0]=buf; break;       // ���������� 1 - 63;
                case 29: buf&=0x7F; if(buf<10) buf=10; t->ikoff[0]=buf; break;       // ���������� 10 - 127;
                case 30: buf&=0x3F; if(buf<1)  buf=1;  t->pkoff[1]=buf; break;       // ���������� 1 - 63;
                case 31: buf&=0x7F; if(buf<10) buf=10; t->ikoff[1]=buf; break;       // ���������� 10 - 127;
               }; 
             } break;
           case KEY_6: setup=0;EEPSAVE = 0;displmode=0;psword=0;buf=0;beeper_ON(DURATION*10); break;
          }; 
       }
    else if(servis)           //***--- ��������� �����  ---***
       {
        waitset=15;           // ���������� ��������� �����
        switch (keykod)
          {
           case KEY_2:
            {
              buf++; EEPSAVE=1; waitkey=WAITCOUNT;
              switch (servis)
               {
                 case 7:  t->identif = buf&0x3F; break;         // C7 -> identif (1-63)
                 case 8:  t->zonality= buf&0x3F; break;         // C8-> ����� ����������� � ������
                 case 9:  t->turnTime= buf&0x3FF; break;        // C9 -> TURNTIME ����� �������� ������� ������ � ���.
                 case 10: t->waitCooling=(buf&0x3F)*6; break;   // C10-> TIME OUT ����� �������� ������ ������ ���������� � ���. 10 ���. *6 = 60(*10) -> 600 ���.
                 case 11: xx = t->hysteresis&3; t->hysteresis = (buf&0x03)<<6; t->hysteresis |= xx; break; // C11-> ��������� ������������ HIH-5030/AM2301
                 case 12: t->koffCurr= buf&0xFF; break;         // C12-> koffCurr ��������� ����. �� ���� ���������
                 case 13: HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                          sTime.Minutes = buf;
                          HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                  break;                                        // C13-> sTime.Hours
                 case 14: HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                          sTime.Hours = buf;
                          HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                  break;                                        // C14-> sTime.Minutes
                 case 15: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          sDate.Date = buf;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                  break;                                        // C15-> sDate.Date
                 case 16: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          sDate.Month = buf;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                  break;                                        // C15-> sDate.Month
                 case 17: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          sDate.Year = buf;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                  break;                                        // C15-> sDate.Year

               }
            } break;
           case KEY_3:
            {
              if(++servis>17) servis=7; displmode=0; waitkey=WAITCOUNT; beeper_ON(DURATION);
              switch (servis)
                {
                 case 7: buf=t->identif; break;                 // C7 -> identif
                 case 8: buf=t->zonality; break;                // C8-> ����� ����������� � ������
                 case 9: buf=t->turnTime; break;                // C9 -> TURNTIME ����� �������� ������� ������ � ���.
                 case 10: buf=t->waitCooling/6; break;          // C10-> TIME OUT ����� �������� ������ ������ ���������� � ���. 60(*10)/6 = 10 ���.
                 case 11: buf=t->hysteresis>>6; break;          // C11-> ��������� ������������ HIH-5030/AM2301
                 case 12: buf=t->koffCurr; break;               // C12-> koffCurr ��������� ����. �� ���� ���������
                 case 13: HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                          buf = sTime.Minutes;
                          HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                    break;                                      // C13-> sTime.Hours
                 case 14: HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                          buf = sTime.Hours;
                          HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                    break;                                      // C14-> sTime.Minutes
                 case 15: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          buf = sDate.Date;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                    break;                                      // C15-> sDate.Date
                 case 16: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          buf = sDate.Month;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                    break;                                      // C15-> sDate.Month
                 case 17: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          buf = sDate.Year;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                    break;                                      // C15-> sDate.Year
                }
            } break;
           case KEY_4:
            {
              buf--; EEPSAVE=1; waitkey=WAITCOUNT;
              switch (servis)
               {
                 case 7:  t->identif = buf&0x3F; break;         // C7 -> identif
                 case 8:  t->zonality= buf&0x3F; break;         // C8-> ����� ����������� � ������
                 case 9:  t->turnTime= buf&0x3FF; break;        // C9 -> TURNTIME ����� �������� ������� ������ � ���.
                 case 10: t->waitCooling=(buf&0x3F)*6; break;   // C10-> TIME OUT ����� �������� ������ ������ ���������� � ���. 10 ���. *6 = 60(*10) ���.
                 case 11: xx = t->hysteresis&3; t->hysteresis = (buf&0x03)<<6; t->hysteresis |= xx; break; // C11-> ��������� ������������ HIH-5030/AM2301
                 case 12: t->koffCurr= buf&0xFF; break;         // C12-> koffCurr ��������� ����. �� ���� ���������
                 case 13: HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                          sTime.Minutes = buf; sTime.Seconds = 0;
                          HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                  break;                                        // C13-> sTime.Minutes
                 case 14: HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                          sTime.Hours = buf;
                          HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                  break;                                        // C14-> sTime.Hours
                 case 15: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          sDate.Date = buf;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                  break;                                        // C15-> sDate.Date
                 case 16: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          sDate.Month = buf;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                  break;                                        // C15-> sDate.Month
                 case 17: HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                          sDate.Year = buf;
                          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                  break;                                        // C15-> sDate.Year
               }
            } break;
           case KEY_5: servis=0; EEPSAVE = 1; waitset=1; displmode=0; psword=0; buf=0; topUser=TOPUSER; botUser=BOTUSER; t->state &=0xE7; beeper_ON(DURATION*5); break;
           case KEY_6: servis=0; EEPSAVE = 0; displmode=0; psword=0; buf=0; topUser=TOPUSER; botUser=BOTUSER; t->state &=0xE7; beeper_ON(DURATION*10); break;
          }
       }
    else if(psword==10)       //***--- ����� ������ ������ ---***
       {
        switch (keykod)
          {
           case KEY_1: setup=1; servis=0; displmode=0; buf=t->spT[0]; waitset=20; waitkey=WAITCOUNT; beeper_ON(DURATION*2); break;
           case KEY_3: if((t->state&7)==0) {servis=1; setup=0; displmode=0; waitset=20; waitkey=WAITCOUNT; beeper_ON(DURATION*2);} break;
           case KEY_6: psword=0; displmode=0; buf=0; t->state &=0xE7; servis=0; setup=0; waitkey=WAITCOUNT; beeper_ON(DURATION*10); break;// ������ ���������
           case KEY_6_5: if((t->state&7)==0){t->state|=0x01; t->state&=0x7F; beeper_ON(DURATION*2);}
                         else {t->state&=0x60; beeper_ON(DURATION*5);} countsec=-5; ok0=0; ok1=0; psword=0; EEPSAVE=1; waitset=1;
                break;
           case KEY_7_5: if((t->state&0x1F)==0) t->state|=0x80; countsec=-5; psword=0; EEPSAVE=1; waitset=1; break; //�������� ������� ������ ��� ����������� ������ !!!
           case KEY_8_5: if(t->state&0x80) t->state&=0x7F; countsec=-5; psword=0; EEPSAVE=1; waitset=1; break;      //��������� ������� ������ ��� ����������� ������ !!!
           case KEY_4_5_6_7: displmode=-10; break;
          }
       }
    else                      //***--- ����� �� ��������� ---***
       {
        switch (keykod)
          {
           case KEY_1:   ram->pvT[1]--; break;     //?????????????????????????????????
           case KEY_2:   ram->pvT[1]++; break;     //?????????????????????????????????
           case KEY_3:   buf++; if(++psword>3){psword=0;buf=0;} waitset=5; break;
           case KEY_4:   buf++; buf <<= 1; psword++; waitset=5; break;
           case KEY_5:   ++displmode; displmode&=7; waitset=255; break;//waitset=20;
           case KEY_6:   psword=0; displmode=0; buf=0; t->state &=0xE7; servis=0; waitkey=WAITCOUNT; beeper_ON(DURATION*10); break;// ������ ���������
           case KEY_7:   ram->pvT[0]--; break;     // ������� ��������� �� 10 ���.
           case KEY_8:   ram->pvT[0]++; break;     // ������� ��������� �� 10 ���.
           case KEY_7_8: t->state |=0x08; beeper_ON(DURATION*2); break;             // �������� ������� !!
           case KEY_8_6: if((t->state&7)==1){t->state |=0x02; beeper_ON(DURATION*2);} break; // �������� ����� "���������� � ����������"
//           case KEY_7_6: nextTurn=1; rotate_trays(); break;                         // �������������� �������
          };
       };
//-----------------------------------------------------------------------------------------------------------------------------------------------------
     if(psword==4) {if(buf==12) {psword=10; waitset=20; beeper_ON(DURATION*5);} else psword=0;}   // ������ ������ -> KEY_3+KEY_4+KEY_3+KEY_4 ��������� ����� �������� ������ (20 ���.)!     
//-----------------------------------------------------------------------------------------------------------------------------------------------------
  }
}

