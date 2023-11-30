#include <main.h>
#include "..\Lib\Uart.h"

#if (FREQ_OSC != 11059200ul)
	#error "Thu vien khong ho tro cho tan so thach anh khac 11059200 !!!"
#endif
void uart_init()
{	// khoi tao uart mode 1, toc so baurd 9600
	SM0 = 0; SM1 = 1; 	//UARRT MODE1
	TMOD &= 0x0F; 		// chon timer 1 hoat dong che do 8bit
	TMOD |= 0x20;
	#if(BAURD == 9600)
	TH1 = 0XFD;  		//TOC DO BAURD =9600
	#elif(BAURD == 2400)
	TH1 = 0XF4;  		//TOC DO BAURD =2400
	#elif(BAURD == 1200)
	TH1 = 0XE8;  		//TOC DO BAURD =1200
	#elif(BAURD == 19200)
	TH1 = 0XFD;  		//TOC DO BAURD =19200
	PCON |= 0x80;
	#else 
		#error "Khong ho tro toc do baurd chi co the la: 19200, 9600, 2400, 1200"
	#endif
	TR1 = 1; 			//TIMER BAT DAU CHAY
	TI = 1;	 			// SAN SANG GUI DU LIEU
	REN = 1;			// CHO PHEP NHAN
}
void uart_write(char c)
{
	while(TI == 0);//TI = 0 laf chua ghi xong
	TI = 0;
	SBUF = c;	
}

void uart_write_string(char *str)
{
	unsigned char i =0;
	while(str[i] != 0)
	{
		 uart_write(str[i]);
		 i++;
	}

}

char UART_READY()
{
	return RI;
}
// RI = 1 la dang co du lieu
char uart_read()
{
	RI = 0;
	return SBUF;
}
