#include <reg52.h>

#define KEY P1

typedef unsigned int u16;
typedef unsigned char u8;

sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;


u8 code smgduan[] =
{
 0x3f,0x06,0x5b,0x4f,
 0x66,0x6d,0x7d,0x07,
 0x7f,0x6f,0x77,0x7c,
 0x39,0x5e,0x79,0x71
};
u8 num_char[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
u8 num_int[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};


void Delay(u16 t)
{
    while (t--);
}


void UsartInit()
{
    SCON=0X50;  //串口工作方式1
    TMOD=0X20;  //计数器工作方式2
    PCON=0X80;  //波特率加倍
    TH1=0XF3;   //计数器初始值设置,波特率是4800
    TL1=0XF3;
    ES=1;       //打开接收总中断
    EA=1;       //打开总中断
    TR1=1;      //打开计数器
}


u8 KeyScan()
{
    u8 keyNum = 0xFF;
    u8 cnt = 0;
    KEY = 0x0F;             //高四位(行)为低电平,低四位(列)为高电平

    if (KEY != 0x0F)        //条件成立时,有按键按下
    {
        Delay(1000);        //延时消抖
        if (KEY != 0x0F)    //消抖后再次判断按键按下
        {
            switch (KEY)    //列扫描
            {
                case 0x07: keyNum = 0; break;
                case 0x0B: keyNum = 1; break;
                case 0x0D: keyNum = 2; break;
                case 0x0E: keyNum = 3; break;
            }

            KEY = 0xF0;     //高四位(行)为高电平,低四位(列)为低电平
            switch (KEY)    //行扫描
            {
                case 0x70: keyNum = keyNum; break;
                case 0xB0: keyNum += 4; break;
                case 0xD0: keyNum += 8; break;
                case 0xE0: keyNum += 12; break;
            }
            //延时
            while ((cnt < 50) && (KEY != 0xF0))
            {
                Delay(1000);
                cnt++;
            }
            return keyNum;
        }
    }
    return keyNum;
}


void KeyDown()
{
    u8 i;
    u8 keyNum = KeyScan();              //行列扫描返回一个键号,0-15

    if (keyNum != 0xFF)                 //有按键按下的情况运行
    {
        P0 = 0;                         //按下按键只在PC显示,清掉数码管之前的显示
        for (i = 0; i < 16; i++)
            if(keyNum == num_int[i])    //判断键号是什么数字
            {
                SBUF = num_char[i];     //根据键号将对应的字符存到SBUF,发送数据
                while(!TI);             //等待发送数据完成
                TI = 0;
            }
    }
}


void main()
{
    LSA = 0;
    LSB = 0;
    LSC = 0;
    P0 = 0;
    UsartInit();  //串口初始化
    while (1)
        KeyDown();
}


void Usart() interrupt 4
{
    u8 i;
    u8 recDat = SBUF;                   //接收数据,PC发过来的数据存到SBUF里,单片机从SBUF里取数据
    RI = 0;                             //清除接收中断标志位
    for(i = 0; i < 16; i++)
        if (recDat == num_char[i])      //判断接收的是什么字符
            P0 = smgduan[num_int[i]];   //接收什么字符就显示字符对应的数字
}
