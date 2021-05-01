// Définitions communes aux modules du jardin

#include <time.h>

// structures de messages 433 MGz

// l'ID jardin est commun à tous les modules du jardin
// il sert de signature pour filtrer les messages radio
// (32 bits non signés)
#define JARDIN_ID       0x8a56b243

/* #define JARDIN_BOX_TYPE_CUVE 1 */
/* #define JARDIN_BOX_TYPE_VANNE 2 */
/* #define JARDIN_BOX_TYPE_CAPTEUR 3 */

/*
typedef enum {
  CUVE          = 1,
  VANNE         = 2,
  CAPTEUR       = 3,
} jardin_box_type_t;
*/
#define BOX_TYPE_CUVE 1
#define BOX_TYPE_VANNE 2
#define BOX_TYPE_CAPTEUR 3
#define BOX_TYPE_RELAIS 4
typedef uint16_t jardin_box_type_t;

#define JARDIN_BOX_RELAIS_ESP_ID    0x0901
#define JARDIN_BOX_HEUCHERE_ID      0x0a01
#define JARDIN_BOX_FOND_ID          0x0a02

/*
typedef enum {
  UPTIME                = 1,    // simple uptime
  DATE_TIME             = 2,
  MESURE_WIFI_SIGNAL    = 3,
  MESURE_TEMPERATURE    = 101,
  MESURE_HUMIDITY       = 102,
  MESURE_PRESSURE       = 103,
  MESURE_LIGHT          = 104,
  MESURE_MOISTURE       = 105,
  MESURE_RAINFALL       = 106,
  MESURE_RAIN           = 107,
  EVENT_PUMP            = 301,
  EVENT_VALVE           = 302,
} jardin_event_type_t;
*/

#define EVENT_TYPE_UPTIME       1
#define EVENT_TYPE_DATE_TIME    2
#define EVENT_TYPE_WIFI_SIGNAL  3
#define EVENT_TYPE_TEMPERATURE  0x0065
#define EVENT_TYPE_HUMIDITY     0x0066
#define EVENT_TYPE_PRESSURE     0x0067
#define EVENT_TYPE_LIGHT        0x0068
#define EVENT_TYPE_MOISTURE     0x0069
#define EVENT_TYPE_RAINFALL     0x006a
#define EVENT_TYPE_RAIN         0x006b
#define EVENT_TYPE_PUMP         0x012d
#define EVENT_TYPE_VALVE        0x012e

typedef uint16_t jardin_event_type_t;


typedef struct {
  uint32_t              jardin_id;
  uint16_t              box_type;
  uint16_t              box_id;
  uint16_t              event_type;

} jardin_data_abstract_t;


typedef struct {
  jardin_data_abstract_t        event;
  uint32_t                      uptime;

} jardin_data_uptime_t;

typedef struct {
  jardin_data_abstract_t        event;
  time_t                        timestamp;

} jardin_data_datetime_t;

typedef struct {
  jardin_data_abstract_t        event;
  int16_t                       strength;

} jardin_data_signal_strength_t;

typedef struct {
  jardin_data_abstract_t        event;
  float                         temperature;    // Celsius
  uint8_t                       sensor_type; // 11, 12, 22, 21 (see DHT.h)

} jardin_data_mesure_temperature_t;

typedef struct {
  jardin_data_abstract_t        event;
  float                         humidity;
  uint8_t                       sensor_type; // 11, 12, 22, 21 (see DHT.h)

} jardin_data_mesure_humidity_t;

typedef struct {
  jardin_data_abstract_t        event;
  uint16_t                      value;
} jardin_data_mesure_adc_t;

/* typedef struct { */
/*   jardin_data_abstract_t        event; */
/*   unsigned short                light; // from ADC */
/* } jardin_data_mesure_light_t; */
/* typedef struct { */
/*   jardin_data_abstract_t        event; */
/*   unsigned short                moisture; // from ADC */
/* } jardin_data_mesure_moisture_t; */

typedef struct {
  jardin_data_abstract_t        event;
  float                         rainfall_delta;
  float                         rainfall_acc;
  float                         rainfall_5minutes[12];
  float                         rainfall_hours[24];
  float                         rainfall_days[7];
} jardin_data_mesure_rainfall_t;
