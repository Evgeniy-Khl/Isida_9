#include <stdbool.h>
#include "rtc.h"
#include "my.h"

//extern RTC_HandleTypeDef hrtc;
extern MY_TimeTypeDef sTime;
extern MY_DateTypeDef sDate;

//----------- Функція встановлення дати і часу ----------------------------------------
void setDataAndTime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday, 
             uint8_t hour, uint8_t min,  uint8_t sec, uint32_t format){
    /**Initialize RTC and set the Time and Date**/
  sTime.Hours = hour;
  sTime.Minutes = min;
  sTime.Seconds = sec;

//  if (HAL_RTC_SetTime(&hrtc, &sTime, format) != HAL_OK){
//    Error_Handler();
//  }

  sDate.WeekDay = weekday;
  sDate.Month = month;
  sDate.Date = day;
  sDate.Year = year;

//  if (HAL_RTC_SetDate(&hrtc, &sDate, format) != HAL_OK){
//    Error_Handler();
//  }
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

