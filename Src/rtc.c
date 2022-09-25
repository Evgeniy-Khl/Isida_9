#include "rtc.h"

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

extern int16_t set[];

extern uint8_t Y_txt, X_left, Y_top, Y_bottom, card, newDate;
extern int16_t fillScreen;
extern char buffTFT[];
//----------- Функція встановлення дати і часу ----------------------------------------
void setDataAndTime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday, 
             uint8_t hour, uint8_t min,  uint8_t sec, uint32_t format){
    /**Initialize RTC and set the Time and Date**/
  sTime.Hours = hour;
  sTime.Minutes = min;
  sTime.Seconds = sec;

  if (HAL_RTC_SetTime(&hrtc, &sTime, format) != HAL_OK){
    Error_Handler();
  }

  sDate.WeekDay = weekday;
  sDate.Month = month;
  sDate.Date = day;
  sDate.Year = year;

  if (HAL_RTC_SetDate(&hrtc, &sDate, format) != HAL_OK){
    Error_Handler();
  }
}

//----- Функція збереження дати до backup регістрів

void writeDateToBackup(uint32_t bkp_reg){
 uint16_t dt;
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); 
  dt = sDate.Year;
  dt <<= 4;
  dt |= sDate.Month;
  dt <<= 5;
  dt |= sDate.Date;
  HAL_RTCEx_BKUPWrite(&hrtc, bkp_reg, dt);
}

//----- Функція відновлення дати з backup регістрів
void readBackupToDate(uint32_t bkp_reg){
 uint16_t reg;

  reg = HAL_RTCEx_BKUPRead(&hrtc,bkp_reg);
  hrtc.DateToUpdate.Date = reg & 0x001f;
  reg >>= 5;
  hrtc.DateToUpdate.Month = reg & 0x000f;
  reg >>= 4;
  hrtc.DateToUpdate.Year = reg;
  
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
//  sprintf(buffTFT,"R1: %02u:%02u:%02u    %02u.%02u.20%02u", sTime.Hours, sTime.Minutes, sTime.Seconds, sDate.Date, sDate.Month, sDate.Year);
//  ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, fillScreen);
//  Y_txt = Y_txt+18+5;
  
  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

uint32_t colodarToCounter (void)
{
	uint8_t a;
	int16_t y;
	uint8_t m;
	uint32_t time;

	a=((14-sDate.Month)/12);// 14-1=13/12=1
	y=sDate.Year+6800-a;    // 1970+4800-1=6769
	m=sDate.Month+(12*a)-3; // 1+12-3=10
  // Вычисляем значение текущего Юлианского дня
  time=sDate.Date;
  time+=(153*m+2)/5;
  time+=365*y;
  time+=y/4;
  time-=y/100;
  time+=y/400;
  time-=32045;
  time-=2440588;
  time*=86400;     // переводим дни в секунды
	time+=sTime.Seconds;
  time+=sTime.Minutes*60;
  time+=sTime.Hours*3600;
	return time;
}

