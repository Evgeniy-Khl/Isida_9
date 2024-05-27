/*
FR_OK (0) - �������� ���������� ��������.
FR_DISK_ERR (1) - ��������� ������ �� ������ ������ �����-������.
FR_INT_ERR (2) - ��������� ���������� �����������.
FR_NOT_READY (3) - ���������� ������ �� ����� � ������.
FR_NO_FILE (4) - �� ������� ����� ����.
FR_NO_PATH (5) - �� ������� ����� ����.
FR_INVALID_NAME (6) - ������������ ������ ����� ����.
FR_DENIED (7) - ������ �������� ��-�� ����������� ��� ������������ ��������.
FR_EXIST (8) - ������ �������� ��-�� ������������ �����������.
FR_INVALID_OBJECT (9) - ���������������� ������ �����/��������.
FR_WRITE_PROTECTED (10) - ���������� ������ ������� �� ������.
FR_INVALID_DRIVE (11) - ���������������� ���������� ����� �������.
FR_NOT_ENABLED (12) - ����������� ������� ������� ��� ����.
FR_NO_FILESYSTEM (13) - �� ������� �������������� �������� ������� FAT.
FR_MKFS_ABORTED (14) - ��������� ���������� f_mkfs() ��-�� ������ ���������.
FR_TIMEOUT (15) - �� ������� �������� ���������� �� ������ � ���� � ������� ��������� �������.
FR_LOCKED (16) - �������� ��������� ��-�� �������� ����������� ������������� ������.
FR_NOT_ENOUGH_CORE (17) - �� ������� �������� ����� ��� ������ � �������� ������� ������.
FR_TOO_MANY_OPEN_FILES (18) - ����� �������� ������ ��������� _FS_SHARE.
FR_INVALID_PARAMETER (19) - ���������� �������� ��������������.
*/
#include "main.h"
#include "global.h"
#include "FatFsAPI.h"
#include "rtc.h"
#include "my.h"

extern char USERPath[]; /* logical drive path */
extern char fileName[];
extern uint8_t cardOk;
extern uint32_t unixTime;
extern union d4v file;
extern struct eeprom *p_eeprom;
extern struct rampv *p_rampv;
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

FATFS SDFatFs;
DWORD fre_clust, fre_sect, tot_sect;
FATFS *fs;
FIL MyFile;
FILINFO fileInfo;
DIR dir;

uint32_t bwrt;   // ��� �������� �� ������� ���������� ������� ���������� ����

  //write ----------------------------------------------------------------------------------------------------
uint8_t SD_write_txt(struct eeprom *t, struct rampv *ram){
  uint8_t item;
  char buffile[90], txt[30];
  uint32_t f_size = MyFile.fsize;
  item = f_lseek(&MyFile, f_size);
  if (item==FR_OK){ // ���� ���� �����
    unixTime = timestamp(); //  ������� � unixTime
    sprintf(buffile,"%u; %2x;%5.1f;%5.1f;%5.1f;%5.1f;",unixTime,t->state,(float)ram->pvT[0]/10,(float)t->spT[0]/10,(float)ram->pvT[1]/10,(float)t->spT[1]/10);
    if(ram->pvRH<=100) sprintf(txt,"%4u%%;%3u%%;",ram->pvRH,t->spRH[1]);
    else sprintf(txt," ---;---;");
    strcat(buffile,txt);
    sprintf(txt,"%3u%%; %02x; %02x; %02x; %02x;  %02u:%02u:%02u\r\n",ram->power,portOut.value,ram->warning,ram->errors,ram->fuses,sTime.Hours,sTime.Minutes,sTime.Seconds);
    strcat(buffile,txt);
    for(item=0;item<100;item++){if (buffile[item]==0) break;}
    item = f_write(&MyFile, buffile, item,(void*)&bwrt);    // if(item==FR_OK) file changed successfully!
    if((bwrt==0)||(item!=FR_OK)) {cardOk = 0; FATFS_UnLinkDriver(USERPath);}
  }
  else {cardOk = 0; FATFS_UnLinkDriver(USERPath);}
  f_close(&MyFile);
  f_mount(NULL,(TCHAR const*)USERPath,0);
  return item;
}

//-- My_LinkDriver --------------------------------------------------------------
uint8_t My_LinkDriver(const char* ext){
 uint8_t item;
 char buffile[80];
  cardOk = 0;
  FATFS_LinkDriver(&USER_Driver, USERPath);
  if(f_mount(&SDFatFs,(const char*)USERPath,0)!=FR_OK) item = 9;  // �� ����������� SD �����!
  else {  // SD ����� ������������.
    char fullfileName[13]={0};
    memcpy(fullfileName, fileName, 8);// ����������� ��� ����� � fullFileName
    strcat(fullfileName,ext);         // �������� ���������� ".txt"
    item = f_open(&MyFile, fullfileName, FA_OPEN_EXISTING|FA_WRITE);  // ��������� ����
    switch (item){
    	case FR_NOT_READY:  break;    // (3) The physical drive cannot work   SD ����� �� ���������!
    	case FR_DISK_ERR:   break;    // (1) A hard error occurred in the low level disk I/O layer    ������ SD �����!
      case FR_EXIST:                // (8) SD ����� ��������� � ���� ��� ���� �� �����.
        f_close(&MyFile); cardOk = 1;
        break;
      case FR_NO_FILE:              // (4) Could not find the file
        // ���� ����� ��� �� ...
        item = f_open(&MyFile, fullfileName, FA_CREATE_NEW|FA_WRITE);  // �������� ������� ����!
        if(item==FR_OK){// ��������� ������ ������...
          if(strstr(fullfileName, ".txt")) {
            sprintf(buffile,"TimeStamp;  St   T1   [U1]   T2   [U2]  RH  [Urh]  Pwr Out  Wa  Er  Fu\r\n");
          } 
          else if(strstr(fullfileName, ".eep")){
            unixTime = timestamp(); //  ������� � unixTime  
            sprintf(buffile,"TimeStamp:%u; d0=%u; d1=%u; d2=%u d3=%u;  %02u:%02u:%02u\r\n",unixTime,file.data[0],file.data[1],file.data[2],file.data[3],sTime.Hours,sTime.Minutes,sTime.Seconds);
          }
          for(item=0; item<80; item++){if (buffile[item]==0) break;}
          item = f_write(&MyFile, buffile, item, (void*)&bwrt);
          if((bwrt == 0)||(item!=FR_OK)) cardOk = 0;       // ������ �������� ���������!
          else {f_close(&MyFile); cardOk = 1;}             // ����� ���� ������!
        }
        break;
      case FR_OK: cardOk = 1; 
        if(strstr(fullfileName, ".txt")){
          SD_write_txt(p_eeprom, p_rampv);
        }
        else if(strstr(fullfileName, ".eep")){
          uint32_t f_size = MyFile.fsize;
          item = f_lseek(&MyFile, f_size);// ���� ����� �����
          if (item==FR_OK){ // ����� ����� �����
            unixTime = timestamp(); //  ������� � unixTime
            sprintf(buffile,"TimeStamp:%u; d0=%u; d1=%u; d2=%u d3=%u;  %02u:%02u:%02u\r\n",unixTime,file.data[0],file.data[1],file.data[2],file.data[3],sTime.Hours,sTime.Minutes,sTime.Seconds);
            for(item=0;item<100;item++){if (buffile[item]==0) break;}
            item = f_write(&MyFile, buffile, item,(void*)&bwrt);    // if(item==FR_OK) file changed successfully!
            if((bwrt==0)||(item!=FR_OK)) {cardOk = 0; FATFS_UnLinkDriver(USERPath);}
          }
          f_close(&MyFile);
        }
        break;        // sprintf(buffile,"SD ����� ����������: 0x%02X",sdinfo.type);
    }
    f_mount(NULL,(TCHAR const*)USERPath,0); // Pointer to the file system object (NULL:unmount); 0:Do not mount (delayed mount)
  }
  if(cardOk==0) FATFS_UnLinkDriver(USERPath);
  return item;
}


