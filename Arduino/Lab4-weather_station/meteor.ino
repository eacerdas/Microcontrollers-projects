#include <Adafruit_PCD8544.h>
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdio.h>
#include <EEPROM.h>
#include <Servo.h>
#include "LowPower.h"
//#include "ThingsBoard.h"

#define THINGSBOARD_SERVER "demo.thingsboard.io"

//definicion de pines
const int LEDazul=2;
const int LEDrojo=53;
const int comm=12;
const int powerScreen = 22;
const int sensorLluvia = 11;
const int xInputPin = 6;
const int yInputPin = 7;

// definimos LCD
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 5, 6, 4, 8);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

// Address de EEPROM
int addr = 0;

//definicion de parametros necesarios para las ecuaciones de S-H para estimar la temperatura a partir del termistor
float R1 = 100000; //resistencia de 100k en serie con el termistor.
float logR2, R2, TEMPERATURA;
float c1 = 0.8586139205e-03, c2 = 2.059709585e-04, c3 = 0.8130635267e-07; //coeficientes de steinhart-hart, obtenidos de: https://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm

//TEMP
int valorTermistor; 
int tempDisplay;
#define TOKEN_TEMP "a9Ggq3euSFkiqrHOaraF"


//HUMEDAD
int valorHumedad; 
float humedadNormalizada; 
int humedadDisplay;

//VIENTO
int valorViento; 
float vientoNormalizado; 
int vientoDisplay;
float vientoVolts;
float vientoVelocidad;

//BATERIA
int valorBateria; 
float bateriaNormalizada;
int bateriaDisplay;
float bateriaVolts;

//LLUVIA
int valorLluvia; 
float lluviaNormalizada;
int lluviaDisplay;

//LUZ
int valorLuz; 
float luzNormalizada;
int luzDisplay;

const long A = 500;     //Resistencia en oscuridad en KΩ
const int B = 12.5;        //Resistencia a la luz (10 Lux) en KΩ
const int Rc = 10;       //Resistencia calibracion en KΩ
const int LDRPin = A1;   //Pin del LDR
int V;
int ilum;



// Variables para contar tiempo
unsigned long startTime;
unsigned long startTimeGeneral;
unsigned long currentTime;
unsigned long currentTimeGeneral;
const unsigned long tenMin = 600000; //10 minutos en ms
unsigned int timerStartEnable = 1;

// Configuracion de servomotores
Servo xAxis;
Servo yAxis;
int xInput;
int yInput;

int contador = 0;
int contador2 = 0;
int parpadeos = 0;
int flag = 0;

void hart(){ //uiliza una ecuacion para estimar la temperatura de acuerdo a la resistencia del termistor
  R2 = R1 * (1023.0 / (float)valorTermistor - 1.0);
  logR2 = log(R2);
  TEMPERATURA = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); //ecuacion de S-H, da temperatura en kelvin
  TEMPERATURA = TEMPERATURA -273.15; //convertimos a grados centigrados la temperatura
}

void blink(){ //parpadea el led integrado
  //led parpadea una vez cada vez que inicia el loop
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);                       
  digitalWrite(LED_BUILTIN, LOW);    
  delay(300);
}

void battery_low(){ //se comprueba si la bateria esta en un nivel que se considera como "bateria baja" y se activa una alerta y el modo de bajo consumo
  if (bateriaDisplay <= 25){ //11.94V es el 25% de la bateria segun el fabricante

  LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF,
                TIMER3_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF,
                USART3_OFF, USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF); //entra en modo bajo consumo

    if (flag < 3){
      flag++;
    }
    else{
      flag = 0;
      digitalWrite(LEDrojo, !digitalRead(LEDrojo));
    }
  }
  else{
    digitalWrite(LEDrojo, LOW);
  }
}

void display_refresh(){ // Se muestan los datos en la pantalla si el switch de powerScreen esta cerrado
  if(digitalRead(powerScreen) == LOW){
      display.setCursor(0,0);
      display.print("BATERIA: ");
      display.print(bateriaDisplay);
      display.println("%");
      display.print("TEMP:    ");
      display.print(tempDisplay);
      display.println("C");
      display.print("HUMEDAD: ");
      display.print(humedadDisplay);
      display.println("%");
      display.print("VIENTO:  ");
      display.print(vientoDisplay);
      display.println("m/s");
      display.print("LLUVIA:  ");
      if(lluviaDisplay == 1){
        display.println("Si");
      }else{
        display.println("No");
      }
      display.print("LUZ:  ");
      display.print(ilum);
      display.println("LUX"); 
  }
  display.display();
  display.clearDisplay();
}

void serial_refresh(){ //se refresca el serial si el el switch de comm esta cerrado
  if(digitalRead(comm) == LOW){
    if(timerStartEnable == 1){
      startTime = millis();
      timerStartEnable = 0;
      }
    currentTime = millis();
    if(currentTime - startTime >= tenMin){
      Serial.print(bateriaDisplay);
      Serial.print(",");
      Serial.print(tempDisplay);
      Serial.print(",");
      Serial.print(humedadDisplay);
      Serial.print(",");
      Serial.print(vientoDisplay);
      Serial.print(",");
      if(lluviaDisplay == 1){
        Serial.print("Si");
      }else{
        Serial.print("No");
      }
      Serial.print(",");
      Serial.println(ilum);
      timerStartEnable = 1;
    }
  }
}

void rainCheck(){
  if(digitalRead(sensorLluvia) == LOW){
    lluviaDisplay = 1;
  }else{
    lluviaDisplay = 0;
  }
}

void memoryVerify(){
  if (addr == EEPROM.length()) {
      addr = 0;
      for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
    }
}

void panelAdjust(){
  xInput = analogRead(xInputPin);
  yInput = analogRead(yInputPin);

  xInput = map(xInput, 0, 1023, 0, 180);
  yInput = map(yInput, 0, 1023, 0, 180);

  xAxis.write(round(xInput));
  yAxis.write(round(yInput));
}

void memoryWrite(){
  currentTimeGeneral = millis();
  if(currentTimeGeneral - startTimeGeneral >= tenMin/2){
    
    memoryVerify();
    EEPROM.write(addr, tempDisplay);
    ++addr;
    
    memoryVerify();
    EEPROM.write(addr, humedadDisplay);
    ++addr;

    memoryVerify();
    EEPROM.write(addr, luzDisplay);
    ++addr;

    memoryVerify();
    EEPROM.write(addr, vientoDisplay);
    ++addr;

    memoryVerify();
    EEPROM.write(addr, lluviaDisplay);
    ++addr;
    startTimeGeneral = currentTimeGeneral;
  }
}

void setup() { // se realiza el setup
  Serial.begin(9600);
  Serial.println("PCD test");
  display.begin();
  display.setContrast(75);
  display.display(); // show splashscreen
  delay(500);
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);

  //configuracion de pines como salida
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LEDazul, OUTPUT);
  pinMode(LEDrojo, OUTPUT);
  pinMode(comm, INPUT);
  pinMode(powerScreen, INPUT);
  pinMode(sensorLluvia, INPUT);

  // Configuracion de servomotores
  xAxis.attach(8, 1000, 2000);
  yAxis.attach(9, 1000, 2000);

  //Timer de inicio de ejcuccion
  startTimeGeneral = millis();

  // Ponemos memoria EEPROM en ceros
  for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
}

void loop() { // loop infinito

  if (digitalRead(comm) == HIGH){
    digitalWrite(LEDazul, LOW);
    parpadeos = 0;
    contador = 0;
    contador2 = 0;
  }

  //El led del pin 2 parpadea 5 veces y se apaga por un periodo antes de volver a parpadear 5 veces, con esto evitamos usar delays y que se retrase en recorrer el loop 
  if (digitalRead(comm) == LOW){
    if (parpadeos < 10){
      if (contador < 10){
        contador = contador + 1;
      }
      else{
        digitalWrite(LEDazul, !digitalRead(LEDazul)); //toggle
        parpadeos = parpadeos + 1;
        contador = 0;
      }
    }
    else{
      if (contador2 < 90){
        contador2 = contador2 + 1;
      }
      else{
        contador2 = 0;
        parpadeos = 0;
      }
    }
  }

  V = analogRead(LDRPin);         
  ilum = ((long)V*A*10)/((long)B*Rc*(1024-V)); //ecuacion tomada de https://aprendiendoarduino.wordpress.com/tag/ldr/
  
  //toma valores para el sensor de temperatura
  valorTermistor = analogRead(A0); //leemos el voltaje que entra al pin A0, valor de 0 a 1023
  hart();
  tempDisplay = round(TEMPERATURA); //convertimos el valor de 10 bits (0-1023) a un valor acorde al voltaje (0V-5V)
  
  //toma valores para el sensor de humedad
  valorHumedad = analogRead(A15); //leemos el voltaje que entra al pin A15, valor de 0 a 1023
  humedadNormalizada = valorHumedad/10.23; //convertimos el valor de 10 bits (0-1023) a un valor normalizado (0%-100%)
  humedadDisplay = round(humedadNormalizada); //se redondea para no mostrar decimales en la pantalla

  //toma valores para el sensor de viento
  valorViento = analogRead(A14); //leemos el voltaje que entra al pin A14, valor de 0 a 1023
  vientoNormalizado = valorViento/10.23; //convertimos el valor de 10 bits (0-1023) a un valor normalizado (0%-100%)
  vientoVolts = valorViento/204.6; //convertimos la velocidad del viento a un valor de voltaje de 0-5V
  
  vientoVelocidad = (vientoVolts)*1000/(1.525*32.4); //base de la ecuación obtenida de https://mstore.ibda3vision.com/index.php?route=product/product&product_id=365
  vientoDisplay = vientoVelocidad;

  //vientoDisplay = round(vientoNormalizado); //se redondea para no mostrar decimales en la pantalla

  //toma valores para el nivel de la batería
  valorBateria = analogRead(A13)-926; //leemos el voltaje que entra al pin A13, valor de 0 a 1023 -> 0 a 96
  bateriaNormalizada = valorBateria/0.96; //convertimos el valor de 10 bits (0-1023) a un valor normalizado (0%-100%)
  
  if (bateriaNormalizada >= 0){
    bateriaDisplay = round(bateriaNormalizada); //se redondea para no mostrar decimales en la pantalla
  }else{
    bateriaDisplay = 0;
  }

  //bateriaVolts = valorBateria/79.6728972; //valor entre 0 y 12.84V

  battery_low(); 
  rainCheck();
  memoryWrite();
  panelAdjust();
  serial_refresh();
  display_refresh();

  //tb.sendTelemetryInt("temperature", 22);
  //tb.sendTelemetryFloat("humidity", 42.5);

  //tb.loop();
}
