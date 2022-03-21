#include <Adafruit_PCD8544.h>
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdio.h>

//definición de pines
const int LEDazul=3;
const int LEDrojo=2;
const int calentador=9;
const int comm=13;

// definimos LCD
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 5, 6, 4, 8);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

//definicion de parámetros necesarios para las ecuaciones de S-H para estimar la temperatura a partir del termistor
float R1 = 100000; //resistencia de 100k en serie con el termistor.
float logR2, R2, TEMPERATURA;

//coeficientes de steinhart-hart, obtenidos de: https://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
float c1 = 0.8586139205e-03, c2 = 2.059709585e-04, c3 = 0.8130635267e-07; 

float dutyCycle;
int valorCalentador=0;

int valorTermistor; //numero de 0 a 1023 entrada A0
float voltajeTermistor; //numero 0 a 5 entrada A04

float valorDeseado;
float valorDeseadoDisplay;

int valorHumedad; //numero de 0 a 1023 entrada A5
float humedadNormalizada; //numero 0 a 100 entrada A5
float humedadDisplay;

float error;

void hart(){ //uiliza una ecuación para estimar la temperatura de acuerdo a la resistencia del termistor
  R2 = R1 * (1023.0 / (float)valorTermistor - 1.0);
  logR2 = log(R2);
  TEMPERATURA = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); //ecuacion de S-H, da temperatura en kelvin
  TEMPERATURA = TEMPERATURA -273.15; //convertimos a grados centigrados la temperatura
}

void alerta_seguridad(){ //se activa un led rojo si la temperatura es superior a 42 grados o un led azul si es menor a 30 grados
  if ((TEMPERATURA < 30)){ //2.74V -> 30 grados celsius
    digitalWrite(LEDazul, HIGH);
    digitalWrite(LEDrojo, LOW);
  }
  else if ((TEMPERATURA > 30) & (TEMPERATURA < 42)){ //funcionamiento normal
    digitalWrite(LEDrojo, LOW);
    digitalWrite(LEDazul, LOW);
  }
  else if ((TEMPERATURA > 42)){ //3.26V -> 42 grados celsius
    digitalWrite(LEDazul, LOW);
    digitalWrite(LEDrojo, HIGH);
  }
}

void indicador_humedad(){ //enciende el led integrado si se supera el 50% de humedad relativa
  if (humedadNormalizada > 50){
    digitalWrite(LED_BUILTIN, HIGH);    
  }
  else{
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void blink(){ //parpadea el led integrado
  //led parpadea una vez cada vez que inicia el loop
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);                       
  digitalWrite(LED_BUILTIN, LOW);    
  delay(300);
}

float ajusteCalentador(float target){
  // Checks de seguridad para prevenir cocinar los huevos
  if(target > 42){
    if(TEMPERATURA <= 42){
      analogWrite(calentador, roundf(42*(255/80)));
      return 42*100/80;
    }else{
      analogWrite(calentador, roundf(0*(255/80)));
      return 0*100/80;
    }
  }else if(target < 30){
    if(TEMPERATURA >= 30){
      analogWrite(calentador, roundf(0*(255/80)));
      return 0*100/80;
    }else{
      analogWrite(calentador, roundf(30*(255/80)));
      return 30*100/80;
    }
  }else{
    if(target < TEMPERATURA){
      analogWrite(calentador, roundf(0*(255/80)));
      return 0*100/80;
    }else{
      analogWrite(calentador, roundf(target*(255/80)));
      return target*100/80;
    }
  }
}

void display_refresh(){
  display.setCursor(0,0);
  display.print("T.Set:");
  display.println(valorDeseadoDisplay);
  display.print("Temp:");
  display.println(TEMPERATURA);
  display.print("Duty:");
  display.print(dutyCycle);
  display.println("%");
  display.print("Hum:");
  display.print(humedadDisplay);
  display.println("%");
  display.display();
  display.clearDisplay();
}

float pidControl(float set, float current){
    return set - current;
}

void setup() {
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
}

void loop() { // loop infinito
  if(digitalRead(comm) == LOW){
    Serial.print("TEMPERATURA:");
    Serial.print(TEMPERATURA);
    Serial.println(" C");
    Serial.print("HUMEDAD:");
    Serial.print(humedadDisplay);
    Serial.println("%");
  }
  valorDeseado = analogRead(A4);
  valorDeseadoDisplay = (valorDeseado)/5.11;

  valorTermistor = analogRead(A0); //leemos el voltaje que entra al pin A0, valor de 0 a 1023
  voltajeTermistor = valorTermistor/204.6; //convertimos el valor de 10 bits (0-1023) a un valor acorde al voltaje (0V-5V)
  
  valorHumedad = analogRead(A5); //leemos el voltaje que entra al pin A5, valor de 0 a 1023
  humedadNormalizada = valorHumedad/14.61428571; //convertimos el valor de 10 bits (0-1023) a un valor normalizado (0%-100%)
  humedadDisplay = humedadNormalizada+20;
  alerta_seguridad();

  hart();

  error = pidControl(valorDeseadoDisplay, TEMPERATURA);

  if(error != 0){
    dutyCycle = ajusteCalentador(TEMPERATURA+error);
  }else{
    dutyCycle = ajusteCalentador(0);
  }
  display_refresh();
}
