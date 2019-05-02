#include "I2Cdev.h"
#include "MPU6050.h"
#include <U8x8lib.h>
#include "Wire.h"
#include "DFRobotDFPlayerMini.h"
#include "Arduino.h"

//REPRODUCCIÓN DFPLAYER
HardwareSerial mySoftwareSerial(1);
DFRobotDFPlayerMini myDFPlayer;

//TEMPORIZACIÓN
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
MPU6050 accelgyro(0x69); // <-- use for AD0 high
hw_timer_t * timer = NULL;

//VARIABLES Y CONSTANTES
int16_t ax, ay, az, ax_r, ay_r, az_r;
const int ad0_pin[] = {0, 5, 23, 19, 2, 17};
char buf[20]; // for sprintf
int n = 0; // accel selection
bool w = false;

//POSICION DE LA MANO
typedef enum {no, up, down, side} pos;
static pos pos1 = no;
static int pos_dedos[5];

//CONTADOR
static int8_t counter;

//LETRAS
static int8_t actual_letter, last_letter;

//TEMPORIZADOR
void IRAM_ATTR onTimer() {
  digitalWrite(ad0_pin[n++], LOW);
  n = n%6;
  digitalWrite(ad0_pin[n], HIGH);
  w = true;
}

/*************************************************************************************************
**************************************INICIALIZACIÓN**********************************************
*************************************************************************************************/
void setup() {
  
  //INICIALIZACIÓN DFPLAYER
  mySoftwareSerial.begin(9600, SERIAL_8N1, 25, 26);
  myDFPlayer.begin(mySoftwareSerial);
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30

  //INICIALIZACIÓN MPU6050
  Wire.begin();
  int i;
  for (i = 0; i < 6; i++) {
    pinMode(ad0_pin[i], OUTPUT);
    digitalWrite(ad0_pin[i], HIGH);
    accelgyro.initialize();
    digitalWrite(ad0_pin[i], LOW);
  }

  //SERIAL
  Serial.begin(115200);

  //INICIALIZACIÓN TEMPORIZADOR
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  timer = timerBegin(0, 80, true); // divisor de 80 para el timer (funciona a 80 MHz)
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 50000, true); // contador -> cuenta hasta 1 millon -> la interrupcion es cada 1 segundo
  timerAlarmEnable(timer);
}

  /*      Pulgar: 4
   *      Índice:. 0 
   *      Corazón: 1 
   *      Anular: 2 
   *      Meñique: 3 
   *      Referencia: 5                                   
   */   

/*************************************************************************************************
**************************************ORIENTACIÓN Y REFERENCIAS***********************************
*************************************************************************************************/
void loop() {
  if (w) {
    
    /********************REFERENCIA (n = 5)********************/
    if (n == 5) {
        accelgyro.getAcceleration(&ax_r, &ay_r, &az_r);
        if (ax_r > 13000) pos1 = up;
        else if (ax_r < -13000) pos1 = down;
        else if (ay_r > 13000) pos1 = side;
        else pos1 = no;
  
        check_letter();
        check_timing();
        
        sprintf(buf, "Posición: %d ", pos1);
        Serial.print(buf);

    /********************PULGAR (n = 4)************************/  
    }else if (n == 4) { // pulgar
        accelgyro.getAcceleration(&ax, &ay, &az);
        switch (pos1) {
          case up:
            if (ax>13000) pos_dedos[4]=1; //vertical
            else if (az<-13000) pos_dedos[4]=2;//posicion en L
            else if (az>13000) pos_dedos[4]=3;//hacia la palma
            else pos_dedos[4]=0;
            break;
          case down:
            if (ax<-13000) pos_dedos[4]=1; //vertical
            else if (az>7000) pos_dedos[4]=2;//posicion en L
            else if (az<-12000) pos_dedos[4]=3;//hacia la palma
            else pos_dedos[4]=0;
            break;
          default:
            break;
        } 
        
       /* // IMPRIMIR EN PANTALLA DEL ESP
        sprintf(buf, "%06d", ax);
        u8x8.drawString(0, 0, buf);
        sprintf(buf, "%06d", ay);
        u8x8.drawString(0, 1, buf);
        sprintf(buf, "%06d", az);
        u8x8.drawString(0, 2, buf);*/
  
        sprintf(buf, "Pulgar= %d", pos_dedos[4]);
        Serial.println(buf);

    /********************RESTO DE DEDOS (n = 0, 1, 2, 3)******************/    
    } else { 
      accelgyro.getAcceleration(&ax, &ay, &az);
      switch (pos1) {
        case up:
          if (ax > 13500)       pos_dedos[n] = 1;    //estirados
          else if (az > 13500)  pos_dedos[n] = 2;    //inclinados
          else if (ax < -13500) pos_dedos[n] = 3;    //doblados 
          else if (az < -13500) pos_dedos[n] = 4;    //"puño cerrado" 
          else pos_dedos[0] = 0;
          break;
        case down:
          if (ax < -13500)      pos_dedos[n] = 1;    //estirados
          else if (az < -13500) pos_dedos[n] = 2;    //inclinados
          else if (ax > 13500)  pos_dedos[n] = 3;    //doblados 
          else if (az > 13500)  pos_dedos[n] = 4;    //"puño cerrado" 
          else pos_dedos[0] = 0;
          break;
        case side:
          break;
        default:
          pos_dedos[n] = 0;
          break;
      }
      if(n == 0)       Serial.print("Índice: ");
      else if(n == 1) Serial.print("Corazón: ");
      else if(n == 2) Serial.print("Anular: ");
      else if(n == 3) Serial.print("Meñique: ");
      Serial.print(pos_dedos[n]);
      Serial.print(" ");

      // IMPRIMIR EN PANTALLA DEL ESP
      if(n==0){
        sprintf(buf, "%06d", ax);
        u8x8.drawString(0, 0, buf);
        sprintf(buf, "%06d", ay);
        u8x8.drawString(0, 1, buf);
        sprintf(buf, "%06d", az);
        u8x8.drawString(0, 2, buf);
      }
    }
    w = false;
  }
}


/*************************************************************************************************
**************************************COMPROBACIÓN DE LAS LETRAS**********************************
*************************************************************************************************/
void check_letter() {
  // A --> OK
  if(pos1 == up && pos_dedos[0]==4 && pos_dedos[1]==4 && pos_dedos[2]==4 && pos_dedos[3]==4) // NO HACE FALTA poner el dedo gordo 
    actual_letter = 1; 
  // B      
  else if(pos1 == side && pos_dedos[0]==1 && pos_dedos[1]==1 && pos_dedos[2]==1 && pos_dedos[3]==1 && pos_dedos[4]==1) //poner el dedo gordo 
    actual_letter = 2;
  // C  
  else if (pos1 == up && pos_dedos[0]==2 && pos_dedos[1]==2 && pos_dedos[2]==2 && pos_dedos[3]==2&& (pos_dedos[4]==1 || pos_dedos[4]==2))
    actual_letter = 3;
  // D
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==3 && pos_dedos[2]==3 && pos_dedos[3]==2 && pos_dedos[4]==1) //revisar el meñique
    actual_letter = 4;
  // E
  else if (pos1 == up && pos_dedos[0]==3 && pos_dedos[1]==3 && pos_dedos[2]==3 && pos_dedos[3]==3 && pos_dedos[4]==2) 
    actual_letter = 5;
  // F
  else if(pos1 == up && pos_dedos[0]==2 && pos_dedos[1]==1 && pos_dedos[2]==1 && pos_dedos[3]==1 && pos_dedos[4]==1) //Revisar el 2
    actual_letter = 6;
  // G  
  else if(pos1 == side && pos_dedos[0]==1 && pos_dedos[1]==3 && pos_dedos[2]==3 && pos_dedos[3]==3 && pos_dedos[4]==1)
    actual_letter = 7;
  // H
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==1 && (pos_dedos[2]==3||pos_dedos[2]==2) && (pos_dedos[3]== 3||pos_dedos[3]==2) && pos_dedos[4]==2) //pulgar en la barbilla
    actual_letter = 8;
  // I 
  else if (pos1 == up && pos_dedos[0]==3 && pos_dedos[1]==3 && pos_dedos[2]==3 && pos_dedos[3]==1 && pos_dedos[4]== 1) 
    actual_letter = 9;
  // J
  else if(pos1 == up && pos_dedos[0]==3 && pos_dedos[1]==3 && pos_dedos[2]==3 && pos_dedos[3]==1 && pos_dedos[4]==3)//como la I (trampear metiendo el pulgar)
    actual_letter = 10;
  // K 
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==2 && pos_dedos[2]==3 && pos_dedos[3]==3 && pos_dedos[4]== 1) //Hacer en vertical (NO COMO EN LA FOTO)
    actual_letter = 11;
  // L
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==3 && pos_dedos[2]==3 && pos_dedos[3]==3 && pos_dedos[4]== 2) 
    actual_letter = 12;
  // M
  else if (pos1 == down && pos_dedos[0]==1 && pos_dedos[1]==1 && pos_dedos[2]==1 && pos_dedos[3]==3 && pos_dedos[4]==3) 
    actual_letter = 13;
  // N
  else if (pos1 == down && pos_dedos[0]==1 && pos_dedos[1]==1 && pos_dedos[2]==3 && pos_dedos[3]==3 && pos_dedos[4]==3) 
    actual_letter = 14;
  // O
  else if (pos1 == up && pos_dedos[0]==3 && pos_dedos[1]==1 && pos_dedos[2]==1 && pos_dedos[3]==1 && (pos_dedos[4]== 1 || pos_dedos[4]==2))  //dedo 1 no se si 2 o 3
    actual_letter = 15;
  // P
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==1 && pos_dedos[2]==1 && pos_dedos[3]==3 && pos_dedos[4]== 3) //enganchar meíque con dedo gordo
    actual_letter = 16;
  // Q

  // R
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==1 && pos_dedos[2]==3 && pos_dedos[3]==3 && pos_dedos[4]==3) //poner el pulgar debajo de meñique y anular
    actual_letter = 18;
  // S
  else if (pos1 == up && pos_dedos[0]==3 && pos_dedos[1]==2 && pos_dedos[2]==1 && pos_dedos[3]==1 && (pos_dedos[4]==1 || pos_dedos[4]==2)) 
    actual_letter = 19; //igual que la O pero doblando un poco el corazon y estirando pulgar    
  // T

  // U
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==1 && pos_dedos[2]==3 && pos_dedos[3]==3 && pos_dedos[4]==1) //poner el pulgar encima de anular y meíque
    actual_letter = 21;
  // V

  // W
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==1 && pos_dedos[2]==1 && pos_dedos[3]==3 && pos_dedos[4]==1) //seria con movimiento pero es la unica con esta forma
    actual_letter = 23;
  // X

  // Y
  else if (pos1 == up && pos_dedos[0]==2 && pos_dedos[1]==3 && pos_dedos[2]==3 && pos_dedos[3]==3 && pos_dedos[4]==1) //seria con movimiento pero es la unica con esta forma
    actual_letter = 25;
  // Y
  else if (pos1 == up && pos_dedos[0]==1 && pos_dedos[1]==3 && pos_dedos[2]==3 && (pos_dedos[3]==3 || pos_dedos[3]==4) && pos_dedos[4]==1) //seria con movimiento pero es la unica con esta forma
    actual_letter = 26;
  // Z
  else if (pos1 == up && pos_dedos[0]==3 && pos_dedos[1]==3 && pos_dedos[2]==3 && pos_dedos[3]==2 && pos_dedos[4]==1) //seria con movimiento pero es la unica con esta forma
    actual_letter = 27;
  else 
    actual_letter = 0;
}


// CONTADOR Y MONITOR
void check_timing() {
  if (actual_letter == last_letter && actual_letter) counter++;
  else counter = 0;
  last_letter = actual_letter;
  Serial.print(counter);
  if (counter == 5) {
    char c = 64 + actual_letter;
    Serial.print("Letra: ");
    Serial.println(c);
    myDFPlayer.play(actual_letter+1);
  }
}

// COMPROBACIÓN SIGNO
static inline int sign(int x) {
  if (x < 0) return -1;
  else if (x == 0) return 0;
  return 1;
}
