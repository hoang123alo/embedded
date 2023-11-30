#include <main.h>
#include "..\Lib\Delay.h"
#include "..\Lib\Uart.h"

void main()
{

	
	uart_init();
	while(1)
	{
//	 	if(UART_READY())
//		{
//			if(uart_read() == 0x01)
//			{
//				P1_0 = 1;
//			}
//			else
//			{
//
//			}
//		}
		if(P3_2 == 0)
		{
			if(UART_READY())
			{
			 	P1 = uart_read();
				Delay_ms(10000);
				P1 = uart_read();	
			}
		}

		

	}
}