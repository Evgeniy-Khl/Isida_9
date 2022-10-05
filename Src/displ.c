#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv
#include "displ.h"
#include "proc.h"

extern int8_t displmode, countsec, disableBeep, keynum;
extern uint8_t ok0, ok1, psword, setup, servis, cardOk;
extern int16_t buf, currAdc, humAdc;

//------- Светодиодная индикация --------------------------------------------------------- 
// 0-нагрев, 1-увлаж.,2-заслонка,3-дополнит.,4-лотки,5-,6-программа,7-SD карта
// 0-обрыв нагрев, 1-преохр.увлаж.,2-преохр.заслонка,3-преохр.дополнит.,4-ДВЕРЬ,5-ОСТАНОВ вент.,6-НЕТ поворота,7-ГОРИЗОНТ
void ledOut(uint8_t condition, uint8_t warning, uint8_t fuses){
 uint8_t led, i;
  for(i=0;i<8;i++) LedOff(i,3);     // All LED Off
  led = portOut.value & 0x0F;       // portOut
  i=0;
  while (i<4){if(led & 1) LedOn(i,1);	led >>= 1; i++;}    // 0b00001111
  led = fuses & 0x0F;               // fuses
  i=0;
  while (i<4){if(led & 1) LedOn(i,2);	led >>= 1; i++;}    // 0b00001111
  if(TURN) LedOn(4,1);  // лотки вверху
  if(countsec&1){
    if(condition&0x02) LedOn(5,1);  // Режим "подгототка к ОХЛАЖДЕНИЮ"
//    if(prog) LedOn(6,1);  // работает по программе
    if(cardOk) LedOn(7,1);  // идет запись на SD карту
    if(!HAL_GPIO_ReadPin(Door_GPIO_Port, Door_Pin)) LedOn(4,2);  // концевик дверей
    if(warning&0x20) LedOn(5,2);    // ОСТАНОВ вентилятора    
    if(warning&0x40) LedOn(6,2);    // НЕТ поворота
    if(condition&0x10) LedOn(7,2);  // ГОРИЗОНТ  установлен
  }
}

void displ_1(int16_t val, uint8_t comma){
 uint8_t neg=0;
  if (val<0) {neg=1; val=-val;}
  if (val<1000){
    if (neg){
       if (val<100){
         setChar(0, SIMBL_MINUS); 
         setChar(1, (val/10)%10); 
         if (comma) PointOn(1); else PointOff(1);
         setChar(2, val%10);
       }
       else {
          setChar(0, SIMBL_MINUS);
          setChar(1, val/100);
          setChar(2, (val/10)%10);
       }
    }
    else {
       if (val<100){
         setChar(0, SIMBL_BL);
         setChar(1, (val/10)%10); 
         if (comma) PointOn(1); else PointOff(1);
         setChar(2, val%10);
        }
       else {
         setChar(0, val/100);
         setChar(1, (val/10)%10); 
         if (comma) PointOn(1); else PointOff(1);
         setChar(2, val%10);
       }
    }
  }
  else {
     setChar(0, SIMBL_MINUS);// -> -
     setChar(1, SIMBL_MINUS);// -> -
     setChar(2, SIMBL_MINUS);// -> -
  }
}

void displ_2(int16_t val, uint8_t comma){
 uint8_t neg=0;
  if (comma) comma=0x80;
  if (val<0) {neg=1; val=-val;}
  if (val<1000){
    if (neg){
       if (val<100){
         setChar(3, SIMBL_MINUS); 
         setChar(4, (val/10)%10); 
         if (comma) PointOn(4); else PointOff(4);
         setChar(5, val%10);
       }
       else {
          setChar(3, SIMBL_MINUS);
          setChar(4, val/100);
          setChar(5, (val/10)%10);
       }
    }
    else {
       if (val<100){
         setChar(3, SIMBL_BL);
         setChar(4, (val/10)%10); 
         if (comma) PointOn(4); else PointOff(4);
         setChar(5, val%10);
        }
       else {
         setChar(3, val/100);
         setChar(4, (val/10)%10); 
         if (comma) PointOn(4); else PointOff(4);
         setChar(5, val%10);
       }
    }
  }
  else {
     setChar(3, SIMBL_MINUS);// -> -
     setChar(4, SIMBL_MINUS);// -> -
     setChar(5, SIMBL_MINUS);// -> -
  }
}

void clr_1(void){
	uint8_t byte;
	for (byte=0; byte<3; byte++) setChar(byte, SIMBL_BL);
}

void clr_2(void){
	uint8_t byte;
	for (byte=3; byte<6; byte++) setChar(byte, SIMBL_BL);
}

void clr_3(void){
	uint8_t byte;
	for (byte=6; byte<8; byte++) setChar(byte, SIMBL_BL);
}

void displ_3(int16_t val, int8_t mode, int8_t blink){
	uint8_t chr;
	switch (mode){
		case ERRORS:  chr=DISPL_A;    break;  // A
		case FUSES:   chr=DISPL_Pe;   break;  // П
    case SETUP:   chr=DISPL_MBott; break; // MINUS Bott
    case SETUP2:  chr=DISPL_M_Top; break; // MINUS Top
		case SERVIS:  chr=DISPL_c;    break;  // c
		case CONTROL: chr=DISPL_P;    break;  // P
		case PASS:    chr=DISPL_TOPn; break;  // TOPn
    case VERS:    chr=DISPL_u;    break;  // u
    case MODUL:   chr=DISPL_o;    break;  // o
		case DISPL:   chr=DISPL_d;    break;  // d
		default:      chr=SIMBL_BL;
	}
  switch (blink){
  	case 2: if(countsec&1){val=0; chr=SIMBL_BL;} break; // мигание дисплея без звука
  	case 1: if(countsec&1){val=0; chr=SIMBL_BL;}        // мигание дисплея со звуком
            else if(disableBeep==0) beeper_ON(DURATION*5);
  		break;
  }
	if (val<100){
		if (val==-10){
			setChar(6, DISPL_Pe); // П
			setChar(7, DISPL_A);  // A     
		}
		else if (val==0){
			setChar(6, chr);
			setChar(7, SIMBL_BL);
		}
		else if (val<10){
			setChar(6, chr);
			setChar(7, val+0xA);
		}
		else {
			setChar(6, val/10+0xA); 
			setChar(7, val%10+0xA);
		}
	}
	else {
		setChar(6, chr);
		setChar(7, DISPL_TOPo);
	}
}


void display(struct eeprom *t, struct rampv *ram){
 int8_t xx=0, yy=CONTROL, blink = 1;  
  switch (displmode){
    case 0: 
       displ_1(ram->pvT[0],COMMA); if(HIH5030||AM2301) displ_2(ram->pvRH,NOCOMMA); else displ_2(ram->pvT[1],COMMA);
       if(psword==10) {xx=-10;  blink=0;}                           // пароль введен
       else if(psword>0) {xx=psword; yy=PASS; blink=0;}             // вводится пароль
       else if(ram->errors&0x70) xx=66+((ram->errors&0x70)>>4);     // мигает "xx" двузначный код ошибки
       else if(ram->errors&0x0F) xx=50+(ram->errors&0x0F);          // мигает "xx" двузначный код ошибки
       else if(ram->warning&0x70) xx=37+((ram->warning&0x70)>>4);   // мигает "xx" двузначный код предупреждения
       else if(ram->warning&0x0F) xx=22+(ram->warning&0x0F);        // мигает "xx" двузначный код предупреждения
       else if(ram->fuses&0x70) {xx=15+((ram->fuses&0x70)>>4);}     // мигает "xx" двузначный код предупреждения (дверь открыта; останов вент.; нет поворота)
       else if(ram->fuses&0x0F) {xx=(ram->fuses&0x0F); yy=FUSES;}   // мигает "Пx" код предохранителя
       else if(t->condition&0x01) {xx=1; yy=CONTROL; blink=0;}          // Режим "P1" "ИНКУБАЦИЯ"
       else if(t->condition&0x02) {xx=2; yy=CONTROL; blink=0;}          // Режим "P2" "подгототка к ОХЛАЖДЕНИЮ"
       else if(t->condition&0x04) {xx=3; yy=CONTROL; blink=0;}          // Режим "P3" "подгототка к ВКЛЮЧЕНИЮ"
       else if((t->condition&0x18)==0x08) {xx=4; yy=CONTROL; blink=0;}  // "P4" ГОРИЗОНТ УСТАНОВЛЕН
       else if(t->condition&0x80) {xx=5; yy=CONTROL; blink=0;}          // "P5" Поворот лотков при ОТКЛЮЧЕННОЙ камере !!!
       else if(VENTIL) {xx=6; yy=CONTROL; blink=0;}                     // "P6" Углекислый газ или Проветривание
       else if(t->condition==0) {xx=ram->cellID&0x1F; yy=0; blink=2;}   // ОТКЛЮЧЕН! показываем номер Блока "xx"
       else {xx=0; yy=SETUP;}                                           // неизвестный режим
       break;
    //---------------уставка t0;-------------------------уставка RH;---------------------уставка t1;-----------------"d1"---------
    case 1: displ_1(t->spT[0],COMMA); if(HIH5030||AM2301) displ_2(t->spRH[1],NOCOMMA); else displ_2(t->spT[1],COMMA); xx=displmode; yy=DISPL; blink=0; break;
    //-------------------t1;--------------------t2;------------------"d2"---------
    case 2: displ_1(ram->pvT[1],COMMA); displ_2(ram->pvT[2],COMMA); xx=displmode; yy=DISPL; blink=0; break;
    //--------------- CO2 ------------------------ flap ----------------------"d3"------------------
//    case 3: displ_1(ram->pvCO2[0]/10,NOCOMMA); displ_2(ram->pvFlap,NOCOMMA); xx=displmode; yy=DISPL; blink=0; break;
    case 3: displ_1(currAdc,NOCOMMA); displ_2(humAdc,NOCOMMA); xx=displmode; yy=DISPL; blink=0; break;
  }
  if(ok0>1) if(countsec&1) clr_1();
  if(ok1>1) if(countsec&1) clr_2();
  displ_3(xx,yy,blink);
  SendDataTM1638();
}

void display_setup(struct eeprom *t){
	if(buf>999) buf=999; else if(buf<-99) buf=-99;
	switch (setup){
		case 1: displ_1(buf,COMMA); clr_2(); break;                    // "_1"
		case 2: if(HIH5030||AM2301){clr_1(); displ_2(buf,NOCOMMA);} else {displ_1(buf,COMMA); clr_2();} break;// "_2"
		case 3: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2(); break; // "_3" время отключенного состояния
		case 4: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2(); break; // "_4" время включенного состояния (если не 0 то это секунды)
		case 5: displ_1(buf,COMMA); clr_2(); break;                    // "_5" тревога по каналу 1
		case 6: if(HIH5030||AM2301){clr_1(); displ_2(buf,NOCOMMA);} else {displ_1(buf,COMMA); clr_2();} break;// "_6" тревога по каналу 2
		case 7: displ_1(buf,COMMA); clr_2(); break;                    // "_7" смещение для ВКЛ. вспомогательного канала 1
		case 8: displ_1(buf,COMMA); clr_2(); break;                    // "_8" смещение для ОТКЛ. вспомогательного канала 1
		case 9:                                                        // "_9" смещение для ВКЛ. вспомогательного канала 2
			if(t->extendMode==4){clr_1(); if(HIH5030||AM2301) displ_2(buf,NOCOMMA); else displ_2(buf,COMMA);}
			else {displ_1(buf,COMMA); clr_2();}
			break;
		case 10:                                                       // "10" смещение для ОТКЛ. вспомогательного канала 2
			if(t->extendMode==4){clr_1(); if(HIH5030||AM2301) displ_2(buf,NOCOMMA); else displ_2(buf,COMMA);}
			else {displ_1(buf,COMMA); clr_2();}
			break;

		case 19: clr_1(); displ_2(buf,COMMA); break;                   // "u3" = 200 - 1,0 сек.
		case 20: clr_1(); displ_2(buf,COMMA); break;                   // "u4" = 2000- 10,0 сек.
		case 21: clr_1(); displ_2(buf,NOCOMMA); break;                 // "u5" = 3000-  15 сек.

		case 26: clr_1(); displ_2(buf,NOCOMMA); break;                 // "~1" подстройка датчика HIH-4000
		case 27: clr_1(); displ_2(buf,NOCOMMA); break;                 // "~2" гистерезис
		default: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2();
	}
	if (setup > 25) displ_3(setup-25,SETUP2,0);
  else if (setup > 15) displ_3(setup-16,VERS,0);
  else displ_3(setup,SETUP,0);
	SendDataTM1638();
}

void display_servis(struct rampv *ram){
	switch (servis){
		case 1: displ_1(currAdc,COMMA); clr_2(); break;                               // C1 -> НАГРЕВ; Сила тока
		case 2: displ_1(ram->pvRH,NOCOMMA); displ_2(humAdc,NOCOMMA); break;           // C2 -> ВЛАЖНОСТЬ; значение АЦП
		case 3: displ_1(ram->pvCO2[0],NOCOMMA); displ_2(ram->pvFlap,NOCOMMA); break;  // C3 -> ЗАСЛОНКА; СО2, СЕРВОПРИВОД град.
		case 8: displ_1(buf,COMMA); clr_2(); break;                                   // C8 -> Уставка форсированного нагрева
		default: displ_1(buf,NOCOMMA); clr_2();
	}
	displ_3(servis,SERVIS,0);
	SendDataTM1638();
}

