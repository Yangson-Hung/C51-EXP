#include <reg51.h>
#include <intrins.h>

#define LED P2

typedef unsigned int u16;
typedef unsigned char u8;


void delay100ms(void)
{
    unsigned char a,b,c;
    for(c=19;c>0;c--)
        for(b=20;b>0;b--)
            for(a=130;a>0;a--);
}


void flash(u16 val)
{
    u8 i;
    for(i=0;i<10;i++)
    {
        delay100ms();
        LED=0xFF;
        delay100ms();
        LED=val;
    }
    LED=val;
}


void delay(u16 i)
{
   while(i--);
}


void t1()
{
    LED=0x7E;
    delay(50000);
    LED=0xBD;
    delay(50000);
    LED=0xDB;
    delay(50000);
    LED=0xE7;
    delay(50000);
    LED=0xFF;
    delay(50000);
}


void t2()
{
    LED=0xFF;
    delay(50000);
    LED=0xE7;
    delay(50000);
    LED=0xDB;
    delay(50000);
    LED=0xBD;
    delay(50000);
    LED=0x7E;
    delay(50000);
}


void main()
{
    u8 i;
    while(1)
    {
        t1();
        t2();
        LED=0xFE;
        for(i=0;i<7;i++)
        {
            flash(LED);
            LED=_crol_(LED,1);
        }
        for(i=0;i<7;i++)
        {
            flash(LED);
            LED=_cror_(LED,1);
        }
        flash(LED);
    }
}
