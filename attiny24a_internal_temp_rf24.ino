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
//  return temperature * 1.21476 - 348.098; //v1 github // use (temperature + temperature / 4 - 348) if not enough program space
  return temperature * 1.25833 - 343.48; //v2 work githubber // use (temperature + temperature / 5 - 343)
}

void setupSevSeg() {
   DDRA=0b11110111;
}

void setup() {
  wdt_reset(); // сбрасываем
  wdt_enable(WDTO_8S); // разрешаем ватчдог 1 сек
  WDTCSR |= _BV(WDIE); // разрешаем прерывания по ватчдогу. Иначе будет резет.
  sei(); // разрешаем прерывания
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  setupADC();
  setupSevSeg();
  
  int temperature = getTemp();
  blinkNumber(temperature);
}

   byte segmentPins[] = {2, 1, 0, 7, 6, 4, 5, 8};
static const byte digitCodeMap[] = {
  //     GFEDCBA  Segments      7-segment map:
//Bdegf-abc
  B11010111, // 0   "0"          AAA2
  B00000011, // 1   "1"         F   B1
  B11100110, // 2   "2"        4F   B
  B10100111, // 3   "3"         5GGG
  B00110011, // 4   "4"         E   C8
  B10110101, // 5   "5"        6E   C
  B11110101, // 6   "6"          DDD7
  B00000111, // 7   "7"
  B11110111, // 8   "8"
  B10110111, // 9   "9"
};

void blink(char digit) {
  PORTA = digitCodeMap[digit];
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
  wdt_reset();

  int temperature = getTemp();
  if (temperature > 40) {
    blinkNumber(temperature);
    wdt_enable(WDTO_4S);
    WDTCSR |= _BV(WDIE);
  } else {
    wdt_enable(WDTO_8S);
    WDTCSR |= _BV(WDIE);
  }
  wdt_reset();
}
