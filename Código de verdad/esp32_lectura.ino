#include <MPU6050.h>
#include <I2Cdev.h>
#include <U8x8lib.h>
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
MPU6050 accelgyro(0x69);

// AD0 pin of acc
const int ad0_pin[] = {0, 17, 23, 19, 2, 5}; // pines de los acelerometros
const int led_pin = 25;

int16_t gx, gy, gz;

hw_timer_t * timer = NULL;

volatile int16_t dedos[5];
char buf[12];

// pone un acelerometro a nivel alto, lee sus datos, y lo vuelve  aponer a nivel bajo
void read_acc(int pin, int16_t *x, int16_t *y, int16_t *z) {
  digitalWrite(pin, HIGH);
  accelgyro.getMotion6 (x, y, z, &gx, &gy, &gz);
  digitalWrite(pin, LOW);
}

// escribe en la pantalla del esp32 (no se si esto funciona junto a lo de leer)
static void printOnScreen()
{
  int i;
  for(i = 0; i < 5; i++) {
    sprintf(buf, "%d", dedos[i]);
    u8x8.drawString(0, i, buf);
  }
}


// Esta funcion se ejecuta cada vez que salta la interrupcion periodica
void IRAM_ATTR onTimer() {
  digitalWrite(led_pin, !digitalRead(led_pin));
  int16_t ref_x, ref_y, ref_z;
  read_acc(ad0_pin[5], &ref_x, &ref_y, &ref_z);

  int i;
  for(i = 0; i < 5; i++) {
    int16_t x, y, z;
    read_acc(ad0_pin[i], &x, &y, &z);
    dedos[i] = sqrt(sq(ref_x-x)+sq(ref_y-y)+sq(ref_z-z));
  }
  
}



void setup() {
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  timer = timerBegin(0, 80, true); // divisor de 80 para el timer (funciona a 80 MHz)
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true); // contador -> cuenta hasta 1 millon -> la interrupcion es cada 1 segundo
  timerAlarmEnable(timer);
  
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "INIT");

  pinMode(led_pin, OUTPUT); // LED
  digitalWrite(led_pin, HIGH);

  int i;
  for (i = 0; i < 6; i++) { // inicializa todos los acelerometros y los pone a nivel bajo
    pinMode(ad0_pin[i], OUTPUT);
    digitalWrite(ad0_pin[i], HIGH);
    accelgyro.initialize();
    digitalWrite(ad0_pin[i], LOW);
  }
}

void loop() {
  printOnScreen();
}
