// ask_transmitter.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to transmit messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) transmitter with an TX-C1 module
// Tested on Arduino Mega, Duemilanova, Uno, Due, Teensy, ESP-12

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library


//#define HEUCHERE
//#define FOND
//#define ESPJACK

////////////////////////////////////////////////////////////////// heuchere
#ifdef HEUCHERE
#define ENABLE_RADIO
#define JARDIN_BOX_ID           0x0a01  // heuchere
#define NB_LED 8
// DHT: Arduino=9 Combo arduino=3
#define PIN_SENSOR_DHT          3
#define PIN_PHOTORESISTOR       A1 // ADC
#define PIN_MOISTURE_SENSOR     A7 // ADC
#define PIN_RAIN_SENSOR         A2 // ADC, to fix

// ESP=2, Ardunio=7, Combo arduino=9
#define JARDIN_NRF24_PIN_CE 9
// ESP=4, Ardunio=8, Combo arduino=10
#define JARDIN_NRF24_PIN_CSN 10

#endif
////////////////////////////////////////////////////////////////// petaouchnok
#ifdef FOND
#define ENABLE_RADIO
#define JARDIN_BOX_ID           0x0a02  // fond
#define NB_LED 8
#define PIN_SENSOR_DHT          9
#define PIN_PHOTORESISTOR       A1 // ADC
// ESP=2, Ardunio=7, Combo arduino=9
#define JARDIN_NRF24_PIN_CE 7
// ESP=4, Ardunio=8, Combo arduino=10
#define JARDIN_NRF24_PIN_CSN 8
#endif
////////////////////////////////////////////////////////////////// esp-jack
#ifdef ESPJACK
#define JARDIN_BOX_ID           0x0a03  // ESP Jack

#define PIN_SENSOR_DHT          1
#define PIN_MOISTURE_SENSOR     A0 // ADC
// ESP=2, Ardunio=7, Combo arduino=9
#define JARDIN_NRF24_PIN_CE 2
// ESP=4, Ardunio=8, Combo arduino=10
#define JARDIN_NRF24_PIN_CSN 4

#endif


// #define PIN_RAINFALL_SENSOR     3 // for interrupt

#define RADIO_SPEED             2000
#define RADIO_PTT_INVERTED      false
#ifdef PIN_SENSOR_DHT
# define SENSOR_DHT_TYPE         DHT22   // DHT11, DHT21, DHT22  (AM2302), AM2321
#endif
#define RAINFALL_BOUNCE         0.011   // en litres

#ifdef NB_LED
# define LED_PIN 2
# define EVERY_NTH 1
# define BRIGHTNESS 1
# define NUM_PIXELS      NB_LED
#endif

#include <RH_NRF24.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

#ifdef PIN_SENSOR_DHT
# include "DHT.h"
#endif
#include <Adafruit_NeoPixel.h>

#include "jardin.h"

#ifdef ENABLE_RADIO
RH_NRF24 radio(JARDIN_NRF24_PIN_CE, JARDIN_NRF24_PIN_CSN);
#endif
#ifdef PIN_SENSOR_DHT
DHT dht(PIN_SENSOR_DHT, SENSOR_DHT_TYPE);
#endif
#ifdef NB_LED
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NB_LED, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

float rain_value = 0;
float moisture_value = 0;

void initialize_data_entry(jardin_data_abstract_t *data, jardin_event_type_t type)
{
#ifdef ENABLE_RADIO
  data->jardin_id = JARDIN_ID;
  data->box_type = BOX_TYPE_CAPTEUR;
  data->box_id = JARDIN_BOX_ID;
  data->event_type = type;
#endif
}
void transmit_data_entry(jardin_data_abstract_t *data, size_t length)
{
#ifdef ENABLE_RADIO
  radio.send((uint8_t *) data, length);
  if (radio.waitPacketSent(/*1000*/)) {
    // Serial.println(F("successfully sent message"));
  } else {
    Serial.println(F("failed sending message"));
  }
#endif
}

void send_uptime() {
  static unsigned long last_millis = 0;
  static unsigned short millis_cycle = 0;
  unsigned long milliseconds = millis();
  jardin_data_uptime_t data_uptime;

  initialize_data_entry((jardin_data_abstract_t *) &data_uptime, EVENT_TYPE_UPTIME);
  if (milliseconds < last_millis) {
    millis_cycle++;
    last_millis = milliseconds;
  }
  // a cycle is 4294967.296 seconds (so we will have 0.296s drifting every 49 days)
  data_uptime.uptime = (millis_cycle * 4294967) + (milliseconds / 1000);
  Serial.print(F("Sending event uptime: "));
  Serial.println(data_uptime.uptime);
  transmit_data_entry((jardin_data_abstract_t *) &data_uptime, sizeof(data_uptime));
}

#ifdef PIN_SENSOR_DHT

float temperature = 0;

// http://idehack.com/blog/tutoriel-arduino-mesure-de-temperature-et-dhumidite-avec-larduino/
void send_temperature() {
  jardin_data_mesure_temperature_t data_temp;

  initialize_data_entry((jardin_data_abstract_t *) &data_temp, EVENT_TYPE_TEMPERATURE);
  data_temp.sensor_type = SENSOR_DHT_TYPE;
  data_temp.temperature = temperature = dht.readTemperature();
  if (isnan(data_temp.temperature)) {
    Serial.println("Failed to read temperature from DHT sensor!");
  } else {
    Serial.println(F("Sending event temperature"));
    Serial.println(data_temp.temperature);
  }
  transmit_data_entry((jardin_data_abstract_t *) &data_temp, sizeof(data_temp));
}

float humidite = 0;

void send_humidity() {
  jardin_data_mesure_humidity_t data_humid;

  initialize_data_entry((jardin_data_abstract_t *) &data_humid, EVENT_TYPE_HUMIDITY);
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
#endif

float send_adc(const char *label, jardin_event_type_t type, int pin) {
  jardin_data_mesure_adc_t data;

  initialize_data_entry((jardin_data_abstract_t *) &data, type);

  data.value = analogRead(pin);
  transmit_data_entry((jardin_data_abstract_t *) &data, sizeof(data));
  Serial.print(F("Sending event "));
  Serial.print(label);
  Serial.print(" ");
  Serial.println(data.value);
  
  return data.value;
}

#ifdef PIN_RAINFALL_SENSOR
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

  initialize_data_entry((jardin_data_abstract_t *) &data_rainfall, EVENT_TYPE_RAINFALL);
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
#endif

#ifdef NB_LED
/**
 * 0=red, 65=vert, 85=turquoise
 */
uint32_t my_color(byte colorWheelPos, float d)
{
  uint32_t c1;

  d *= BRIGHTNESS;

  colorWheelPos = 255 - colorWheelPos;
  if(colorWheelPos < 85) {
    c1 = strip.Color(d * (255 - colorWheelPos * 3), 0, d * colorWheelPos * 3);
  } else if(colorWheelPos < 170) {
    colorWheelPos -= 85;
    c1 = strip.Color(0, d * (colorWheelPos * 3), d * (255 - colorWheelPos * 3));
  } else {
    colorWheelPos -= 170;
    c1 = strip.Color(d * (colorWheelPos * 3), d * (255 - colorWheelPos * 3), 0);
  }

  return c1;
}
#endif

#ifdef ENABLE_RADIO
void setup_radio()
{
  if (radio.init()) {
    Serial.println("NRF433 init successfully");
    if (!radio.setChannel(JARDIN_NRF24_CHANNEL)) {
      Serial.println("NRF433 setChannel failed");
    }
    if (!radio.setRF(RH_NRF24::DataRate250kbps, RH_NRF24::TransmitPower0dBm)) {
      Serial.println("NRF433 setRF failed");
    }
    Serial.println("NRF433 started");

  } else {
    Serial.println("NRF433 init failed");
  }
}
#endif

void setup()
{
  Serial.begin(9600);
  Serial.println(F("systeme-capteur init BEGIN"));

#ifdef ENABLE_RADIO
  setup_radio();
#endif
#ifdef NB_LED
  strip.begin();
#endif
#ifdef PIN_SENSOR_DHT
  dht.begin();
#endif
#ifdef PIN_RAINFALL_SENSOR
  init_rainfall();
#endif
  Serial.println(F("systeme-capteur init END"));
}

static short count = -1;
static short displayCount = -1;
#define INTERVAL 40

#ifdef NB_LED
void loop_led() {
  static uint8_t hue = 0;

  hue += 1;

  float speed = 4; // 4
  byte colorWheelPos = hue;
  float n_waves = 50;
  float sin_offset = 0;
  float divisor = 1;

  uint16_t idx, ridx;
  uint32_t c1;
  float pos;

  for (idx = 0; idx < NUM_PIXELS; idx++) {

    c1 = 0;
    ridx = NUM_PIXELS - idx;
    
    pos = (float)(displayCount % INTERVAL) / INTERVAL;
    if (displayCount >= INTERVAL) {
      if (displayCount < INTERVAL * 2) {
        if (pos < 0.2) {
          c1 = my_color(0, 0.5);
        } else if (pos > 0.4 && pos < 0.8) {
          c1 = my_color(65 - (temperature - 20) * 10, 1);
        }
      } else if (displayCount < INTERVAL * 3) {
        if (pos < 0.2) {
          c1 = my_color(128, 0.5);
        } else if (pos > 0.4 && pos < 0.8) {
          c1 = my_color(128 - (humidite - 50) * 128 / 50, 1);
        }
      }
#ifdef PIN_MOISTURE_SENSOR     
      else if (displayCount < INTERVAL * 4) {
        if (pos < 0.2) {
          c1 = my_color(50, 0.5);
        } else if (pos > 0.4 && pos < 0.8) {
          c1 = my_color(rain_value / 4, 1); 
        }
      }
#endif
#ifdef PIN_RAIN_SENSOR   
      else if (displayCount < INTERVAL * 5) {
        if (pos < 0.2) {
          c1 = my_color(170, 0.5);
        } else if (pos > 0.4 && pos < 0.8) {
          c1 = my_color(moisture_value / 4, 1); 
        }
      }
#endif
      else if (displayCount < INTERVAL * 6) {
      }
    }

//    ridx = NUM_PIXELS - idx;
//    float d;
//    d = (float)((ridx + hue) % 15) / 14;
//    c1 = my_color(colorWheelPos + (ridx * 120 / NUM_PIXELS), d);

    strip.setPixelColor(idx, c1);
  }
  strip.show();
}
#endif

void loop()
{
  // send_adc("photoresistance", MESURE_LIGHT, PIN_PHOTORESISTOR);
  // send_adc("moisture", MESURE_MOISTURE, PIN_MOISTURE_SENSOR);
  // send_adc("rain", MESURE_RAIN, PIN_RAIN_SENSOR);
  // send_temperature();
  // send_humidity();

  if (++count >= 100) {
    count = 0;
  }
  if (++displayCount >= 300) {
    displayCount = 0;
  }
  switch (count) {
  case 10:
    send_uptime();
    break;
  case 20:
#ifdef PIN_SENSOR_DHT
    send_temperature();
#endif
    break;
  case 30:
#ifdef PIN_SENSOR_DHT
    send_humidity();
#endif
    break;
  case 40:
#ifdef PIN_PHOTORESISTOR
    send_adc("photoresistance", EVENT_TYPE_LIGHT, PIN_PHOTORESISTOR);
#endif
    break;
  case 50:
#ifdef PIN_MOISTURE_SENSOR
    moisture_value = send_adc("moisture", EVENT_TYPE_MOISTURE, PIN_MOISTURE_SENSOR);
#endif
    break;
  case 60:
#ifdef PIN_RAIN_SENSOR
    rain_value = send_adc("rain", EVENT_TYPE_RAIN, PIN_RAIN_SENSOR);
#endif
    break;
  case 70:
#ifdef PIN_RAINFALL_SENSOR
    // rainfall_send();
#endif
    break;

  case 80:
#ifdef ENABLE_RADIO
    char str[] = "courgette";
    radio.send((uint8_t *) str, sizeof(str));
    radio.waitPacketSent();
#endif
    break;

  }
#ifdef NB_LED
  loop_led();
#endif
  delay(50);
}
