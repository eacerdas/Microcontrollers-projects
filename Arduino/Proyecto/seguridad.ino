#include <Servo.h>
#include <Adafruit_PCD8544.h>
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdio.h>
#include <Keypad.h>
#include <TimerOne.h>

//definicion de pines generales
const int comm = 12;
const int alarmSound = 46;

//TECLADO
char TECLA;
const byte ROWS = 4; //4 filas
const byte COLS = 3; //3 columnas

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {49, 47, 45, 43}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {31, 33, 41}; //connect to the column pinouts of the keypad
Keypad teclado = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

byte INDICE = 0;
int teclaDisplay = 0;
int ALARM_ON = 0;

//PASSWORD
char CLAVE[5];
char CLAVE_MAESTRA[5] = "1234";
char CLAVE_CAMBIAR[5] = "#*#*";
int correctPassword = 2;
const int cambiarPass = 27;
int changePassword = 0;
int passwordChanged = 1;
int flagPass = 0;

//sensor ultrasonico
const int echo = 2;
const int trig = 3;

const int echo2 = 10;
const int trig2 = 11;

int dist = 0;
int dist2 = 0;

const int cam1 = 25;
const int cam2 = 23;

int movement = 0;
int movement2 = 0;

// definimos LCD
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 5, 6, 4, 8);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

//front door
const int frontDoor = 53;
const int frontDoorLED = 52;
int frontDoorState = 0;

//back door
const int backDoor = 44;
const int backDoorLED = 48;
int backDoorState = 0;

//window
int windowState = 0;
const int window = 51;
const int windowLED = 50;

//CONTADORES PARA EL PARPADEO DEL LED de comm
int contador = 0;
int contador2 = 0;
int parpadeos = 0;

//Servos de puertas
Servo lockFront;
Servo lockBack;

// Detector de cambios para serial
int pastRun[9];
int currentRun[9];

// Senal recibida por serial
int locker;

void blink(){ //parpadea el led integrado
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);                       
  digitalWrite(LED_BUILTIN, LOW);    
  delay(300);
}

void movement_detected(){ //camera number 1
  if (dist > 1){
    digitalWrite(cam1, HIGH);
    movement = 1;
  }else{
    movement = 0;
    digitalWrite(cam1, LOW);
  }
} 

void movement_detected2(){ //camera number 2
  if (dist2 > 1){
    digitalWrite(cam2, HIGH);
    movement2 = 1;
  }else{
    movement2 = 0;
    digitalWrite(cam2, LOW);
  }
} 

void window_open(){
  if (digitalRead(window) == HIGH){
    windowState = 1;
    digitalWrite(windowLED, HIGH);
  }
  else{
    windowState = 0;
    digitalWrite(windowLED, LOW);
  }
}

void front_door_open(){
  if (digitalRead(frontDoor) == HIGH){
    frontDoorState = 1;
    digitalWrite(frontDoorLED, HIGH);
  }
  else{
    frontDoorState = 0;
    digitalWrite(frontDoorLED, LOW);
  }
}

void back_door_open(){
  if (digitalRead(backDoor) == HIGH){
    backDoorState = 1;
    digitalWrite(backDoorLED, HIGH);
  }
  else{
    backDoorState = 0;
    digitalWrite(backDoorLED, LOW);
  }
}

void soundAlert(){
  if ((frontDoorState == 1) | (backDoorState == 1) | (windowState == 1) | (movement == 1) | (movement2 == 1)){
    digitalWrite(alarmSound, HIGH);
  }
  else{
    digitalWrite(alarmSound, LOW);
  }
}

void trigger(){
  digitalWrite(trig, HIGH);
  delayMicroseconds(10); //Enviamos un pulso de 10us
  digitalWrite(trig, LOW);
}

void trigger2(){
  digitalWrite(trig2, HIGH);
  delayMicroseconds(10); //Enviamos un pulso de 10us
  digitalWrite(trig2, LOW);
}

float calcular_distancia(){
  float t = pulseIn(echo, HIGH); //obtenemos el ancho del pulso
  float d = t/59;             //escalamos el tiempo a una distancia en cm
  
  int dround = round(d);
  return dround;
}

float calcular_distancia2(){
  float t2 = pulseIn(echo2, HIGH); //obtenemos el ancho del pulso
  float d2 = t2/59;             //escalamos el tiempo a una distancia en cm
  
  int dround2 = round(d2);
  return dround2;
}

void display_refresh(){ // Se refrescan los datos en la pantalla con los resultados mas recientes
  display.setCursor(0,0);

  if ((changePassword == 1) & (ALARM_ON == 0)){
    
    display.println("CHANGING PASS");
    display.println("");
    display.println("");
    display.println("Type the new");
    display.print("pass: ");
    display.print(CLAVE);
    display.println("");
  }

  else if (changePassword == 0){

    if (correctPassword == 1 & flagPass == 0){
      display.print("CHECK: ");
      display.println("CORRECT");
      display.println("");
      display.print("Pass: ");
      display.println(CLAVE);
      display.println("");
      display.print("Alarm: ");

      if (ALARM_ON == 1){
        display.print("ON");
      }
      else{
        display.print("OFF");
      }
    }
    else if(correctPassword == 0 & flagPass == 0){
      display.print("CHECK: ");
      display.println("WRONG");
      display.println("");
      display.print("Pass: ");
      display.println(CLAVE);
      display.println("");
      display.print("Alarm: ");

      if (ALARM_ON == 1){
        display.print("ON");
      }
      else{
        display.print("OFF");
      }
    }
    else if (flagPass == 1){
      display.println("CHANGES SAVED");
      display.println("");
      display.print("Pass: ");
      display.println(CLAVE);
      display.println("");
      display.println("Alarm: OFF");
    }
    else{
      display.print("CHECK: ");
      display.println("WAITING");
      display.println("");
      display.print("Pass: ");
      display.println(CLAVE);
      display.println("");
      display.println("Alarm: OFF");
    }
    display.println("");
  }
  display.display();
  display.clearDisplay();
}

void alarms_off(){
  digitalWrite(frontDoorLED, LOW);
  digitalWrite(backDoorLED, LOW);
  digitalWrite(windowLED, LOW);
  digitalWrite(alarmSound, LOW);
  digitalWrite(cam1, LOW);
  digitalWrite(cam2, LOW);
}

bool areEqual(){
  for (int i = 0; i < 8; i++){
    if (currentRun[i] != pastRun[i]){
      return true;
      }
  }
  return false;
}

void serial_refresh(){ //Si la comunicacion esta activada, se envian los datos
  locker = Serial.read();
  if(locker == 49){
    lock(1);
    lock(2);
  }
  if(digitalRead(comm) == LOW){
    //Status de alarma
      currentRun[0] = ALARM_ON;
      
      //Status de alarma activada
      if ((frontDoorState == 1) | windowState == 1 | movement == 1 | movement2 == 1){
        currentRun[1] = 1;
      }else{
        currentRun[1] = 0;
      }
      
      //Status de puerta frontal
      currentRun[2] = frontDoorState;

      //Status de puerta trasera
      currentRun[3] = backDoorState;
      
      //Condicion de lock de puerta frontal
      if(lockFront.read() >= 140){
        currentRun[4] = 1;
      }else{
        currentRun[4] = 0;
      }
      
      //Condicion de lock de puerta trasera
      if(lockBack.read() >= 140){
        currentRun[5] = 1;
      }else{
        currentRun[5] = 0;
      }
      
      //Status de ventana
      currentRun[6] = windowState;
      
      //Status de sensor movimiento 1
      currentRun[7] = movement;
      
      //Status de sensor movimiento 2
      currentRun[8] = movement2;

    if(areEqual()){
      //Status de alarma
      Serial.print(ALARM_ON);
      pastRun[0] = ALARM_ON;
      Serial.print(",");
      
      //Status de alarma activada
      if ((frontDoorState == 1) | windowState == 1 | movement == 1 | movement2 == 1){
        Serial.print("1");
        pastRun[1] = 1;
      }else{
        Serial.print("0");
        pastRun[1] = 0;
      }
      Serial.print(",");
      
      //Status de puerta
      Serial.print(frontDoorState);
      pastRun[2] = frontDoorState;
      Serial.print(",");

      //Status de puerta trasera
      Serial.print(backDoorState);
      pastRun[3] = backDoorState;
      Serial.print(",");
      
      //Condicion de lock de puerta frontal
      if(lockFront.read() >= 140){
        Serial.print("1");
        pastRun[4] = 1;
      }else{
        Serial.print("0");
        pastRun[4] = 0;
      }
      Serial.print(",");
      
      //Condicion de lock de puerta trasera
      if(lockBack.read() >= 140){
        Serial.print("1");
        pastRun[5] = 1;
      }else{
        Serial.print("0");
        pastRun[5] = 0;
      }
      Serial.print(",");
      
      //Status de ventana
      Serial.print(windowState);
      pastRun[6] = windowState;
      Serial.print(",");
      
      //Status de sensor movimiento 1
      Serial.print(movement);
      pastRun[7] = movement;
      Serial.print(",");
      
      //Status de sensor movimiento 2
      Serial.println(movement2);
      pastRun[8] = movement2;
    }
  }
}

void unlock(int door){
  if(door == 1){
    lockFront.write(0);
  }else if(door ==2){
    lockBack.write(0);
  }
}

void lock(int door){
  if(door == 1){
    lockFront.write(180);
  }else if(door ==2){
    lockBack.write(180);
  }
}

void isr(){
  
  TECLA = teclado.getKey();
  
  if ((changePassword == 1) & (ALARM_ON == 0)){ 
    if (TECLA){ // comprueba que se haya presionado una tecla
      CLAVE[INDICE] = TECLA;
      CLAVE_MAESTRA[INDICE] = TECLA;
      INDICE++;
    }
    if(INDICE == 4){
      INDICE = 0;
      flagPass = 1;
      passwordChanged = 1;
      changePassword = 0;
      digitalWrite(cambiarPass, LOW);
      for (int i = 0; i < 4; ++i){ //llenamos con guiones la clave de nuevo
        CLAVE[i] = '-'; 
      }
    }
  }
  else if ((changePassword == 1) & (ALARM_ON == 1)){
    passwordChanged = 0;
    correctPassword = 0;
    changePassword = 0;
    digitalWrite(cambiarPass, LOW);
  }
  else if (changePassword == 0){
    passwordChanged = 0;
    if (TECLA){ // comprueba que se haya presionado una tecla
      CLAVE[INDICE] = TECLA; // almacena en array la tecla presionada
      if (!strcmp(CLAVE, CLAVE_CAMBIAR)){
        INDICE = 0;
        for (int i = 0; i < 4; ++i){ //llenamos con guiones la clave de nuevo
          CLAVE[i] = '-'; 
        }
        digitalWrite(cambiarPass, HIGH);
        changePassword = 1;
      }
      else{
        digitalWrite(cambiarPass, LOW);
        INDICE++; // incrementa indice en uno
      }
      
    }
    if(INDICE == 4){ // si ya se almacenaron los 4 digitos
      flagPass = 0;
      if(!strcmp(CLAVE, CLAVE_MAESTRA)){ // compara clave ingresada con clave maestra
        correctPassword = 1;  // imprime en pantalla que es correcta la clave
        if (ALARM_ON == 1){ //si la pass es correcta, se cambia de estado a la alarma
          ALARM_ON = 0;
          unlock(1);
          unlock(2);
        }
        else{
          ALARM_ON = 1;
          lock(1);
          lock(2);
        }
        for (int i = 0; i < 4; ++i){ //llenamos con guiones la clave de nuevo
          CLAVE[i] = '-'; 
        }
      }
      else{
        for (int i = 0; i < 4; ++i){
          CLAVE[i] = '-';
        }
        correctPassword = 0;  // imprime en pantalla que es incorrecta la clave
      }
      INDICE = 0;
    }
  }
}

void setup() {
  Serial.begin(9600);
  for(int i =0;i<8;i++){
    pastRun[i] = 0;
  }
  
  for (int i = 0; i < 4; ++i){ //llenamos CLAVE con guiones
    CLAVE[i] = '-';
  }

  //inicializacion de pantalla
  Serial.println("PCD test");
  display.begin();
  display.setContrast(75);
  display.display(); // show splashscreen
  delay(500);
  display.clearDisplay(); // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);

  //configuracion de pines
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(frontDoorLED, OUTPUT);
  pinMode(backDoorLED, OUTPUT);
  pinMode(windowLED, OUTPUT);
  pinMode(alarmSound, OUTPUT);
  pinMode(cam1, OUTPUT);
  pinMode(cam2, OUTPUT);
  pinMode(trig,OUTPUT);
  pinMode(trig2,OUTPUT);
  pinMode(cambiarPass,OUTPUT);

  pinMode(echo,INPUT);
  pinMode(echo2,INPUT);
  pinMode(comm, INPUT);

  //Configuracion de servos
  lockFront.attach(8, 1000, 2000);
  lockBack.attach(9, 1000, 2000);

  //digitalWrite(trig, LOW);// Inicializamos el pin con 0
  digitalWrite(trig2, LOW);// Inicializamos el pin con 0

  //timer
  Timer1.initialize(100000);//Cada cuantos us ocurre le interrupt
  Timer1.attachInterrupt(isr, 100000);
}

void loop() { // loop infinito

  trigger();
  dist = calcular_distancia();

  delayMicroseconds(2); //delay necesario para leer los sensores

  trigger2();
  dist2 = calcular_distancia2();
  
  if (ALARM_ON == 1){
    front_door_open(); //revisa si se abrio una puerta
    back_door_open(); //revisa si se abrio una puerta
    window_open(); //revisa si se abrio una ventana
    movement_detected(); //revisa si hubo movimiento cerca de la cam1
    movement_detected2(); //revisa si hubo movimiento cerca de la cam2
    soundAlert(); //enciende la alarma de sonido
  }
  else{
    alarms_off();
  }

  serial_refresh();
  display_refresh();
}
