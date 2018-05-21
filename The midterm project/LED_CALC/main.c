/************************************************************************************
*               计算器（共阴极；从左到右代表最高到最低）
*               1   2   3   +
*               4   5   6   -
*               7   8   9   ×
*               退   0   =   ÷
************************************************************************************/
#include <reg51.h>      //此文件中定义了单片机的一些特殊功能寄存器
#include <intrins.h>    //延时所需要的头文件

#define KEY   P1        //矩阵键盘的引脚宏定义以键盘功能键的键号宏定义，方便修改
#define JIA   3
#define JIAN  7
#define CHENG 11
#define CHU   15
#define DYU   14

typedef unsigned char u8;
typedef unsigned int u16;

sbit SK = P3 ^ 6;       //移位时钟控制（11脚）
sbit RK = P3 ^ 5;       //并行输出控制（12脚）
sbit SR = P3 ^ 4;       //串行数据输入（14脚）
u8 res = 0xFF;          //存储第一个数字
u8 net = 0xFF;          //存储第二个数字
u8 status;              /*记录输入运算符的情况，默认是0，输入运算符后会置1
                          用来控制给res赋值还是net，为0给res赋值，为1给net赋值
                          在没有输入运算符时按下数字都是给res
                        */
u8 show_number = 11;    //要显示在点阵上的数字的变量，默认11，11是ledval中←的角标
u8 sign1 = 0xFF;            //记录输入的符号，在调用等与函数时会用到
u8 sign2 = 0xFF;

//二维数组存储0-9、#在点阵上显示的对应编码
u8 code ledval[][8] =
{
    {0X00,0X00,0X3E,0X41,0X41,0X41,0X3E,0X00},  //0
    {0X00,0X00,0X00,0X00,0X21,0X7F,0X01,0X00},  //1
    {0X00,0X00,0X27,0X45,0X45,0X45,0X39,0X00},  //2
    {0X00,0X00,0X22,0X49,0X49,0X49,0X36,0X00},  //3
    {0X00,0X00,0X0C,0X14,0X24,0X7F,0X04,0X00},  //4
    {0X00,0X00,0X72,0X51,0X51,0X51,0X4E,0X00},  //5
    {0X00,0X00,0X3E,0X49,0X49,0X49,0X26,0X00},  //6
    {0X00,0X00,0X40,0X40,0X40,0X4F,0X70,0X00},  //7
    {0X00,0X00,0X36,0X49,0X49,0X49,0X36,0X00},  //8
    {0X00,0X00,0X32,0X49,0X49,0X49,0X3E,0X00},  //9
    {0X00,0X24,0X7E,0X24,0X24,0X7E,0X24,0X00},  //#
    {0X10,0X38,0X54,0X92,0X10,0X10,0X10,0X10},  //←
};

//函数声明
void Addition();
void Subtraction();
void Multiplication();
void Division();

//延时函数（i=1时，大约延时10us）
void Delay(u16 i)
{
    while (i--);
}


//加函数
void Addition()
{
    if (sign1 == JIA)
    {
        status = 1;
        if (res != 0xFF && net != 0xFF) //只有在res和net都不为初值的情况也就是有按下值才能计算
        {
            res = res + net;        //累加
            if (res > 9)
                show_number = 10;   //计算得到的数值大于9时要显示#，10是#编码在ledval中的角标
            else
                show_number = res;  //计算完将结果赋值给显示变量

            net = 0xFF;
            sign1 = sign2; //要把第二个运算符赋给第一个存储变量
        }
    }
    else if (sign1 == JIAN)
        Subtraction();
    else if (sign1 == CHENG)
        Multiplication();
    else if (sign1 == CHU)
        Division();
}


//减函数
void Subtraction()
{
    if (sign1 == JIAN)
    {
        status = 1;
        if (res != 0xFF && net != 0xFF) //只有在res和net都不为初值的情况也就是有按下值才能计算
        {
            res = res - net;
            if (res < 10 && res >= 0)
                show_number = res;  //计算完将结果赋值给显示变量
            else
                show_number = 10;   //计算得到的数值磝小于0时要显示#，10是#编码在ledval中的角标

            net = 0xFF;
            sign1 = sign2;
        }
    }
    else if (sign1 == JIA)
        Addition();
    else if (sign1 == CHENG)
        Multiplication();
    else if (sign1 == CHU)
        Division();
}


//乘函数
void Multiplication()
{
    if (sign1 == CHENG)
    {
        status = 1;
        if (res != 0xFF && net != 0xFF) //只有在res和net都不为初值的情况也就是有按下值才能计算
        {
            res = res * net;
            if (res > 9 || res < 0)
                show_number = 10;
            else
                show_number = res;  //计算完将结果赋值给显示变量

            net = 0xFF;
            sign1 = sign2;
        }
    }
    else if (sign1 == JIA)
        Addition();
    else if (sign1 == JIAN)
        Subtraction();
    else if (sign1 == CHU)
        Division();
}


//除函数
void Division()
{
    if (sign1 == CHU)
    {
        status = 1;
        if (res != 0xFF && net != 0xFF) //只有在res和net都不为初值的情况也就是有按下值才能计算
        {
            res = res / net;
            show_number = res;      //计算完将结果赋值给显示变量

            net = 0xFF;
            sign1 = sign2;
        }
    }
    else if (sign1 == JIA)
        Addition();
    else if (sign1 == CHENG)
        Multiplication();
    else if (sign1 == JIAN)
        Subtraction();
}


//等于函数
void Equal(u8 tmp)
{
    switch (tmp)
    {
        case JIA: Addition(); break;
        case JIAN: Subtraction(); break;
        case CHENG: Multiplication(); break;
        case CHU: Division(); break;
    }
}


//矩阵键盘检测函数，该函数返回对应按键的键号，一共16个键，键号是0-15
u8 KeyScan()
{
    u8 a = 0;
    u8 keyval = 0xFF;
    KEY = 0x0f;
    if (KEY != 0x0f)        //读取按键是否按下
    {
        Delay(1000);        //延时10ms进行消抖
        if (KEY != 0x0f)    //再次检测键盘是否按下
        {
            switch (KEY)
            {
                case(0x07):keyval = 0; break;
                case(0x0b):keyval = 1; break;
                case(0x0d):keyval = 2; break;
                case(0x0e):keyval = 3; break;
            }
            KEY = 0xf0;     //测试行
            switch (KEY)
            {
                case(0x70):keyval = keyval; break;
                case(0xb0):keyval = keyval + 4; break;
                case(0xd0):keyval = keyval + 8; break;
                case(0xe0):keyval = keyval + 12; break;
            }
            while ((a<50) && (KEY != 0xf0))     //按键松手检测
            {
                Delay(1000);
                a++;
            }

            return keyval;
        }
    }
    return keyval;
}


//转换函数
void Transform()
{
    u8 keyval = 0xFF;       //存储矩阵键盘的键号
    keyval = KeyScan();

    if (keyval != 0xFF)     //不为默认的初始值时转换函数才可以运行
    {
        switch (keyval)
        {
            case 0:
            case 1:
            case 2:
                show_number = keyval + 1;
                if (status == 0)
                    res = keyval + 1;
                else if (status == 1)
                    net = keyval + 1;
                break;
            case 4:
            case 5:
            case 6:
                show_number = keyval;
                if (status == 0)
                    res = keyval;
                else if (status == 1)
                    net = keyval;
                break;
            case 8:
            case 9:
            case 10:
                show_number = keyval - 1;
                if (status == 0)
                    res = keyval - 1;
                else if (status == 1)
                    net = keyval - 1;
                break;
            case 13:
                show_number = 0;
                if (status == 0)
                    res = 0;
                else if (status == 1)
                    net = 0;
                break;
            case 12:                //删除
                res = 0xFF;
                net = 0xFF;
                status = 0;
                sign1 = 0xFF;
                show_number = 11;
                break;
            case JIA:
                if (sign1 == 0xFF)  //记录下符号，等于函数会用到
                    sign1 = JIA;
                else
                    sign2 = JIA;
                Addition();         //加法运算
                break;
            case CHU:
                if (sign1 == 0xFF)
                    sign1 = CHU;
                else
                    sign2 = CHU;
                Division();         //除法运算
                break;
            case JIAN:
                if (sign1 == 0xFF)
                    sign1 = JIAN;
                else
                    sign2 = JIAN;
                Subtraction();      //减法运算
                break;
            case CHENG:
                if (sign1 == 0xFF)
                    sign1 = CHENG;
                else
                    sign2 = CHENG;
                Multiplication();   //乘法运算
                break;
            case DYU:
                Equal(sign1);       //等于的情况
                break;
        }
    }
}


//74HC595芯片
void HC595SendByte(u8 dat)
{
    u8 i = 0;
    SK = 0;
    RK = 0;
    for (i = 0; i < 8; i++)
    {
        SR = dat >> 7;  //高位往低位拆，最高位变成串行的1位
        dat <<= 1;      //次高位变成最高位
        SK = 1;         //SK上升沿
        SK = 0;
    }
    RK = 1;             //RK上升沿
    RK = 0;
}


//LED点阵显示函数
void LedDisplay(u8 a)
{
    u8 i = 0;
    for (i = 0; i<8; i++)
    {
        P0 = _cror_(0x7f, i);           //跳到下一列
        HC595SendByte(ledval[a][i]);    //将该列的数值显示
        Delay(100);                     //延时
        HC595SendByte(0x00);            //消影
    }
}


//程序入口函数
void main()
{
    while (1)
    {
        Transform();
        LedDisplay(show_number);
    }
}
