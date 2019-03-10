
// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"

//Define PINS
#define FIRST_GYRO    2
#define SECOND_GYRO   3
#define THIRD_GYRO    4

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t gx, gy, gz;

//STATUS OF THE GYROS
bool frst_gyr;
bool scnd_gyr;
//bool thrd_gyr;

// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO


void setup() {
  // put your setup code here, to run once:
  timer_setup();

  /**************** Control PINs initialize ************/
  pinMode(FIRST_GYRO, OUTPUT);
  pinMode(SECOND_GYRO, OUTPUT);
  //pinMode(THIRD_GYRO, OUTPUT);
  /**************** I2C initialize **************/
  // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    /************** MPU Initialize ****************/
    accelgyro.initialize();

    /*********** Testing Serial *****************/
    Serial.begin(38400);

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

   //////////////// Lets party **************/ 
   digitalWrite(FIRST_GYRO, HIGH);
   digitalWrite(SECOND_GYRO, LOW);
   //digitalWrite(THIRD_GYRO, HIGH);
   
}

void loop() {
  frst_gyr = digitalRead(FIRST_GYRO);
  scnd_gyr = digitalRead(SECOND_GYRO);
  //thrd_gyr = digitalRead(THIRD_GYRO);
  
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
        Serial.print("a/g:\t");
        Serial.print(ax); Serial.print("\t");
        Serial.print(ay); Serial.print("\t");
        Serial.print(az); Serial.print("\t");
        Serial.print(gx); Serial.print("\t");
        Serial.print(gy); Serial.print("\t");
        Serial.println(gz);
    #endif

    #ifdef OUTPUT_BINARY_ACCELGYRO
        Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
        Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
        Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
        Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
        Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
        Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
    #endif

    
}

void timer_setup(){
  cli();  //para interrupciones
  
  PRR |= (0 << 3);  // power reduction register -> encender el timer

  TCCR1A = 0; // inicializar control
  TCCR1B = 0;

  TCNT1 = 0;  // inicializar contador a 0

  TCCR1B |= (1 << WGM12); // CTC clear timer on compare (reiniciar contador cada vez que se active la interrupcion)

  TCCR1B |= (1 << CS12) | (1 << CS10);  // 1024 preescaler (16MHz / 1024 = f_timer = 15625 Hz)

  OCR1A = 15624;  // cuenta de 0 a 15624 -> f_timer / 15625 = f_efectiva = 1 Hz (cambiar aqui la frecuencia)

  TIMSK1 |= (1 << OCIE1A);  //activar interrupcion del timer;

  sei();  // activa interrupciones
}

ISR(TIMER1_COMPA_vect){
  // esta es la atencion a la interrupcion
  // se ejecuta cada vez que se activa la interrupcion (ahora mismo, a 1 Hz, una vez cada segundo)
  // lo que este aqui dentro tiene que hacerse antes de que vuelva a generarse la interrupcion (ahora mismo, tiene una duracion maxima de 1 segundo)
  if(!frst_gyr){
    //Serial.println("1");
   digitalWrite(FIRST_GYRO, HIGH);
   digitalWrite(SECOND_GYRO, LOW);
  }else if(!scnd_gyr){
    //Serial.println("2");   
   digitalWrite(FIRST_GYRO, LOW);
   digitalWrite(SECOND_GYRO, HIGH);
  }
}






