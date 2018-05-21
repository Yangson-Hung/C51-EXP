/****************************************************************************
* 文 件 名: main.c
* 作    者: Yangson
* 修    订: 2018-05-11
* 版    本: 2.0
* 描    述: 使用矩阵键盘来控制LED点阵上一个三个点长方形进行向下向左向右移动，
            并且按下变换按钮，三个点的长方形从横向变纵向，或者从纵向变横向
* 编译环境: Keil uVision5
****************************************************************************/
#include <reg51.h>
#include <intrins.h>

#define LED         P0
#define KEY         P1
#define BYTE_SIZE   8
#define LEFT        4
#define RIGHT       6
#define DOWN        9
#define CHANGE      7


typedef unsigned char u8;
typedef unsigned int  u16;
sbit SRCLK = P3^6;
sbit RCLK  = P3^5;
sbit SER   = P3^4;
u8 led_row[] = {0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00};
u8 shift_cnt;
u8 max_cnt;


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
* 名 称: RecodeLedRow(u8 cnt, u8 index, u8 cod)
* 功 能: 对LedRow进行重新编码
* 入口参数: cnt:纵向还是横向; index: 矩形在数组中的下标; cod: 转换编码
* 出口参数: 无
*******************************************************************************/
void RecodeLedRow(u8 cnt, u8 index, u8 cod)
{
    if (cnt == 1)
    {
        if (index == 0)
        {
            led_row[0] = cod;
            led_row[1] = cod;
            led_row[7] = cod;
        }
        else if (index == 7)
        {
            led_row[0] = cod;
            led_row[6] = cod;
            led_row[7] = cod;
        }
        else
        {
            led_row[index]   = cod;
            led_row[index-1] = cod;
            led_row[index+1] = cod;
        }
    }
    else if (cnt == 3)
    {
        if ((led_row[0]==led_row[6]) && (led_row[0]!=0))
        {
            led_row[0] = 0;
            led_row[6] = 0;
            led_row[7] = cod;
        }
        else if ((led_row[1]==led_row[7]) && (led_row[0]!=0))
        {
            led_row[0] = cod;
            led_row[1] = 0;
            led_row[7] = 0;
        }
        else
        {
            led_row[index-2] = 0;
            led_row[index-1] = cod;
            led_row[index]   = 0;
        }
    }
}


/*******************************************************************************
* 名 称: SwitchDir()
* 功 能: 变换LED矩形的方向
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void SwitchDir()
{
    u8 i;
    u8 index = 0;
    u8 cnt = 0;
    for (i = 0; i < BYTE_SIZE; i++)
    {
        if (led_row[i] != 0)
        {
            cnt++;
            index = i;
        }
    }
    switch (led_row[index])
    {
        //纵向转横向的编码转换
        case 0xE0: RecodeLedRow(cnt, index, 0x40); break;
        case 0x70: RecodeLedRow(cnt, index, 0x20); break;
        case 0x38: RecodeLedRow(cnt, index, 0x10); break;
        case 0x1C: RecodeLedRow(cnt, index, 0x08); break;
        case 0x0E: RecodeLedRow(cnt, index, 0x04); break;
        case 0x07: RecodeLedRow(cnt, index, 0x02); break;
        //横向转纵向的编码转换
        case 0x40: RecodeLedRow(cnt, index, 0xE0); break;
        case 0x20: RecodeLedRow(cnt, index, 0x70); break;
        case 0x10: RecodeLedRow(cnt, index, 0x38); break;
        case 0x08: RecodeLedRow(cnt, index, 0x1C); break;
        case 0x04: RecodeLedRow(cnt, index, 0x0E); break;
        case 0x02: RecodeLedRow(cnt, index, 0x07); break;
    }
}


/*******************************************************************************
* 名 称: LeftShift()
* 功 能: LED矩形向左移动
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
* 功 能: LED矩形向右移动
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
* 名 称: DownShift()
* 功 能: LED矩形向下移动
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void DownShift()
{
    u8 i;
    u8 index = 0;
    u8 cnt = 0;
    shift_cnt++;
    for (i = 0; i < BYTE_SIZE; i++)
    {
        if (led_row[i] != 0)
        {
            cnt++;
            index = i;
        }
    }
    if (cnt == 1)
        led_row[index] = led_row[index] >> 1;
    else if (cnt == 3)
    {
        if ((led_row[0]==led_row[6]) && (led_row[0]!=0))
        {
            led_row[0] >>= 1;
            led_row[6] >>= 1;
            led_row[7] >>= 1;
        }
        else if ((led_row[1]==led_row[7]) && (led_row[0]!=0))
        {
            led_row[0] >>= 1;
            led_row[1] >>= 1;
            led_row[7] >>= 1;
        }
        else
        {
            led_row[index] >>= 1;
            led_row[index-1] >>= 1;
            led_row[index-2] >>= 1;
        }
    }
}


/*******************************************************************************
* 名 称: Stop()
* 功 能: 落到最下方停止操作
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void Stop()
{
    u8 i;
    u8 index = 0;
    u8 cnt = 0;
    for (i = 0; i < BYTE_SIZE; i++)
    {
        if (led_row[i] != 0)
        {
            cnt++;
            index = i;
        }
    }

    if (cnt == 1)
        max_cnt = 5;
    else
        max_cnt = 6;

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
        Stop();
        if (shift_cnt < max_cnt)
        {
            switch (num)
            {
                case LEFT: LeftShift(); break;
                case RIGHT: RightShift(); break;
                case DOWN: DownShift(); break;
                case CHANGE: SwitchDir(); break;
            }
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
