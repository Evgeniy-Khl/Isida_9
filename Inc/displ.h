#ifndef __DISPL_H
#define __DISPL_H

#define NOCOMMA   0
#define COMMA     1
#define ERRORS    2
#define FUSES     3
#define SETUP     4
#define SETUP2    5
#define SERVIS    6
#define CONTROL   7
#define PASS      8
#define VERS      9
#define MODUL     10
#define DISPL     11

//void display(struct eeprom *t, struct rampv *ram);
//void display_setup(struct eeprom *t);
//void display_servis(struct rampv *ram);
void ledOut(uint8_t condition, uint8_t fuses);
void displ_1(int16_t val, uint8_t comma);
void displ_2(int16_t val, uint8_t comma);
void displ_3(int16_t val, int8_t mode, int8_t blink);
void clr_1(void);
void clr_2(void);
void clr_3(void);

#endif /* __DISPL_H */
