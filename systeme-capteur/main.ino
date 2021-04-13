// ask_transmitter.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to transmit messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) transmitter with an TX-C1 module
// Tested on Arduino Mega, Duemilanova, Uno, Due, Teensy, ESP-12

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library

#define JARDIN_BOX_ID           0x0a01
#define PIN_RADIO_RX            4
#define PIN_RADIO_TX            5
#define PIN_RADIO_PTT           0
#define PIN_SENSOR_DHT          2
#define PIN_RAINFALL_SENSOR     3 // for interrupt
#define PIN_PHOTORESISTOR       0 // ADC
#define PIN_MOISTURE_SENSOR     1 // ADC
#define PIN_RAIN_SENSOR         2 // ADC, to fix

#define RADIO_SPEED             2000
#define RADIO_PTT_INVERTED      false
#define SENSOR_DHT_TYPE         DHT22   // DHT11, DHT21, DHT22  (AM2302), AM2321
#define RAINFALL_BOUNCE         0.011   // en litres

#include "../common/jardin.h"

#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

#include "DHT.h"


RH_ASK radio_ask(RADIO_SPEED, PIN_RADIO_RX, PIN_RADIO_TX,
    PIN_RADIO_PTT, RADIO_PTT_INVERTED);

DHT dht(PIN_SENSOR_DHT, SENSOR_DHT_TYPE);


void initialize_data_entry(jardin_data_abstract_t *data, jardin_event_type_t type)
{
  data->jardin_id = JARDIN_ID;
  data->box_type = CAPTEUR;
  data->box_id = JARDIN_BOX_ID;
  data->event_type = type;
}
void transmit_data_entry(jardin_data_abstract_t *data, size_t length)
{
  radio_ask.send((uint8_t *) data, length);
  if (!radio_ask.waitPacketSent(1000)) {
    Serial.println(F("failed sending message"));
  }
}

void send_uptime() {
  static unsigned long last_millis = 0;
  static unsigned short millis_cycle = 0;
  unsigned long milliseconds = millis();
  jardin_data_uptime_t data_uptime;

  initialize_data_entry((jardin_data_abstract_t *) &data_uptime, UPTIME);
  if (milliseconds < last_millis) {
    millis_cycle++;
    last_millis = milliseconds;
  }
  // a cycle is 4294967.296 seconds (so we will have 0.296s drifting every 49 days)
  data_uptime.uptime = (millis_cycle * 4294967) + (milliseconds / 1000);
  Serial.println(F("Sending event uptime"));
  Serial.println(data_uptime.uptime);
  transmit_data_entry((jardin_data_abstract_t *) &data_uptime, sizeof(data_uptime));
}

// http://idehack.com/blog/tutoriel-arduino-mesure-de-temperature-et-dhumidite-avec-larduino/
void send_temperature() {
  jardin_data_mesure_temperature_t data_temp;

  initialize_data_entry((jardin_data_abstract_t *) &data_temp, MESURE_TEMPERATURE);
  data_temp.sensor_type = SENSOR_DHT_TYPE;
  data_temp.temperature = dht.readTemperature();
  if (isnan(data_temp.temperature)) {
    Serial.println("Failed to read temperature from DHT sensor!");
  } else {
    Serial.println(F("Sending event temperature"));
    Serial.println(data_temp.temperature);
  }
  transmit_data_entry((jardin_data_abstract_t *) &data_temp, sizeof(data_temp));
}
void send_humidity() {
  jardin_data_mesure_humidity_t data_humid;

  initialize_data_entry((jardin_data_abstract_t *) &data_humid, MESURE_HUMIDITY);
  data_humid.sensor_type = SENSOR_DHT_TYPE;
  data_humid.humidity = dht.readHumidity();
  if (isnan(data_humid.humidity)) {
    Serial.println("Failed to read humidity from DHT sensor!");
  } else {
    Serial.println(F("Sending event humidity"));
    Serial.println(data_humid.humidity);
  }
  transmit_data_entry((jardin_data_abstract_t *) &data_humid, sizeof(data_humid));
}
void send_adc(const char *label, jardin_event_type_t type, int pin) {
  jardin_data_mesure_adc_t data;

  initialize_data_entry((jardin_data_abstract_t *) &data, type);

  data.value = analogRead(pin);
  transmit_data_entry((jardin_data_abstract_t *) &data, sizeof(data));
  Serial.print(F("Sending event "));
  Serial.print(label);
  Serial.println(data.value);
}

jardin_data_mesure_rainfall_t data_rainfall;
static unsigned long rain_last = 0;
static unsigned long rain_last_5minutes = 0;
static unsigned long rain_last_hours = 0;
static unsigned long rain_last_days = 0;
static bool rainfall_update = false;

bool irq_rainfall_value(unsigned long duration_ms, unsigned long rain_time,
                          unsigned long &last_time, float values[])
{
  bool update = false;
  unsigned long shift = rain_time - last_time / duration_ms;

  if (shift > 0) {
    unsigned long i;
    for (i = 11 - shift; i >= 0; i--) {
      values[i + shift] = values[i];
    }
    for (i = 0; i < min(shift, 12); i++) {
      values[i] = 0;
    }
    last_time = rain_time;
    update = true;
  }
  values[0] += RAINFALL_BOUNCE;

  return update;
}
void irq_rainfall()
{
  unsigned long rain_time = millis();
  unsigned long rain_interval = rain_time - rain_last;

  if (rain_interval > 10) {

    data_rainfall.rainfall_delta = RAINFALL_BOUNCE;
    data_rainfall.rainfall_acc += RAINFALL_BOUNCE;

    irq_rainfall_value((unsigned long) 1000*60*5, rain_time, rain_last_5minutes, data_rainfall.rainfall_5minutes);
    irq_rainfall_value((unsigned long) 1000*60*60, rain_time, rain_last_hours, data_rainfall.rainfall_hours);
    irq_rainfall_value((unsigned long) 1000*60*60*24, rain_time, rain_last_days, data_rainfall.rainfall_days);

    rainfall_update = true;
    rain_last = rain_time;
  }
}
void rainfall_send()
{
  Serial.println(F("Sending event rainfall"));
  transmit_data_entry((jardin_data_abstract_t *) &data_rainfall, sizeof(data_rainfall));
}
void init_rainfall()
{
  int i;

  initialize_data_entry((jardin_data_abstract_t *) &data_rainfall, MESURE_RAINFALL);
  for (i = 0; i < 12; i++) {
    data_rainfall.rainfall_5minutes[i] = -1;
  }
  for (i = 0; i < 24; i++) {
    data_rainfall.rainfall_hours[i] = -1;
  }
  for (i = 0; i < 7; i++) {
    data_rainfall.rainfall_days[i] = -1;
  }
  rain_last = millis();
  rain_last_5minutes = rain_last;

  attachInterrupt(digitalPinToInterrupt(PIN_RAINFALL_SENSOR), irq_rainfall, CHANGE);
}


void setup()
{
  Serial.begin(9600);
  Serial.println(F("systeme-capteur"));
  if (radio_ask.init()) {
    radio_ask.setModeTx();
  } else {
    Serial.println(F("radio_ask init failed"));
  }

  dht.begin();
  init_rainfall();
}

static short count = -1;

void loop()
{
  if (++count >= 10) {
    count = 0;
  }
  switch (count) {
  case 0:
    send_uptime();
    break;
  case 1:
    send_temperature();
    send_humidity();
    break;
  case 2:
    break;
  case 3:
    send_adc("photoresistance", MESURE_LIGHT, PIN_PHOTORESISTOR);
    break;
  case 4:
    send_adc("moisture", MESURE_MOISTURE, PIN_MOISTURE_SENSOR);
    break;
  case 5:
    send_adc("rain", MESURE_RAIN, PIN_RAIN_SENSOR);
    break;
  case 6:
    break;
  case 7:
    break;
  case 8:
    break;
  case 9:
    rainfall_send();
    break;
  }

  delay(1000);
}
