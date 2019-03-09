void setup() {
  timer_setup();

}

void loop() {
  // put your main code here, to run repeatedly:

}

void timer_setup() {
  cli(); //para interrupciones
  
  PRR |= 0 << 3 // power reduction register -> encender el timer

  TCCR1A = 0; // inicializar control
  TCCR1B = 0; // 
  
  TCNT1  = 0; // inicializar contador a 0
  
  
  TCCR1B |= (1 << WGM12); // CTC clear timer on compare (reiniciar contador cada vez que se active la interrupcion)

  TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 preescaler (16MHz / 1024 = f_timer = 15625 Hz)
  
  OCR1A = 15624; // cuenta de 0 a 15624 -> f_timer / 15625 = f_efectiva = 1 Hz (cambiar aqui la frecuencia)

  TIMSK1 |= (1 << OCIE1A); //activar interrupcion del timer;

  sei(); // activa interrupciones
}

ISR(TIMER1_COMPA_vect) {
  // esta es la atencion a la interrupcion
  // se ejecuta cada vez que se activa la interrupcion (ahora mismo, a 1 Hz, una vez cada segundo)
  // lo que este aqui dentro tiene que hacerse antes de que vuelva a generarse la nterrupcion (ahora mismo, tiene una duracion maxima de 1 segundo)

  
}
