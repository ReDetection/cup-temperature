#include <SevSeg.h>
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

void setupSevSeg() {
   DDRA=0b11110111;
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  setupADC();
  setupSevSeg();

  wdt_reset(); // сбрасываем
  wdt_enable(WDTO_8S); // разрешаем ватчдог 1 сек
  WDTCSR |= _BV(WDIE); // разрешаем прерывания по ватчдогу. Иначе будет резет.
  sei(); // разрешаем прерывания
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

static const byte digitCodeMap[] = {
  //     GFEDCBA  Segments      7-segment map:
  B00111111, // 0   "0"          AAA
  B00000110, // 1   "1"         F   B
  B01011011, // 2   "2"         F   B
  B01001111, // 3   "3"          GGG
  B01100110, // 4   "4"         E   C
  B01101101, // 5   "5"         E   C
  B01111101, // 6   "6"          DDD
  B00000111, // 7   "7"
  B01111111, // 8   "8"
  B01101111, // 9   "9"
};

void blink(char digit) {
  PORTA = c;
  delay(100);
  PORTA = 0;
}

void blinkNumber(int number) {
  blink(number / 10);
  delay(100);
  blink(number % 10);
}

ISR (WDT_vect) {
  WDTCSR |= _BV(WDIE);
}

void loop() {
  sleep_enable();
  sleep_cpu();

  int temperature = getTemp();
  if (temperature > 40) {
    blinkNumber(temperature);
    wdt_enable(WDTO_4S);
  } else {
    wdt_enable(WDTO_8S);
  }
  wdt_reset();
}
