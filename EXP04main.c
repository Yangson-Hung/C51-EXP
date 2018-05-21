#include <reg52.h>

typedef unsigned char u8;
typedef unsigned int u16;

sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;
sbit K3  = P3^2;
sbit K4  = P3^3;
u8 code smgduan[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,
                    0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,0x40};
u16 msec;       //毫秒
u8  sec;        //秒
u8  min;        //分钟
u8  hour;       //小时
u8  clock[8];   //时钟数组


void Delay(u16 t)
{
    while (t--);
}


void DigDisplay()
{
    u8 i;
    u8 tmp = 0;
    for(i = 0; i < 8; i++)
    {
        LSA = i % 2;
        tmp = i / 2;
        LSB = tmp % 2;
        tmp = tmp / 2;
        LSC = tmp % 2;

        P0 = smgduan[clock[i]];
        Delay(1);
        P0 = 0x00;
    }
}


//定时器1初始化
void Timer1Init()
{
    TMOD |= 0x10;   //选择为定时器0模式，工作方式1，仅用TR0打开启动
    TH1 = 0xFC;     //定时器赋初值，定时1ms
    TL1 = 0x18;
    TR1 = 1;        //打开定时器
}


//外部中断0初始化
void Int0Init()
{
    IT0 = 1;
    EX0 = 1;
    EA  = 1;
}


//外部中断1初始化
void Int1Init()
{
    IT1 = 1;//跳变沿触发方式（下降沿）
    EX1 = 1;//打开INT1的中断允许。
    EA  = 1;//打开总中断
}


//时钟函数
void DigClock()
{
    if (TF1==1)
    {
        TF1 = 0;
        TH1 = 0XFC;         //给定时器赋初值，定时1ms
        TL1 = 0X18;
        msec++;
    }
    if (msec > 999)         //到达1s时间
    {
        msec = 0;
        sec++;
        if (sec > 59)       //计时到60秒后重新开始
        {
            sec = 0;
            min++;
            if(min > 59)
            {
                min = 0;
                if(hour < 24)
                    hour++;
                else
                {
                    hour = 0;
                    min = 0;
                    sec = 0;
                    msec = 0;
                }
            }
        }
    }
    clock[0] = sec % 10;       //时钟0位
    clock[1] = sec / 10;       //时钟1位
    clock[3] = min % 10;       //时钟3位
    clock[4] = min / 10;       //时钟4位
    clock[6] = hour % 10;      //时钟6位
    clock[7] = hour / 10;      //时钟7位
    clock[2] = 16;
    clock[5] = 16;
}


//程序入口函数
void main()
{
    Int0Init();
    Int1Init();
    Timer1Init();

    while (1)
    {
        DigClock();
        DigDisplay();
    }
}


//外部中断0中断函数
void Int0() interrupt 0
{
    Delay(100);
    if (K3 == 0)
    {
        min++;
        if (min > 59)
        {
            min = 0;
            if (hour < 24)
                hour++;
            else
                hour = 0;
        }
    }
}


//外部中断1中断函数
void Int1() interrupt 2
{
    Delay(100);
    if (K4 == 0)
    {
        if (hour < 24)
            hour++;
        else
            hour = 0;
    }
}
