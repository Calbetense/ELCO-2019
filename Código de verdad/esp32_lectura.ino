#include <MPU6050.h>
#include <I2Cdev.h>
#include <U8x8lib.h>
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
MPU6050 accelgyro(0x69);

// AD0 pin of acc
const int ad0_pin[] = {0, 17, 23, 19, 2, 5};
const int led_pin = 25;

hw_timer_t * timer = NULL;

volatile float dedos[5];
char buf[12];

void read_acc(int pin, int *x, int *y, int *z) {
  digitalWrite(pin, HIGH);
  accelgyro.initialize();
  *x = accelgyro.getAccelerationX ();
  *y = accelgyro.getAccelerationY ();
  *z = accelgyro.getAccelerationZ ();
  digitalWrite(pin, LOW);
}

static void doSomeWork()
{
  int i;
  for(i = 0; i < 5; i++) {
    sprintf(buf, "%f", dedos[i]);
    u8x8.drawString(0, i, buf);
  }
}

void IRAM_ATTR onTimer() {
  digitalWrite(led_pin, !digitalRead(led_pin));
  int ref_x, ref_y, ref_z;
  read_acc(ad0_pin[5], &ref_x, &ref_y, &ref_z);

  int i;
  for(i = 0; i < 5; i++) {
    int x, y, z;
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

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 100000, true);
  timerAlarmEnable(timer);

  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  pinMode(led_pin, OUTPUT); // LED
  digitalWrite(led_pin, HIGH);
}

void loop() {
  doSomeWork();
}
