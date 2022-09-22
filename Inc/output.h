#ifndef __OUTPUT_H
#define __OUTPUT_H

uint8_t heatCondition(int16_t err, uint8_t alarm, uint8_t extOn);
uint8_t humCondition(int16_t err, uint8_t alarm, uint8_t extOn);
int16_t UpdatePID(int16_t err, uint8_t cn, struct eeprom *t);
uint16_t heater(int16_t err, struct eeprom *t);
uint16_t humidifier(int16_t err, struct eeprom *t);
uint8_t RelayNeg(int16_t err, uint8_t cn, int8_t extOn, int8_t extOff);
void extra_2(struct eeprom *t, struct rampv *ram);
void OutPulse(int16_t err, struct eeprom *t);

#endif /* __OUTPUT_H */

