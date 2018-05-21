#include "reg52.h"
#define KEY P1

typedef unsigned int u16;
typedef unsigned char u8;

sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;

u8 smgduan[]= {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
               0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
u8 flag = 0;
u8 keyValueArr[8] = {0};

void Delay (u16 i) {
   while (i--);
}

u8 RowColmScanKey()
{
    u8 keyValue = 0xff;
    u8 colm = 0xff;    //列
    u8 a;

    KEY = 0x0f;
    if (KEY != 0x0f) {
        //消抖
        Delay(1000);
        if (KEY != 0x0f) {
            KEY = 0x0f;
            switch (KEY) {
            case 0x07:
                colm = 0;
                break;
            case 0x0b:
                colm = 1;
                break;
            case 0x0d:
                colm = 2;
                break;
            case 0x0e:
                colm = 3;
                break;
            }
            KEY = 0xf0;
            switch (KEY) {
            case 0x70:
                keyValue = colm;
                break;
            case 0xb0:
                keyValue = colm + 4;
                break;
            case 0xd0:
                keyValue = colm + 8;
                break;
            case 0xe0:
                keyValue = colm + 12;
                break;
            }
            a = 0;
            while ((a < 50) && (KEY != 0xf0)) {
                Delay(1000);
                a++;
            }

            return keyValue;

        }
    }

    return keyValue;
}

void DigDisplay()
{
    u8 i;
    u8 tmp = 0;
    for(i = 0; i < flag; i++)
    {
        // 数码管的位选操作
        LSA = i % 2;
        tmp = i / 2;
        LSB = tmp % 2;
        tmp = tmp / 2;
        LSC = tmp % 2;

        P0 = smgduan[keyValueArr[flag - (i + 1)]];
        Delay(100); // 延时1ms
        P0 = 0x00;
    }
}

void SaveKeyValueToArray(keyValue)
{
    if (flag < 8)
    {
        if (flag == 1 && keyValueArr[0] == 0 && keyValue == 0) // 第一次按0，第二次也按0，还是显示0
        {
            flag--;
        }
        else if (flag == 1 && keyValueArr[0] == 0) // 第一次按0，第二次按非0时显示非0数字
        {
            flag--;
        }

        keyValueArr[flag] = keyValue;
        flag++;
    }
}

void DefineKey()
{
    u8 keyValue = 0xFF;
    keyValue = RowColmScanKey();

    if (keyValue != 0xFF)
    {
        switch(keyValue)
        {
            case 0:
            case 1:
            case 2:
                keyValue++;
                SaveKeyValueToArray(keyValue);
                break;
            case 3:
                if(flag >= 1)
                {
                    flag--;
                }
                break;
            case 4:
            case 5:
            case 6:
                SaveKeyValueToArray(keyValue);
                break;
            case 8:
            case 9:
            case 10:
                keyValue--;
                SaveKeyValueToArray(keyValue);
                break;
            case 13:
                SaveKeyValueToArray(0);
                break;
        }
    }
}


void main()
{
    while(1)
    {
        DefineKey(); // 按键定义
        DigDisplay(); // 数码管显示
    }
}
