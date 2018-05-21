/****************************************************************************
* 文 件 名: main.c
* 作    者: Yangson
* 修    订: 2018-05-11
* 版    本: 1.2
* 描    述: 计算器，显示在LED点阵上，只显示一位数据
* 编译环境: Keil uVision5
****************************************************************************/
#include "reg51.h"
#include "intrins.h"

#define LED         P0
#define KEY         P1
#define BYTE_SIZE   8
#define PLUS        3
#define MINUS       7
#define TIME        11
#define DIVIDED     15
#define EQUAL       14
#define DELETE      12


typedef unsigned char u8;
typedef unsigned int  u16;
sbit SRCLK = P3^6;
sbit RCLK  = P3^5;
sbit SER   = P3^4;
u8 hasOpr = 0;
u8 rowPos = 10;
u8 equation[4] = {255, 255, 255, 255};
u8 code led_row[][8] =
{
    {0x00,0x00,0x3e,0x41,0x41,0x41,0x3e,0x00},  //0
    {0x00,0x00,0x00,0x00,0x21,0x7f,0x01,0x00},  //1
    {0x00,0x00,0x27,0x45,0x45,0x45,0x39,0x00},  //2
    {0x00,0x00,0x22,0x49,0x49,0x49,0x36,0x00},  //3
    {0x00,0x00,0x0c,0x14,0x24,0x7f,0x04,0x00},  //4
    {0x00,0x00,0x72,0x51,0x51,0x51,0x4e,0x00},  //5
    {0x00,0x00,0x3e,0x49,0x49,0x49,0x26,0x00},  //6
    {0x00,0x00,0x40,0x40,0x40,0x4f,0x70,0x00},  //7
    {0x00,0x00,0x36,0x49,0x49,0x49,0x36,0x00},  //8
    {0x00,0x00,0x32,0x49,0x49,0x49,0x3e,0x00},  //9
    {0x30,0x78,0x7C,0x3E,0x3E,0x7C,0x78,0x30},  //心
    {0x24,0x24,0xFF,0x24,0x24,0xFF,0x24,0x24}   //#
};


/*******************************************************************************
* 名 称: Delay(u16 msec)
* 功 能: 延时函数，msec=1时，大约延时10us
* 入口参数: msec，以毫秒为延时
* 出口参数: 无
*******************************************************************************/
void Delay(u16 msec)
{
    while (msec--);
}


/*******************************************************************************
* 名 称: KeyScan()
* 功 能: 对矩阵键盘进行按键的检测，函数返回按键序号
* 入口参数: 无
* 出口参数: key_number
*******************************************************************************/
u8 KeyScan()
{
    u8 keyNumber = 0xFF;    //矩阵键盘编号0-15
    u8 col = 0xFF;
    u8 cnt = 0;

    KEY = 0x0F;
    if (KEY != 0x0F)
    {
        Delay(1000);
        if (KEY != 0x0F)
        {
            switch (KEY)
            {
                case 0x07: col = 0; break;
                case 0x0B: col = 1; break;
                case 0x0D: col = 2; break;
                case 0x0E: col = 3; break;
            }
            KEY = 0xF0;
            switch (KEY)
            {
                case 0x70: keyNumber = col; break;
                case 0xB0: keyNumber = col + 4; break;
                case 0xD0: keyNumber = col + 8; break;
                case 0xE0: keyNumber = col + 12; break;
            }
            while ((cnt < 50) && (KEY != 0xF0))
            {
                Delay(1000);
                cnt++;
            }
            return keyNumber;
        }
    }
    return keyNumber;
}


/*******************************************************************************
* 名 称: HC595SendByte()
* 功 能: 向74HC595发送一个字节的数据
* 入口参数: tmp，要发送的一个字节
* 出口参数: 无
*******************************************************************************/
void HC595SendByte(u8 tmp)
{
    u8 i;
    SRCLK = 0;
    RCLK  = 0;
    for (i = 0; i < BYTE_SIZE; i++)
    {
        SER = tmp >> 7;
        tmp <<= 1;
        SRCLK = 1;
        SRCLK = 0;
    }
    RCLK = 1;
    RCLK = 0;
}


/*******************************************************************************
* 名 称: LedDisplay(u8 num)
* 功 能: 在LED点阵上显示内容
* 入口参数: num，按下的数字
* 出口参数: 无
*******************************************************************************/
void LedDisplay(u8 num)
{
    u8 i;
    LED = 0xFE;
    for (i = 0; i < BYTE_SIZE; i++)
    {
        LED = _cror_(LED, 1);
        HC595SendByte(led_row[num][i]);
        Delay(100);
        HC595SendByte(0x00);
    }
}


/*******************************************************************************
* 名 称: Calculate()
* 功 能: 计算函数
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void Calculate()
{
    if (equation[3]==PLUS||equation[3]==MINUS||equation[3]==TIME||equation[3]==DIVIDED||equation[3]==EQUAL)
    {
        if (equation[2] != 255)
        {
            switch (equation[1])
            {
                case PLUS:    equation[0] = equation[0] + equation[2];break;
                case MINUS:   equation[0] = equation[0] - equation[2];break;
                case TIME:    equation[0] = equation[0] * equation[2];break;
                case DIVIDED: equation[0] = equation[0] / equation[2];break;
            }
            if (equation[3] == EQUAL)
                equation[1] = 255;  //当第二次输入的运算符时等号时，不做运算符的移动
            else
                equation[1] = equation[3];  //  将下一轮的运算符移到前面
            if (equation[0] < 10 && equation[0] >= 0)
                rowPos = equation[0];
            else
            {
                rowPos = 11;    //溢出，显示“#”
                hasOpr = 0;
                equation[1] = equation[3] = 255; //初始为默认值
            }
            equation[2] = 255;
        }
    }
}


/*******************************************************************************
* 名 称: KeyDown()
* 功 能: 按下按键进行相应的操作
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void KeyDown()
{
    u8 keyValue = 0xFF;                     //矩阵键盘编号的对应键值
    keyValue = KeyScan();
    if (keyValue != 0xFF)
    {
        switch(keyValue)
        {
        case 0:
        case 1:
        case 2:
            keyValue++;
            rowPos = keyValue;
            if (hasOpr)
                equation[2] = keyValue;
            else
                equation[0] = keyValue;
            break;
        case 4:
        case 5:
        case 6:
            rowPos = keyValue;
            if (hasOpr)
                equation[2] = keyValue;
            else
                equation[0] = keyValue;
            break;
        case 8:
        case 9:
        case 10:
            keyValue--;
            rowPos = keyValue;
            if (hasOpr)
                equation[2] = keyValue;
            else
                equation[0] = keyValue;
            break;
        case 13:
            rowPos = 0;
            if (hasOpr)
                equation[2] = keyValue;
            else
                equation[0] = keyValue;
            break;
        case DELETE:    //退格键
            if (equation[2] != 255 && equation[3] == 255)//1+2时删除
            {
                equation[2] = 255;
                rowPos = equation[0];
            }
            else if (hasOpr && equation[2] != 255 && equation[3] != 255)//1+2=3时删除
            {
                rowPos = 10;
                equation[1] = 255;
                equation[3] = 255;
                hasOpr = 0;
            }
            else//1+时删除
            {
                rowPos = 10;
                equation[0] = 255;
                equation[1] = 255;
                hasOpr = 0;
            }
            break;
        case PLUS:      //加法
        case MINUS:     //减法
        case TIME:      //乘法
        case DIVIDED:   //除法
        case EQUAL:     //等于
            hasOpr = 1;
            if (equation[1] == 255)
                equation[1] = keyValue;
            else
                equation[3] = keyValue;
            Calculate();
            break;
        }
    }
}


/*******************************************************************************
* 程序入口函数
*******************************************************************************/
void main()
{
    while(1)
    {
        KeyDown();
        LedDisplay(rowPos);
    }
}
