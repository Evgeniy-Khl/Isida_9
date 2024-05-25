#include "main.h"
#include "global.h"
#include "FatFsAPI.h"
#include "rtc.h"
#include "my.h"

extern char USERPath[]; /* logical drive path */
extern char fileName[];
extern uint8_t cardOk;
extern uint32_t unixTime;

FATFS SDFatFs;
DWORD fre_clust, fre_sect, tot_sect;
FATFS *fs;
FIL MyFile;
FILINFO fileInfo;
DIR dir;

char buffile[LEN_BUFF], txt[25];
uint32_t bwrt;   // для возврата из функции количества реально записанных байт

//-- My_LinkDriver --------------------------------------------------------------
uint8_t My_LinkDriver(void){
 uint8_t item;
  cardOk = 0;
  FATFS_LinkDriver(&USER_Driver, USERPath);
  if(f_mount(&SDFatFs,(const char*)USERPath,0)!=FR_OK) item = 9;  // Не монтируется SD карта!
  else {
    item = f_open(&MyFile, fileName, FA_OPEN_EXISTING|FA_WRITE);
    switch (item){
    	case FR_NOT_READY:  break;    // (3) The physical drive cannot work   SD карта не вставлена!
    	case FR_DISK_ERR:   break;    // (1) A hard error occurred in the low level disk I/O layer    Ошибка SD карты!
      case FR_EXIST:                // (8) SD карта вставлена и файл уже есть на диске.
        f_close(&MyFile); cardOk = 1;
        break;
      case FR_NO_FILE:              // (4) Could not find the file
        // если файла нет то ...
        item = f_open(&MyFile, fileName, FA_CREATE_NEW|FA_WRITE);  // Пытаемся создать файл!
        if(item==FR_OK){// формируем первую строку...
          sprintf(buffile,"TimeStamp;  St   T1  [U1]   T2  [U2]  RH  [Urh] Pwr Out Wa Er Fu\r\n");
          for(item=0; item<LEN_BUFF; item++){if (buffile[item]==0) break;}
          item = f_write(&MyFile, buffile, item, (void*)&bwrt);
          if((bwrt == 0)||(item!=FR_OK)) cardOk = 0;       // Немогу записать заголовок!
          else {f_close(&MyFile); cardOk = 1;}             // Новый файл создан!
        }
        break;
      case FR_OK: cardOk = 1; break;        // sprintf(buffile,"SD карта определена: 0x%02X",sdinfo.type);
    }
    f_mount(NULL,(TCHAR const*)USERPath,0); // Pointer to the file system object (NULL:unmount); 0:Do not mount (delayed mount)
  }
  if(cardOk==0) FATFS_UnLinkDriver(USERPath);
  return item;
}

  //write ----------------------------------------------------------------------------------------------------
uint8_t SD_write(const char* flname, struct eeprom *t, struct rampv *ram){
 uint8_t item;
  item = f_mount(&SDFatFs,(const char*)USERPath,0);
  if(item==FR_OK){      // Монтируем SD карту!
    item = f_open(&MyFile, flname, FA_OPEN_EXISTING|FA_WRITE);
    if(item==FR_OK){    // Открываем файл
      uint32_t f_size = MyFile.fsize;
      item = f_lseek(&MyFile, f_size);
      if (item==FR_OK){ // Ищем конц файла
        unixTime = timestamp(); //  персчет в unixTime
        sprintf(buffile,"%u; %2x;%5.1f;%4.1f;%5.1f;%4.1f;",unixTime,t->state,(float)ram->pvT[0]/10,(float)t->spT[0]/10,(float)ram->pvT[1]/10,(float)t->spT[1]/10);
        if(ram->pvRH<=100) sprintf(txt,"%5u%%;%5u%%;",ram->pvRH,t->spRH[1]);
        else sprintf(txt," --.-;--.-;");
        strcat(buffile,txt);
        sprintf(txt,"%3u; %2x;%2x;%2x;%2x\r\n",ram->power,portOut.value,ram->warning,ram->errors,ram->fuses);
        strcat(buffile,txt);
        for(item=0;item<LEN_BUFF;item++){if (buffile[item]==0) break;}
        item = f_write(&MyFile, buffile, item,(void*)&bwrt);    // if(item==FR_OK) file changed successfully!
        if((bwrt==0)||(item!=FR_OK)) {cardOk = 0; FATFS_UnLinkDriver(USERPath);}
      }
      else {cardOk = 0; FATFS_UnLinkDriver(USERPath);}
      f_close(&MyFile);
      f_mount(NULL,(TCHAR const*)USERPath,0);
    }
    else {cardOk = 0; FATFS_UnLinkDriver(USERPath);}
  }
  return item;
}

