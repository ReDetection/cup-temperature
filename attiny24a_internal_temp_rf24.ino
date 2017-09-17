#include <avr/wdt.h> 
#include <avr/sleep.h>
#include <avr/interrupt.h>

void setupADC(){

  //ADC Multiplexer Selection Register
  ADMUX = 0;
  ADMUX = 0b10100010;
//  ADMUX |= (1 << REFS1);  //Internal 2.56V Voltage Reference with external capacitor on AREF pin
//  ADMUX |= (0 << REFS0);  //Internal 2.56V Voltage Reference with external capacitor on AREF pin
//  ADMUX |= (1 << MUX5);  //Temperature Sensor - 100111
//  ADMUX |= (0 << MUX4);  //Temperature Sensor - 100111
//  ADMUX |= (0 << MUX3);  //Temperature Sensor - 100111
//  ADMUX |= (0 << MUX2);  //Temperature Sensor - 100111
//  ADMUX |= (1 << MUX1);  //Temperature Sensor - 100111
//  ADMUX |= (0 << MUX0);  //Temperature Sensor - 100111

  //ADC Control and Status Register A 
  ADCSRA = 0;
  ADCSRA |= (1 << ADEN);  //Enable the ADC
  ADCSRA |= (1 << ADPS2);  //ADC Prescaler - 16 (16MHz -> 1MHz)

  //ADC Control and Status Register B 
  ADCSRB = 0;
  ADCSRB |= (1 << MUX5);  //Temperature Sensor - 100111
}

int getTemp() {
  ADCSRA |= (1 << ADSC);  //Start temperature conversion
  while (bit_is_set(ADCSRA, ADSC));  //Wait for conversion to finish
  byte low  = ADCL;
  byte high = ADCH;
  int temperature = (high << 8) | low;  //Result is in kelvin
  return temperature - 273;
}

void blink(byte times) {
  for (;times>0;times--) {
    digitalWrite(9, HIGH);
    delay(100);
    digitalWrite(9, LOW);
    delay(100); 
  }
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  setupADC();
  pinMode(9, OUTPUT);

  blink(5);
  wdt_reset(); // сбрасываем
  wdt_enable(WDTO_8S); // разрешаем ватчдог 1 сек
  WDTCSR |= _BV(WDIE); // разрешаем прерывания по ватчдогу. Иначе будет резет.
  sei(); // разрешаем прерывания
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void blinkNumber(int number) {
  if (number < 0) {
    digitalWrite(9, HIGH);
    delay(1000);
    digitalWrite(9, LOW);
    delay(300);
    number *= -1;
  }
  while(number > 0) {
    byte digit = number % 10;
    number = number / 10;
    blink(digit + 1);
    delay(250);
  }
}

ISR (WDT_vect) {
  WDTCSR |= _BV(WDIE);
}

void loop() {
  sleep_enable();
  sleep_cpu();

  blinkNumber(getTemp());
  wdt_reset();
}
