#include <pic14/pic12f675.h>
 
typedef unsigned int word;
word __at 0x2007 __CONFIG =(_MCLRE_OFF & _WDT_OFF & _INTRC_OSC_NOCLKOUT);//desactivar WDT no deseado y MCLRE no deseado al usar pin

void delay (unsigned inttiempo);
 
void main(void)
{
	ANSEL = 0; //Se coloca el ANSEL en cero para digitalizar los pines.
    TRISIO = 0b00100000; //Poner todos los pines como salidas menos el 5, el cual va a ser el lector del botón
	GPIO = 0b00000000; //Poner pines en bajo
	
	int contador1 = 0; //inicializamos un contador en 0
    unsigned int time = 300; //Variable que define el delay

//Loop forever
	delay(time/2); //delay para que le de tiempo de cargar al capacitor
	while ( 1 )
	{
		if (contador1 < 29) //se limita el contador para que solo cuente del 0 al 29 para un total de 30 números.
		{
			contador1 = contador1 + 1; 
		}
		else
		{
			contador1 = 0; //si el contador llega a 29, se reinicia a 0.
		}

		if (GP5==0) //este if detecta si el botón conectado en el pin5 es presionado.
		{
			while (GP5==0) //se usa un while como método de seguridad por si el botón se mantiene presionado un largo tiempo.
			{	
				int contador = contador1; //se fija el valor del contador de afuera una vez  
										  //se entra en el while para que no cambie el numero

				//numero 1
				if (0<=contador & contador<5) //si el valor del contador es 0, 1, 2, 3 o 4: se enciende solamente un led
				{
					GPIO = 0b00000001;
					delay(time);
					GPIO = 0b00000000; //se apaga el led luego del delay
				}

				//numero 2
				if (5<=contador & contador<10) //si el valor del contador es 5, 6, 7, 8 o 9: se encienden dos leds
				{
					GPIO = 0b00000010;
					delay(time);
					GPIO = 0b00000000; //se apaga el led luego del delay
				}

				//numero 3
				if (10<=contador & contador<15) //si el valor del contador es 10, 11, 12, 13 o 14: se encienden tres leds
				{
					GPIO = 0b00000011;
					delay(time);
					GPIO = 0b00000000; //se apaga el led luego del delay
				}

				//numero 4
				if (15<=contador & contador<20) //si el valor del contador es 15, 16, 17, 18 o 19: se encienden cuatro leds
				{
					GPIO = 0b00000110; 
					delay(time);
					GPIO = 0b00000000; //se apaga el led luego del delay
				}

				//numero 5
				if (20<=contador & contador<25) //si el valor del contador es 20, 21, 22, 23 o 24: se encienden cinco leds
				{
					GPIO = 0b00000111;
					delay(time);
					GPIO = 0b00000000; //se apaga el led luego del delay
				}

				//numero 6
				if (25<=contador & contador<30) //si el valor del contador es 25, 26, 27, 28 o 29: se encienden seis leds
				{
					GPIO = 0b00010111;
					delay(time);
					GPIO = 0b00000000; //se apaga el led luego del delay
				}
			}			
		}
	}
}

void delay(unsigned int tiempo)
{
	unsigned int i;
	unsigned int j;

	for(i=0;i<tiempo;i++)
	  for(j=0;j<1275;j++);
}
