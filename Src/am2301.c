#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv

extern uint8_t ds18b20_amount;

//--------------------------------------------------
__STATIC_INLINE void DelayMicro(__IO uint32_t micros){
  micros *= (SystemCoreClock / 1000000) / 8;
  while (micros--) ;  // Wait till done
}
//--------------------------------------------------
void am2301_port_init(void){
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);
  GPIOA->CRH |= GPIO_CRH_MODE12;        // ����� ����� � �������� 12
  GPIOA->CRH |= GPIO_CRH_CNF12_0;       // ����� � �������� �����������
  GPIOA->CRH &= ~GPIO_CRH_CNF12_1;
}
//-----------------------------------------------
uint8_t am2301_Start(void){
  uint8_t status;
  GPIOA->BSRR = GPIO_BSRR_BR12;//������ ������� (�������� 12 ��� ����� A )
  DelayMicro(5000);//�������� ��� ������� �� 800 �����������
  GPIOA->BSRR = GPIO_BSRR_BS12;//������� ������� (���������� 12 ��� ����� A )
  DelayMicro(60);//�������� �� 60 ����������� (wait for DHT respond 20-40uS)
  status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);//��������� ������� (low-voltage-level response signal & keeps it for 80us)
  if(status) return 0;// status==1 ������ �� ������� ������ �������
  DelayMicro(80);//�������� �� 80 �����������
  status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);//��������� ������� (hi-voltage-level response signal & keeps it for 80us)
  return status;//������ ���������. status==0 ���� ����� �������� � ������ ������. ����� �������� 46 us �� ������� ������.
}
//--------------------------------------------------
uint8_t am2301_Read(struct rampv *ram, int8_t biasHum){
  uint8_t i, j, status=0, crc, tem[5];
  int16_t result;
  static uint8_t err;
  if(am2301_Start()){
    DelayMicro(60);// �� ������� ������ �������� 46 us. �������� �� 60 �����������
    for(i=0; i<5; i++){
      tem[i]=0;
      for(j=0; j<8; j++){
        tem[i]<<= 1;
        status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);//��������� �������
        while(!status) {status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);}// ������� (hi-voltage-level)
        crc = 0;
        while(status) {
          DelayMicro(1); crc++;
          status = (GPIOA->IDR & GPIO_IDR_IDR12 ? 1 : 0);
        }// ������� (low-voltage-level)
        
        if(crc > 23) tem[i]|= 1;  // data "1" 12<crc<33
      }
    }
    crc=tem[0]+tem[1]+tem[2]+tem[3];
    if(crc==tem[4]){
      result =((int)tem[0]*256+tem[1] + biasHum)/10;            // ��������� ������� ���������
      if(ds18b20_amount<3) ram->pvT[2] =(int)tem[2]*256+tem[3]; // DHT21
      if(result > 100) result=100; else if(result < 0) result=0;
      ram->pvRH = result;
      err = 0;
      status=1;
    }
    else if(++err>3) {status=0; ram->pvRH = 150; ram->errors |= 0x02;}
  }
  else {ram->pvRH = 199; ram->pvT[ds18b20_amount] = 1999; ram->errors |= 0x02;}
  return status;
}
//-----------------------------------------------
