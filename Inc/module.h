#ifndef __MODULE_H
#define __MODULE_H

//--------------------------------------------------
#define OneWR2_IDR    GPIO_IDR_IDR8  //
#define OneWR2_BSRR   GPIO_BSRR_BR8  //
//--------------------------------------------------
int8_t module_check(uint8_t fc);
int8_t chkcooler(uint8_t state);
int8_t chkhorizon(uint8_t state);
int8_t chkflap(uint8_t cmd, uint8_t *pvF);
#endif /* __MODULE_H */
