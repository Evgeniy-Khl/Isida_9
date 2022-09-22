#ifndef __POWER_H
#define __POWER_H

void OutPulse(uint8_t cn);
uint8_t RelayPos(uint8_t cn, uint8_t offSet);
uint8_t RelayNeg(uint8_t cn, uint8_t offSet, uint8_t bias );



#endif /* __POWER_H */
