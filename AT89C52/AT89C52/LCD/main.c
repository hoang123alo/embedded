
#include "main.h"
#include "..\Lib\Delay.h"

sbit LCD_RS = P2^0;
sbit LCD_EN = P2^1;

#define LCD_DATA  P3
// gui lenh
void Lcd_Cmd(unsigned char cmd)
{
	LCD_RS = 0;
	LCD_DATA = cmd;
	LCD_EN = 0;
	LCD_EN = 1;
	if(cmd <= 0x02)
	{
		Delay_ms(2);
	}
	else
	{
		Delay_ms(1);
	}
}
// gui ky tu
void Lcd_Chr_Cp(char c)
{
	LCD_RS = 1;
	LCD_DATA = c;
	LCD_EN = 0;
	LCD_EN = 1;
	Delay_ms(1);
}
void Lcd_Chr(unsigned char row,unsigned char col, char c)
{
	unsigned char cmd;
	cmd = (row == 1?0x80:0xC0) + col - 1;
	Lcd_Cmd(cmd);
	Lcd_Chr_Cp(c);
}

// gui chuoi
void Lcd_Out_Cp(char *str)
{
	unsigned char i = 0;
	while(str[i] != 0)
	{
		Lcd_Chr_Cp(str[i]);
		i++;
	}
}
void Lcd_Str(unsigned char row,unsigned char col, char *str)
{
	unsigned char cmd;
	cmd = (row == 1?0x80:0xC0) + col - 1;
	Lcd_Cmd(cmd);
	Lcd_Out_Cp(str);
}
void Lcd_Init()
{
	Lcd_Cmd(0x30);
	Delay_ms(5);
	Lcd_Cmd(0x30);
	Delay_ms(1);
	Lcd_Cmd(0x30);

	Lcd_Cmd(0x38); // so dong hien thi la 2

	Lcd_Cmd(0x01);	 // xoa noi dung hien thi
	Lcd_Cmd(0x0C);	   // bat hien thi va tat con tro
}
void main()
{
	int i;
	Lcd_Init();

	Lcd_Str(2,1,"toi yeu dat nuoc cua toi");
	for(i=0;i<8;i++)
	{
		Delay_ms(500);
		Lcd_Cmd(0x18);	   // lenh ddich trai
		//Lcd_Cmd(0x1C);	   // lenh dich Phai
		Lcd_Cmd(0x02);	   // lenh tro ve vi tri ban dau
	}
	

	while(1)
	{
		
	}
}
