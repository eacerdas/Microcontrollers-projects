#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

//definicion de estados
#define esperando_input 0 //estado en el que espera a que se presione un botón para iniciar el juego 
//-> next state siempre es inicio_ronda

#define inicio_ronda 1 //estado en el que se muestra el patrón que el jugador debe replicar 
//-> next state siempre es leer inputs

#define leer_inputs 2 //lee cuales botones fueron presionados y a partir de esto cuenta cuantos botones fueron presionados
//-> next state siempre es comparacion

#define comparacion 3 //se comparan los inputs con los valores generados
//->next state puede ser ronda_ganada o ronda_perdida

#define ronda_ganada 4 //estado en el que se da feedback de que la ronda se ganó
//->next state siempre es inicio_ronda

#define  ronda_perdida 5 //estado en el que se da feedback de que la ronda se perdió
//->next state siempre ronda perdida, se encicla parpadeando 3 veces todos los leds hasta que se apague el circuito


//declaración de variables que se necesitan luego
int flag0 = 0; int flag1 = 0; int flag2 = 0; int flag3 = 0; 
int flancobajo0 = 0; int flancobajo8 = 0; //Evitan que se generen problemas por detectar tanto el flanco alto como el bajo al pulsar el botón
int pulsacion = 0; //cuenta las veces que se pulsan los botones
int generar_arreglo = 0; //bandera que se levanta cuando el arreglo se deba llenar con valores random
int contador = 2; int count = 0; //contadores para generar numeros random
int round_win = 0; //bandera que se levanta si ganamos la ronda
int ronda = 4; // esta variable dice en qué ronda vamos, empieza en 4 porque queremos que la primera ronda se enciendan 4 leds random de una vez
int arreglo_generado[50] = {}; int arreglo_introducido[50] = {};
int cuenta = 0;

/*ISR (TIMER0_OVF_vect) //interrupcion por timer0
{
  if (cuenta==100) //este número se modifica para obtener el tiempo que uno desea en el delay
  {
    //código que se quiere ejecutar
    cuenta=0; //se vuelve a empezar la cuenta desde 0
  }
  else  cuenta++; //se suma 1 hasta llegar a 100
}*/

ISR(PCINT0_vect) //detecta interrupcion B0 - PCINT0
{
  if (flancobajo0 == 0) //bandera para que solo se active la bandera si es el flanco positivo
  {
    flancobajo0 = flancobajo0 + 1;
  }
  else if (flancobajo0 == 1)
  {
    flag2 = 1; //Bandera arriba cuando se activa la interrupcion
    flancobajo0 = 0;
  }
}
  
ISR(PCINT1_vect) //detecta interrupcion A0 - PCINT8
{
  if (flancobajo8 == 0){ //bandera para que solo se active la bandera si es el flanco positivo
    flancobajo8 = flancobajo8 + 1;
  }

  else if (flancobajo8 == 1){
    
    flag3 = 1; //Bandera arriba cuando se activa la interrupcion
    flancobajo8 = 0;
  }
}

ISR (INT0_vect){ //detecta interrupcion int0
flag0 = 1; //Bandera arriba cuando se activa la interrupcion
}

ISR (INT1_vect){ //detecta interrupcion int1
flag1 = 1; //Bandera arriba cuando se activa la interrupcion
}

void parpadeo_inicial(){ //las luces parpadean 2 veces
  int DELAY = 5000;
  PORTB = 0b00000000; 
  _delay_ms(DELAY);
  PORTB = 0b00111100; 
  _delay_ms(DELAY);
  PORTB = 0b00000000; 
  _delay_ms(DELAY);
  PORTB = 0b00111100;
  _delay_ms(DELAY);
  PORTB = 0b00000000;
  _delay_ms(7000);
} 

void parpadeo_final(){ //las luces parpadean 3 veces indicando que se pierde
  int DELAY = 5000;
  PORTB = 0b00000000; 
  _delay_ms(DELAY);
  PORTB = 0b00111100; 
  _delay_ms(DELAY);
  PORTB = 0b00000000; 
  _delay_ms(DELAY);
  PORTB = 0b00111100;
  _delay_ms(DELAY);
  PORTB = 0b00000000;
  _delay_ms(DELAY);
   PORTB = 0b00111100;
  _delay_ms(DELAY);
  PORTB = 0b00000000;
  _delay_ms(DELAY);
} 

void flash_b2(){ //parpadea una vez el led del pin B2
  int DELAY = 6000;
  PORTB = 0b00000100;
  _delay_ms(DELAY);
  PORTB = 0b00000000;
  _delay_ms(DELAY);
}

void flash_b3(){ //parpadea una vez el led del pin B3
  int DELAY = 6000;
  PORTB = 0b00001000;
  _delay_ms(DELAY);
  PORTB = 0b00000000;
  _delay_ms(DELAY);
}

void flash_b4(){ //parpadea una vez el led del pin B4
  int DELAY = 6000;
  _delay_ms(DELAY);
  PORTB = 0b00010000;
  _delay_ms(DELAY);
  PORTB = 0b00000000;
  _delay_ms(DELAY);
}

void flash_b5(){ //parpadea una vez el led del pin B5
  int DELAY = 6000;
  PORTB = 0b00100000;
  _delay_ms(DELAY);
  PORTB = 0b00000000;
  _delay_ms(DELAY);
}

void encender_leds(int *array_random){ //enciende la secuencia random de leds a partir de un arreglo generado aleatoriamente
  int DELAY = 2000;
  for (int i = 0; i < ronda; i++) {
    int random = array_random[i];
  
    if (random == 0){
      _delay_ms(DELAY); 
      flash_b2(); 
    }
        
    if (random == 1){
      _delay_ms(DELAY);
      flash_b3();
    }

    if (random == 2){
      _delay_ms(DELAY);
      flash_b4();
    }

    if (random == 3){
      _delay_ms(DELAY);
      flash_b5();
    }
  }
}

void configuracion_timers() //función de configuración inicial de los timers
{
 TCCR0A=0x00; //modo normal, contar desde 0 hasta 255
 TCCR0B=0x00; 
 TCCR0B |= (1<<CS00)|(1<<CS02); //se utiliza reescalado 1024 para dividir la frecuencia
 TCNT0=0; //aquí se guarda el conteo del timers, es de 8 bits entonces llega hasta 255
 TIMSK|=(1<<TOIE0); //se habilitan las interrupciones por timer0
}

void delay_timer()
{
 unsigned int i=0;
 while(i<=62)
  { 
   while((TIFR & (1 << TOV0) )==0); //contando de 0 a 255 y a la bandera a que se levante
   TIFR|=(1<<TOV0); //se baja la bandera
   i++; 
  }
}

//############################################################################################################
int main(void)
{
    //configuracion_timers(); //acá se configuran los timers pero al final no pudieron ser implementados

    _delay_ms(5000);

    //delay_timer(); //tiempo para que se carguen los capacitores al inicio, utilizando delay del timer del micro

    sei(); // Interrupción global

    DDRB = 0b00111100; //Configuracion del puertos B2, B3, B4 y B5 como salidas para los 4 leds

    GIMSK |= (1<<INT0)|(1<<INT1); // habilitando el INT0 e INT1 (interrupciones externas) (D2 y D3)

    GIMSK |= (1<<PCIE0)|(1<<PCIE1); // habilitando el PCINT0 y PCINT8 (B0 y A0)

    PCMSK |= 0b00000001; // enable PCINT0 pin B0

    PCMSK1 |= 0b00000001; // enable PCINT8 pin A0

    //parpadeo_inicial(); // parpadea 2 veces antes de iniciar el juego
    _delay_ms(1000); // pequeña pausa antes de iniciar a mostrar las 4 primeras luces del juego


    char state = esperando_input; //inicializamos la variable que guarda el estado
    char next_state = esperando_input; //inicializamos la variable que guarda el siguiente estado

    while (1) { // loop infinito
    
        if (contador < 3){
        contador = contador + 1;
        }
        else{
        contador = 0;
        }

        state = next_state;
        switch (state){

            //###################################################################################
            case (esperando_input): //espera a que se presione un botón para iniciar el juego

                //se aumenta 2 en el contador cada vez que se entra a este estado
                if (count < 3){
                    count = count + 2;
                }
                else{
                    count = 0;
                }
                    
                //condición para saber si se toca algún botón para iniciar el juego
                if ((flag0 == 1) | (flag1 == 1) | (flag2 == 1) |(flag3 == 1)){
                    parpadeo_inicial();
                    flag0 = 0; flag1 = 0; flag2 = 0; flag3 = 0;
                    next_state = inicio_ronda;
                    generar_arreglo = 1;
                    }
                else{
                    next_state = esperando_input;
                }
                        
                //llenamos las primeras 4 posiciones del vector generado con valores random del contador utilizado en este estado
                if (generar_arreglo == 1)
                {
                        arreglo_generado[0] = count; // numero entre 0 y 3.
                        arreglo_generado[1] = contador; // numero entre 0 y 3.
                        if (contador == 1)
                        {
                            arreglo_generado[2] = 3;
                            arreglo_generado[3] = 0; 
                        }
                        else if (contador == 2)
                        {
                            arreglo_generado[2] = 1;
                            arreglo_generado[3] = 3; 
                        }
                        else if (contador == 3)
                        {
                            arreglo_generado[2] = 0;
                            arreglo_generado[3] = 2; 
                        }
                        else if (contador == 0)
                        {
                            arreglo_generado[2] = 3;
                            arreglo_generado[3] = 2; 
                        }
                        else if (contador > 3)
                        {
                            contador = 0;
                        }
                }
                break;

            //###################################################################################

            case (inicio_ronda):
                encender_leds(arreglo_generado);
                next_state = leer_inputs;
                break;
            
            //###################################################################################
            
            case (leer_inputs):

                if (pulsacion == ronda) {
                    next_state = comparacion;
                    pulsacion = 0; //se resetea el contador de pulsaciones para la siguiente ronda
                }
                else{
                    next_state = leer_inputs;

                    if (flag0 == 1){
                        flag0 = 0;
                        flash_b2(); //feedback de que se presiono el boton
                        arreglo_introducido[pulsacion] = 0;
                        pulsacion = pulsacion + 1;
                    }

                    else if (flag1 == 1){
                        flag1 = 0;
                        flash_b3(); //feedback de que se presiono el boton
                        arreglo_introducido[pulsacion] = 1;
                        pulsacion = pulsacion + 1;
                    }

                    else if (flag2 == 1){
                        flag2 = 0; 
                        flash_b4(); //feedback de que se presiono el boton
                        arreglo_introducido[pulsacion] = 2;
                        pulsacion = pulsacion + 1;
                    }

                    else if (flag3 == 1){
                        flag3 = 0;
                        flash_b5(); //feedback de que se presiono el boton
                        arreglo_introducido[pulsacion] = 3;
                        pulsacion = pulsacion + 1;
                    }
                }
                break;

            //###################################################################################

            case (comparacion):

                //funcion que compara 2 arrays para ver si son iguales.
                round_win = 1; //iniciamos con la suposicion de que son iguales, entonces lo hicimos bien y ganamos la ronda

                int x = 0;

                while (x < ronda) {
                    // Obtener elementos de ambos arreglos en misma posición o índice
                    int valorGenerado = arreglo_generado[x], valorIntroducido = arreglo_introducido[x];
                    // Comparar
                    if (valorGenerado != valorIntroducido) {
                        round_win = 0;
                    }
                    x = x + 1;
                }

                if (round_win == 1){
                    next_state = ronda_ganada;
                    round_win = 0; //se resetea la bandera de que ganamos la ronda para la siguiente ronda
                }
                else if (round_win == 0){
                    next_state = ronda_perdida;
                }
                break;

            //###################################################################################

            case (ronda_ganada):
                
                arreglo_generado[ronda] = contador; //se agrega un número random a el array de números generados
                ronda = ronda +1; //los leds a encender son ahora uno más que la ronda anterior

                //enciende las luces durante un tiempo, indicando que ganamos
                _delay_ms(5000);
                PORTB = 0b00111100; 
                _delay_ms(10000);
                PORTB = 0b00000000;
                _delay_ms(10000);

                next_state = inicio_ronda; //comienza el juego de nuevo
                break;

            //###################################################################################

            case (ronda_perdida):
                parpadeo_final(); //parpadea 3 veces indicando que se pierde
                
                _delay_ms(15000);

                next_state = ronda_perdida; // ciclo infinito en el que parpadean 3 veces las luces indicando que perdimos

                break;
        }
    }
}
//###########################################################################################