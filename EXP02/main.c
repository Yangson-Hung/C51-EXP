#include "reg52.h"

#define LED         P0
#define KEYONE      1
#define KEYTWO      2
#define KEYDOWN     0
#define MAXIMUM     9
#define MINIMUM     0
#define BEEPNUMBER  8
typedef unsigned int u16;
typedef unsigned char u8;

sbit LSA  = P2^2;
sbit LSB  = P2^3;
sbit LSC  = P2^4;
sbit key1 = P3^1;
sbit key2 = P3^0;
sbit beep = P1^5;
u8 smgIndex = 0;
u8 keyValue = 0;
u8 smgduan[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};


//延时
void Delay(u16 t) {
    while(t--);
}

//蜂鸣器函数
void Beep() {
    beep = ~beep;
    Delay(10);
}

//检测按键函数，k1返回1，k2返回2
u8 KeyPress() {
    u8 keyFlag = 0;
    if (key1 == KEYDOWN) {
        Delay(1000);
        if (key1 == 0)
            keyFlag = 1; // 检测到按键1
        while (!key1); //松手检测
    } else if (key2 == KEYDOWN) {
        Delay(1000);
        if (key2 == 0)
            keyFlag = 2; // 检测到按键2
        while(!key2);
    }
    return keyFlag;
}

//按下k1按键数码管数字自增，按下k2数字自减
void KeyLight() {
    if (keyValue == KEYONE) { // 按键1执行加
        if (smgIndex < MAXIMUM) {
            smgIndex++;
            LED = smgduan[smgIndex];
            if (smgIndex == BEEPNUMBER) {
                while (1) {
                    Beep();
                    keyValue = KeyPress();
                    if (keyValue == KEYTWO) {
                        if (smgIndex > MINIMUM) {
                            smgIndex--;
                            LED = smgduan[smgIndex];
                        }
                        break;
                    }
                }
            }
        }
    } else if (keyValue == KEYTWO) { // 按键2执行减
        if (smgIndex > MINIMUM) {
            smgIndex--;
            LED = smgduan[smgIndex];
        }
    }
}

//数码管选择
void SelectSmg() {
    LSA = 0;
    LSB = 0;
    LSC = 0;
}


void main() {
    SelectSmg();
    LED = smgduan[smgIndex];
    while (1) {
        keyValue = KeyPress();
        KeyLight();
    }
}
