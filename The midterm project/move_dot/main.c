/****************************************************************************
* 文 件 名: main.c
* 作    者: Yangson
* 修    订: 2018-05-11
* 版    本: 1.0
* 描    述: 使用矩阵键盘来控制LED点阵上一个点进行上下左右移动（没有蜂鸣器）
* 编译环境: Keil uVision5
****************************************************************************/
#include <reg51.h>
#include <intrins.h>

#define LED         P0
#define KEY         P1
#define UP          1
#define LEFT        4
#define RIGHT       6
#define DOWN        9
#define BYTE_SIZE   8


typedef unsigned char u8;
typedef unsigned int  u16;
sbit SRCLK = P3^6;
sbit RCLK  = P3^5;
sbit SER   = P3^4;
u8 led_row[] = {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00};


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
    u8 key_number = 0xFF;
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
                case 0x70: key_number = col; break;
                case 0xB0: key_number = col + 4; break;
                case 0xD0: key_number = col + 8; break;
                case 0xE0: key_number = col + 12; break;
            }
            while ((cnt < 50) && (KEY != 0xF0))
            {
                Delay(1000);
                cnt++;
            }
            return key_number;
        }
    }
    return key_number;
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
    RCLK = 0;
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
* 名 称: UpShift()
* 功 能: LED一个点向上移动
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void UpShift()
{
    u8 i;
    for (i = 0; i < BYTE_SIZE; i++)
    {
        if (led_row[i] != 0)
            led_row[i] = _crol_(led_row[i], 1);
    }
}


/*******************************************************************************
* 名 称: DownShift()
* 功 能: LED一个点向下移动
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void DownShift()
{
    u8 i;
    for (i = 0; i < BYTE_SIZE; i++)
    {
        if (led_row[i] != 0)
            led_row[i] = _cror_(led_row[i], 1);
    }
}


/*******************************************************************************
* 名 称: LeftShift()
* 功 能: LED一个点向左移动
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void LeftShift()
{
    u8 i, tmp;
    tmp = led_row[0];
    for (i = 0; i < BYTE_SIZE-1; i++)
    {
        led_row[i] = led_row[i+1];
    }
    led_row[i] = tmp;
}


/*******************************************************************************
* 名 称: RightShift()
* 功 能: LED一个点向右移动
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void RightShift()
{
    u8 i, tmp;
    tmp = led_row[BYTE_SIZE-1];
    for (i = BYTE_SIZE-1; i > 0; i--)
    {
        led_row[i] = led_row[i-1];
    }
    led_row[i] = tmp;
}


/*******************************************************************************
* 名 称: KeyDown()
* 功 能: 按下按键进行相应的操作
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void KeyDown()
{
    u8 num = 0xFF;
    num = KeyScan();
    if (num != 0xFF)
    {
        switch (num)
        {
            case UP: UpShift(); break;
            case DOWN: DownShift(); break;
            case LEFT: LeftShift(); break;
            case RIGHT: RightShift(); break;
        }
    }
}


/*******************************************************************************
* 名 称: LedDisplay()
* 功 能: 在LED点阵上显示内容
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void LedDisplay()
{
    u8 i;
    LED = 0xFE;
    for (i = 0; i < BYTE_SIZE; i++)
    {
        LED = _cror_(LED, 1);
        HC595SendByte(led_row[i]);
        Delay(100);
        HC595SendByte(0x00);
    }
}


/*******************************************************************************
* 程序入口函数
*******************************************************************************/
void main()
{
    while (1)
    {
        KeyDown();
        LedDisplay();
    }
}
