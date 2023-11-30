#include"..\Lib\Delay.h"
#include"main.h"

void main()
{
	unsigned char low,high;
	unsigned int num;
	TMOD &= 0xF0;	// XOA THANH GHI TREN TIMER 0
	TMOD |= 0x05;	// CHON CHE DO COUNTER, 16 BYTE
	TR0 = 1; 	// bat timer 0
	while(1)
	{
		num = TH0 << 8 | TL0;
		P1 = num;
	}
}

