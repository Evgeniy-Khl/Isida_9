#ifndef __AM2301_H
#define __AM2301_H

//--------------------------------------------------
#define OneWR2_IDR    GPIO_IDR_IDR8  //
#define OneWR2_BSRR   GPIO_BSRR_BR8  //
#define DHTport PORTC.2
#define dhtddr DDRC.2
#define dhtpin PINC.2
//--------------------------------------------------
void am2301_port_init(void);
uint8_t am2301_Start(void);
uint8_t am2301_Read(struct rampv *ram, uint8_t biasHum);

#endif /* __AM2301_H */
