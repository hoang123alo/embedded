#include <main.h>
#include "F:\KeilC\AT89C52\Lib\Delay.h"

sbit SPI_SCLK 	= P2^0;
sbit SPI_MOSI 	= P2^1;
sbit SPI_MISO 	= P2^2;
sbit SPI_CS 	= P2^3;

void Soft_SPI_Init()
{
	SPI_CS = 1;
	SPI_SCLK = 0;
	SPI_MOSI = 1;
}
void Delay_100us()
{
	unsigned int y;
		for(y=0;y<12;y++);
	
}
char Soft_SPI_Write(unsigned char x)
{
	unsigned char i;
	unsigned char rev = 0;
	SPI_CS = 0;
	Delay_100us();
	for(i = 0; i<8;i++)
	{
		if((x&0x80)>>7 != 0)
			SPI_MOSI = 1;
		else 
			SPI_MOSI = 0;


		SPI_SCLK = 1;
		rev <<= 1;
		if(SPI_MISO)
			rev |= 0x01;
		SPI_SCLK = 0;
		x <<= 1;
	}
	SPI_CS = 1;
	return rev;
}
void main()
{
	char num;
	unsigned char dem = 0;
	Soft_SPI_Init();
	while(1)
	{
	
		num = Soft_SPI_Write(dem);
		P1 = num;
		dem++;
		Delay_ms(10);
	}
}