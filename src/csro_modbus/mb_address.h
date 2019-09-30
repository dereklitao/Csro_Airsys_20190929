#ifndef __MB_ADDRESS_H
#define __MB_ADDRESS_H

/*system register for hmi to read/write*/
//Disc address
#define D_USER_AC_ERR_START 0 // total 39: 0->38

//Coil address
#define C_USER_RLY1 0
#define C_USER_RLY2 1
#define C_USER_RLY3 2
#define C_USER_RLY4 3
#define C_USER_RLY5 4
#define C_USER_RLY6 5
#define C_USER_RLY7 6
#define C_USER_RLY8 7
#define C_USER_RLY9 8
#define C_USER_RLY10 9
#define C_USER_RLY11 10
#define C_USER_RLY12 11
#define C_USER_AIRSYS_POWER 12

//input address
#define I_USER_AC_STATUS_START 0  // total 20: 0->20
#define I_USER_AP_STATUS_START 30 // total 30: 30->59

//Holding address
#define H_USER_AIRSYS_MODE 0 //0->dehumi/1->cold/2->hot
#define H_USER_AIRSYS_TEMP 1 //0.1 degree
#define H_USER_AIRSYS_FAN 2  //0->low/1->mid/2->high/3->super

typedef struct
{
    uint8_t ac_power;
    int16_t ac_mode;
    int16_t ac_temp;
} csro_ac_control;

typedef struct
{
    int16_t ap_power;
    int16_t ap_mode;
    int16_t ap_fan;
    int16_t ap_temp;
} csro_ap_control;

#endif
