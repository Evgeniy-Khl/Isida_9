#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv
extern UART_HandleTypeDef huart1;
extern struct eeprom *p_eeprom;
extern uint8_t waitset;
extern HAL_StatusTypeDef ret_stat;

void data_parse(struct eeprom *t){
    //HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);  // LED On
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    switch (bluetoothData.buf[4]){
    	case 0x57: t->condition = bluetoothData.buf[6]; EEPSAVE=1; waitset=1; break;  // включить/отключить камеру, только поворот
    	case 0x37: memcpy(p_eeprom,&bluetoothData.buf[6],4); EEPSAVE=1; waitset=1; break;
    	default: HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(500); HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(500);
               HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(500); HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(500);
    		break;
    }
}

void bluetoothCallback(void){
  uint8_t first, second;
  int16_t crc=0;
  union reccrc{
    uint8_t data[2];
    uint16_t val;
  }rcrc;
  first = bluetoothData.RXBuffer[0]; second = bluetoothData.RXBuffer[1];  // Receive 2 bytes
  bluetoothData.timeOut = 0;
 //если вдруг случайно превысим длину буфера
  if (bluetoothData.ind>58){
    bluetoothData.ind=0;
    HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2);
    return;
  }
  bluetoothData.buf[bluetoothData.ind] = first;
  bluetoothData.buf[bluetoothData.ind+1] = second;
  bluetoothData.ind += 2;
 // -----------  "\r\n"  --------------------------------------
  if(first==0x0D && second==0x0A){
    first = bluetoothData.buf[2]+4;
    rcrc.data[0] = bluetoothData.buf[bluetoothData.ind-4];
    rcrc.data[1] = bluetoothData.buf[bluetoothData.ind-3];
    for(int8_t i=0;i<first;i++) {
    crc += bluetoothData.buf[i];
    crc = crc ^ (crc>>2);
    }
    if(crc==rcrc.val){
      bluetoothData.buf[17] = 1;
      data_parse(p_eeprom);
    }
    else {
      bluetoothData.buf[12] = crc&0xFF;
      bluetoothData.buf[13] = (crc>>8)&0xFF;
      bluetoothData.buf[15] = bluetoothData.buf[bluetoothData.ind-4];
      bluetoothData.buf[16] = bluetoothData.buf[bluetoothData.ind-3];
      bluetoothData.buf[17] = 2;
    }
    bluetoothData.ind=0;
  }
  HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2);
}

uint8_t bluetoothName(void){
 char str1[15];
 uint8_t sizeX;
  sprintf(str1,"AT+NAMEISIDA02\r\n");//
  sizeX=strlen(str1);
  bluetoothData.ind = 0;
  ret_stat = HAL_UART_Transmit(&huart1, (uint8_t*)str1, sizeX, 0x1000);  // Передача команды str=[AT+NAME=<>]
//  ret_stat = HAL_UART_Receive(&huart1,(uint8_t*)str1,2, 0x1000); // OKIsida01
  return ret_stat;
//  bluetoothData.TXBuffer[0] = 0x0D;  bluetoothData.TXBuffer[1] = 0x0A;  // "\r=<CR>-\n=<LF>"
//  HAL_UART_Transmit(&huart1,(uint8_t*)bluetoothData.TXBuffer,2,0x1000);
//  HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2); // запуск приема
}

