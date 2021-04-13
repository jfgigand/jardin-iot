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

typedef enum {
  CUVE          = 1,
  VANNE         = 2,
  CAPTEUR       = 3,
} jardin_box_type_t;

typedef enum {
  UPTIME                = 1,    // simple uptime
  DATE_TIME             = 2,
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



typedef struct {
  unsigned long         jardin_id;
  jardin_box_type_t     box_type;
  unsigned short        box_id;
  jardin_event_type_t   event_type;

} jardin_data_abstract_t;


typedef struct {
  jardin_data_abstract_t        event;
  unsigned long                 uptime;

} jardin_data_uptime_t;

typedef struct {
  jardin_data_abstract_t        event;
  time_t                        timestamp;

} jardin_data_datetime_t;

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
  unsigned short                value;
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
