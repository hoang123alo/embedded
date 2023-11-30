#include <main.h>
#include "..\Lib\Delay.h"
#include "..\Lib\Uart.h"

void main()
{
	char read[20];
	int i;
	uart_init();
	while(1)
	{
		
		if(UART_READY())
		{
			i = 0;
			do
			{
				read[i] = uart_read();
				Delay_ms(10);
			}while(UART_READY());
			
			uart_write_string(read);	
		}
		
		//Delay_ms(500);
//		if(P1_0 == 0)
//		{
//			Delay_ms(20);
//			uart_write(0x01);
//			while(P1_0 == 0);
//		}
//		if(P1_1 == 0)
//		{
//			Delay_ms(20);
//			uart_write(0x02);
//			while(P1_1 == 0);
//		}
	}
}