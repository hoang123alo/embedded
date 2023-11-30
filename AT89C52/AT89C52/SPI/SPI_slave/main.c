#include <main.h>
#include "F:\KeilC\AT89C52\Lib\Delay.h"

sbit SPI_SCLK 	= P2^0;
sbit SPI_MOSI 	= P2^1;
sbit SPI_MISO 	= P2^2;
sbit SPI_CS 	= P2^3;

void Soft_SPI_Init()
{
	SPI_MISO = 1;
}
char Soft_SPI_Read(unsigned char x)
{
	unsigned char i;
	unsigned char rev = 0;

	for(i = 0; i<8;i++)
	{
		if((x&0x80)>>7 != 0)
			SPI_MISO = 1;
		else 
			SPI_MISO = 0;
		
		while(SPI_SCLK == 0);
		rev <<= 1;
		if(SPI_MOSI)
			rev |= 0x01;
		x <<= 1;
		//while(SPI_SCLK == 1);
	}
	while(SPI_CS == 0);
	return rev;

}
void main()
{
	char num = 0;
	//char dem = 0;
	Soft_SPI_Init();
	while(1)
	{  
		if(SPI_CS == 0)
		num = Soft_SPI_Read(P1);

		if(num != 0)
		{
			P1 = num;
		}
	}
}