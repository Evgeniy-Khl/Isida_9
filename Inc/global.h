/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H

extern struct rampv {
    uint8_t model;       // 1 ���� ind=0  ������ �������
    uint8_t node;        // 1 ���� ind=1  ������� ����� �������
    int16_t pvT[MAX_DEVICES]; // 6 ���� ind=2-ind=7   �������� [MAX_DEVICES=3] �������� �����������
    uint8_t pvRH;        // 1 ���� ind=8  �������� ������� ������������� ���������
    uint8_t pvCO2;       // 1 ���� ind=9  �������� ������� CO2 (�� 4 -> 400 �� 50 -> 5000)
    uint8_t nextTurn;    // 1 ���� ind=10 �������� ������� �� ������ �������� ������
    uint8_t fan;         // 1 ���� ind=11 �������� �������� ����������� �����������
    uint8_t flap;        // 1 ���� ind=12 ��������� �������� 
    uint8_t power;       // 1 ���� ind=13 �������� ���������� �� ����
    uint8_t fuses;       // 1 ���� ind=14 �������� ��������� 
    uint8_t errors;      // 1 ���� ind=15 ������
    uint8_t warning;     // 1 ���� ind=16 ��������������
    uint8_t nothing0;    // 1 ���� ind=17
    uint8_t nothing1;    // 1 ���� ind=18
    uint8_t nothing2;    // 1 ���� ind=19
  } pv;// ------------------ ����� 20 bytes -------------------------------

extern struct eeprom {
    int16_t spT[2];     // 4 ���� ind=0-ind=3   ������� ����������� sp[0].spT->����� ������; sp[1].spT->������� ������
    //spRH[0]->���������� HIH-5030/AM2301; spRH[1]->������� ��������� ������ HIH-5030/AM2301
    int8_t  spRH[2];    // 2 ���� ind=4;ind=5   
    uint8_t state;      // 1 ���� ind=6;        ��������� ������ (����. ���. ����������, � �.�.)
    uint8_t extendMode; // 1 ���� ind=7;        ����������� ����� ������  0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
    uint8_t relayMode;  // 1 ���� ind=8;        �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1] 4->�� ���.[1] ���������� �����
    uint8_t programm;   // 1 ���� ind=9;        ������ �� ���������
    uint8_t minRun;     // 1 ���� ind=10        ���������� ���������� ������� �����������
    uint8_t maxRun;     // 1 ���� ind=11        ���������� ���������� ������� �����������
    uint8_t period;     // 1 ���� ind=12        ���������� ���������� ������� �����������
    uint8_t timer[2];   // 2 ���� ind=13;ind=14 [0]-������.��������e [1]-�����.��������e
    uint8_t alarm[2];   // 2 ���� ind=15;ind=16 ������ 5 = 0.5 ��.C
    uint8_t extOn[2];   // 2 ���� ind=17;ind=18 �������� ��� ���. ���������������� ������
    uint8_t extOff[2];  // 2 ���� ind=19;ind=20 �������� ��� ����. ���������������� ������
    uint8_t air[2];     // 2 ���� ind=21;ind=22 ������ ������������� air[0]-�����; air[1]-������; ���� air[1]=0-���������
    uint8_t spCO2;      // 1 ���� ind=23;       ������� �������� ��� ���������� ������������ ��2
    uint8_t koffCurr;   // 1 ���� ind=24;       ��������� ����. �� ���� ���������  (150 ��� AC1010)
    uint8_t hysteresis; // 1 ���� ind=25;       ���������� ������ ����������
    uint8_t zonaFlap;   // 1 ���� ind=26;       ����� ����������� � ������
    uint8_t turnTime;   // 1 ���� ind=27;       ����� �������� ������� ������ � ��������
    uint8_t waitCooling;// 1 ���� ind=28        ����� �������� ������ ������ ����������
    uint8_t pkoff[2];   // 2 ���� ind=29;ind=30 ���������������� �����.
    uint8_t ikoff[2];   // 2 ���� ind=31;ind=32 ������������ �����.
    uint8_t identif;    // 1 ���� ind=33;       ������� ����� �������
    uint8_t ip[4];      // 4 ���� ind=34;ind=35;ind=36;ind=37; IP MQTT broker [192.168.100.100]
    uint8_t nothing0;   // 1 ���� ind=38;       �� ������������ !
    uint8_t nothing1;   // 1 ���� ind=39;       �� ������������ !
} sp;// ------------------ ����� 40 bytes -------------------------------

extern union Byte portFlag;
extern union Byte portOut;
/* ---��������� � �������� ������ -----*/
//extern struct byte {
//    unsigned a0: 1;
//    unsigned a1: 1;
//    unsigned a2: 1;
//    unsigned a3: 1;
//    unsigned a4: 1;
//    unsigned a5: 1;
//    unsigned a6: 1;
//    unsigned a7: 1;
//};
///* ---��������� ���������� � ������� ������ -----*/
//union Byte {
//    unsigned char value;
//    struct byte bitfield;
//}portOut;

//#define HEATER  portOut.bitfield.a0  // �����������
//#define HUMIDI	portOut.bitfield.a1  // �����������
//#define FLAP		portOut.bitfield.a2  // �������� �������������
//#define EXTRA		portOut.bitfield.a3  // ��������������� �����
//#define TURN		portOut.bitfield.a4  // ������� ������
//#define COOLER	portOut.bitfield.a5  // ���������� ����������

#endif /* __GLOBAL_H */
