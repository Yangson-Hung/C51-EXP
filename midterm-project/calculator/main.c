/****************************************************************************
* 文 件 名: main.c
* 作    者: Yangson
* 修    订: 2018-05-11
* 版    本: 3.1
* 描    述: 计算器，显示在数码管上，可以显示八位数据
* 编译环境: Keil uVision5
****************************************************************************/
#include "reg52.h"
#include "intrins.h"

#define KEY  P1


typedef unsigned char u8;
typedef unsigned int  u16;
typedef unsigned long u32;
sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;
enum keyNum {plus=3, minus=7, time=11, divided=15, equal=14, delete=12};            //对功能键枚举
u8 ledCode[]= {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x40};   //数码管led编码
long next;                  //下一个操作数
long result;                //运算结果
long seg[8] = {0};          //存储result被剪切后的数字
u8 each[8] = {0};           //存储result的每一位数
u8 keyVal[8] = {0};         //存储按键值
u8 withMinus[8] = {0};      //存储带有负号的值
u8 clf = -1;                //控制清空的标识
u8 opr1 = 0xFF;             //存储输入的第一个运算符
u8 opr2 = 0xFF;             //存储接下来输入的运算符
u8 gcnt;                    //result的位数
u8 index;                   //keyVal数组的下标标识
u8 hasNext;                 //表示有下一个数输入
u8 hasOpr;                  //表示有输入运算符的标识
u8 itrsw = 0xFF;            //退出异常开关
u8 minf1 = 0;               //负数标识1
u8 minf2 = 0;               //负数标识2


/*******************************************************************************
* 名 称: Delay100ms()
* 功 能: 延时函数，延时100ms
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void Delay100ms()
{
    u8 a,b,c;
    for(c=19; c>0; c--)
        for(b=20; b>0; b--)
            for(a=130; a>0; a--);
}


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
* 名 称: CutResult()
* 功 能: 运算得到的结果result进行“剪切”为每一个单个数，然后保存在each数组中
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void CutResult()
{
    u8 i;
    u8 hun = 1;         //要取位要除的基数
    u8 cnt = 0;         //数字有几位
    u8 base = 10;       //除10的基数
    while (result / base != 0)
    {
        cnt++;
        base *= 10;
    }
    cnt = cnt + 1;              //最终的位数

    for (i = 0; i < cnt-1; i++)
    {
        hun *= 10;
    }

    for (i = 0; i < cnt; i++)
    {
        seg[i] = result / hun;
        hun /= 10;
    }

    each[0] = seg[0];   //最高位的赋值
    for (i = 1; i < cnt; i++)
    {
        each[i] = seg[i] - seg[i-1] * 10;
    }
    gcnt = cnt;     //赋给全局变量，方便使用
}


/*******************************************************************************
* 名 称: KeyScan()
* 功 能: 对矩阵键盘进行按键的检测，函数返回按键序号
* 入口参数: 无
* 出口参数: keyNumber
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
* 名 称: Clear()
* 功 能: 对数组进行清空
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void Clear()
{
    if (hasOpr == 1 && clf == 1)
    {
        int i;
        for (i = 0; i < index; i++)
        {
            keyVal[i] = 0;
        }
        index = 0;  //重新置0
        clf = 0;    //设为0后，第二次按下数字或者保存数字由于条件不成立所以不会清空
        // 1 + 23 在输入2时clf为0
    }
}


/*******************************************************************************
* 名 称: SaveKeyValue(u8 keyValue)
* 功 能: 将键值保存到数组中
* 入口参数: keyValue
* 出口参数: 无
*******************************************************************************/
void SaveKeyValue(u8 keyValue)
{
    Clear();
    if (index < 8)
    {
        //第一次按0，第二次也按0，还是显示0
        if (keyVal[0] == 0 && index == 1 && keyValue == 0)
        {
            index--;
        }
        else if (keyVal[0] == 0 && index == 1) //第一次按0，第二次按非0时显示非0数字
        {
            index--;
        }
        keyVal[index] = keyValue;
        index++;
        if (index == 1 && clf != 0)
            clf = index;
    }
}


/*******************************************************************************
* 名 称: SaveResult()
* 功 能: 将结果保存到数组中
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void SaveResult()
{
    u8 i;
    CutResult();//对result进行分割，存储
    clf = 1;
    for (i = 0; i < gcnt; i++)
    {
        SaveKeyValue(each[i]);//存储分割的每个位
    }
    clf = 1;    //得到运算结果后进行下一轮的运算时输入数字提供清空的条件
}


/*******************************************************************************
* 名 称: ZeroException()
* 功 能: 对除0进行异常处理
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void ZeroException()
{
    u8 ledCodeError[5] = {0xB1, 0XBF, 0xB1, 0xB1,0xF9};
    u8 i,temp = 0;
    while (1)
    {
        for(i = 0; i < 5; i++)
        {
            LSA = i % 2;
            temp = i / 2;
            LSB = temp % 2;
            temp = temp / 2;
            LSC = temp % 2;

            P0 = ledCodeError[i];
            Delay(100);
            P0 = 0x00;
        }
        Delay100ms();
        itrsw = KeyScan();
        if(itrsw == delete)
            break;
    }
    SaveResult();
    hasOpr = 0; //置为0表示发生除0异常后，应回到输入第一个数时的状态
}


/*******************************************************************************
* 名 称: SetMinusFlag()
* 功 能: 记录负数标识
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void SetMinusFlag()
{
    if (result < 0)
    {
        result = -result;  //负数变正数
        minf2 = minf1 = -1;
    }
    else if (result >= 0)
    {
        minf2 = minf1 = 1;
    }
}


/*******************************************************************************
* 名 称: GetMinus()
* 功 能: 获得负数标识
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void GetMinus()
{
    if (minf1 == -1)
    {
        result = -result; //正数变负数
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
    switch(opr1)
    {
    case plus:                  //加法
        GetMinus();             //判断负数标识是否为-1，是用负数计算
        result = result + next;
        SetMinusFlag();         //设置负数标识
        SaveResult();
        break;
    case minus:                 //减法
        GetMinus();
        result = result - next;
        SetMinusFlag();
        SaveResult();
        break;
    case time:                  //乘法
        GetMinus();
        result = result * next;
        SetMinusFlag();
        SaveResult();
        break;
    case divided:               //除法
        if (next != 0)
        {
            GetMinus();
            result = result / next;
            SetMinusFlag();
            SaveResult();
        }
        else if (next == 0)
        {
            ZeroException();
        }
        break;
    }
}


/*******************************************************************************
* 名 称: IsNotEqual()
* 功 能: 不是等于号触发得到结果的情况
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void IsNotEqual()
{
    if (hasNext)
    {
        Calculate();
        opr1 = opr2;        //计算完后要做运算符的移位
        /*
        *   发生除0异常如果是由按运算符触发的（此时是opr2），退出异常后触发运算符会赋给opr1
        *   退出异常后用显示的数字做计算输入的运算符是赋给opr2的，而在计算判断运算符时是用opr1做判断的
        *   opr1就是之前的触发运算符opr2
        */
        if (next == 0)
            opr1 = 0xFF;
        hasNext = 0;
    }
}


/*******************************************************************************
* 名 称: IsEqual()
* 功 能: 等于号触发得到结果的情况
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void IsEqual()
{
    if (hasNext)
    {
        Calculate();
        hasNext = 0;
    }
    opr1 = 0xFF;    //在第一次进行正常运算后，要对opr1恢复初值;当按运算符后按等于要对运算符清空
}


/*******************************************************************************
* 名 称: ReadValue()
* 功 能: 从数组中读取值
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void ReadValue()
{
    u32 sum = 0;
    u8 i,j,ten;
    u8 tenCnt = index-1;
    for(i=0; i<index; i++)
    {
        ten = 1;
        for(j=0; j<tenCnt; j++)
        {
            ten = ten * 10;
        }
        sum = sum + keyVal[i] * ten;    //sum是从keyVal保存数字的数组中取出来的值
        tenCnt--;
    }
    if (hasOpr == 1)
    {
        next = sum;
        hasNext = 1;
    }
    else
        result = sum;
}


/*******************************************************************************
* 名 称: AddMinus()
* 功 能: 产生负数时，将负号添加至数组中
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void AddMinus()
{
    u8 i;
    withMinus[0] = 10;                  //ledCode的负号在数组中的下标值
    for (i = 0; i < index; i++)
    {
        withMinus[i+1] = keyVal[i];
    }
}


/*******************************************************************************
* 名 称: Delete()
* 功 能: 退格键做删除操作
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void Delete()
{
    u8 i;
    if(index >= 1)
    {
        if (hasOpr == 1 && hasNext == 0 && clf == 1) //对得到的运算结果全部清除
        {
            for (i = 0; i < index; i++)
            {
                keyVal[i] = 0;
            }
            //重新设置为初始值
            index = 0;
            clf = -1;           //控制没有输入数字的情况下输入运算符不进行任何操作
            hasOpr = 0;         //控制清空后输入的第一个数赋值给result
            result = 0;         //result初始为0
            //设为默认初值，当用运算符触发得到结果在删除结果，下轮计算会用触发运算符计算
            opr1 = 0xFF;
            if (minf1 == -1)    //对显示结果是负数的处理，将值初始化为默认值，控制负号不显示
            {
                minf1 = 0;
                minf2 = 0;
            }
        }
        else                        //单个位的删除
        {
            keyVal[index-1] = 0;    //删掉最后一位数
            index--;                //数组元素个数减1
            ReadValue();
        }
    }
}


/*******************************************************************************
* 名 称: NewCalculate()
* 功 能: 按下等于号触发得到结果可输入新的数字进行新一轮计算
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void NewCalculate()
{
    u8 i;
    if(index >= 1)
    {
        //只有在按等于号得到运算结果时可以做新一轮的运算
        if (hasOpr==1 && hasNext==0 && clf==1 && opr1==0xFF)
        {
            for (i = 0; i < index; i++)
            {
                keyVal[i] = 0;
            }
            //重新设置为初始值
            index = 0;
            clf = -1;           //控制没有输入数字的情况下输入运算符不进行任何操作
            hasOpr = 0;         //新一轮的运算输入的第一个数赋值给result
            result = 0;         //result初始为0
            //设为默认初值，当用运算符触发得到结果在删除结果，下轮计算会用触发运算符计算
            opr1 = 0xFF;
            if (minf1 == -1)    //对显示结果是负数的处理，将值初始化为默认值，控制负号不显示
            {
                minf1 = 0;
                minf2 = 0;
            }
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
            NewCalculate();                 //判断是否进行新一轮的运算
            SaveKeyValue(keyValue);         //保存按键的数字到数组中
            ReadValue();                    //读取数字到变量中
            /*
            *   在得到一个负数的结果后数码管会显示负号
            *   输入新的数字后，负号还会存在，将minf2设置为1后，控制不显示负号
            */
            if (minf1 == -1)
                minf2 = 1;
            break;
        case 4:
        case 5:
        case 6:
            NewCalculate();
            SaveKeyValue(keyValue);
            ReadValue();
            if (minf1 == -1)
                minf2 = 1;
            break;
        case 8:
        case 9:
        case 10:
            NewCalculate();
            keyValue--;
            SaveKeyValue(keyValue);
            ReadValue();
            if (minf1 == -1)
                minf2 = 1;
            break;
        case 13:
            NewCalculate();
            SaveKeyValue(0);
            ReadValue();
            if (minf1 == -1)
                minf2 = 1;
            break;
        case delete:    //退格键
            Delete();   //调用删除的函数
            break;
        case plus:      //加法
        case minus:     //减法
        case time:      //乘法
        case divided:   //除法
            if (clf != -1)              //没有输入数字的情况即clf=-1时，按运算符不进行任何操作
            {
                hasOpr = 1;             //当输入运算符后，hasOpr置1
                if (opr1 == 0xFF)
                    opr1 = keyValue;
                else
                    opr2 = keyValue;
                IsNotEqual();
            }
            break;
        case equal:     //等于
            IsEqual();
            break;
        }
    }
}


/*******************************************************************************
* 名 称: Display()
* 功 能: 数码管的显示
* 入口参数: 无
* 出口参数: 无
*******************************************************************************/
void Display()
{
    u8 i;
    u8 temp = 0;
    u8 minusIndex;
    if (minf1 == -1 && minf2 == -1)
    {
        AddMinus();
        minusIndex = index + 1;
        for(i = 0; i < minusIndex; i++)
        {
            //数码管的位选操作
            LSA = i % 2;
            temp = i / 2;
            LSB = temp % 2;
            temp = temp / 2;
            LSC = temp % 2;

            P0 = ledCode[withMinus[minusIndex-(i+1)]];
            Delay(100); //延时1ms
            P0 = 0x00;  //消影
        }
    }
    else
    {
        for(i = 0; i < index; i++)
        {
            //数码管的位选操作
            LSA = i % 2;
            temp = i / 2;
            LSB = temp % 2;
            temp = temp / 2;
            LSC = temp % 2;

            P0 = ledCode[keyVal[index-(i+1)]];
            Delay(100); //延时1ms
            P0 = 0x00;  //消影
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
        Display();
    }
}
