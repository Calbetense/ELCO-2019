#include "I2Cdev.h"
#include "MPU6050.h"
#include <U8x8lib.h>
#include "Wire.h"
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
MPU6050 accelgyro(0x69); // <-- use for AD0 high

hw_timer_t * timer = NULL;

int16_t ax, ay, az, ax_r, ay_r, az_r;
const int ad0_pin[] = {0, 5, 23, 19, 2, 17};
char buf[10];
int n = 3;
bool w = false;

typedef enum {no, up, down, side} pos;
static pos pos1 = no;

static int pos_dedos[5];

void IRAM_ATTR onTimer() {
  digitalWrite(ad0_pin[n++], LOW);
  n = n%6;
  digitalWrite(ad0_pin[n], HIGH);
  w = true;
}

void setup() {
  Wire.begin();
  int i;
  for (i = 0; i < 6; i++) {
    pinMode(ad0_pin[i], OUTPUT);
    digitalWrite(ad0_pin[i], HIGH);
    accelgyro.initialize();
    digitalWrite(ad0_pin[i], LOW);
  }
  delay(250);


  Serial.begin(115200);

  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  timer = timerBegin(0, 80, true); // divisor de 80 para el timer (funciona a 80 MHz)
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 50000, true); // contador -> cuenta hasta 1 millon -> la interrupcion es cada 1 segundo
  timerAlarmEnable(timer);

}

void loop() {
  if (w) {
    
    if (n == 5) {
      accelgyro.getAcceleration(&ax_r, &ay_r, &az_r);
      /*sprintf(buf, "%06d", ax_r);
      u8x8.drawString(0, 0, buf);
      sprintf(buf, "%06d", ay_r);
      u8x8.drawString(0, 1, buf);
      sprintf(buf, "%06d", az_r);
      u8x8.drawString(0, 2, buf);*/
      if (ax_r > 13000) pos1 = up;
      else if (ax_r < -13000) pos1 = down;
      else if (ay_r > 13000) pos1 = side;
      else pos1 = no;
      //Serial.println(pos1);
      
    } else { //if (n == 0) {
      accelgyro.getAcceleration(&ax, &ay, &az);

      switch (pos1) {
        case up:
          if (ax > 13500) pos_dedos[n] = 1;
          else if (az > 13500) pos_dedos[n] = 2;
          else if (ax < -13500) pos_dedos[n] = 3;
          else if (az < -13500) pos_dedos[n] = 4;
          else pos_dedos[0] = 0;
          break;
        case down:
          break;
        case side:
          break;
        default:
          pos_dedos[n] = 0;
          break;
      }

      Serial.print(pos_dedos[n]);
      Serial.print(" ");
      if (n==4) Serial.println();
      
      /*sprintf(buf, "%06d", ax);
      u8x8.drawString(0, 0, buf);
      sprintf(buf, "%06d", ay);
      u8x8.drawString(0, 1, buf);
      sprintf(buf, "%06d", az);
      u8x8.drawString(0, 2, buf);*/
    }
    
    w = false;
  }
}

static inline int sign(int x) {
  if (x < 0) return -1;
  else if (x == 0) return 0;
  return 1;
}
