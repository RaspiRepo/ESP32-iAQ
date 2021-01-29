#ifndef WEATHER_H_
#define WEATHER_H_

#define INDOOR_WEATHER_CHECK_RATE    20  //20s 
#define OUTDOOR_WEATHER_CHECK_RATE   60  //1 minute
#define DATA_COLLECTION_COMPLETED   100
#define DATA_COLLECTION_IN_PROGRESS 101
#define DATA_COLLECTION_ERROR       103


#define WIFI_SSID             "SSID"
#define WIFI_PASSWORD         "PSW"
#define IOT_ENDPOINT          "http://192.168.0.115:8086/write?db=weather"
#define OPEN_WEATHER_API_ID   "OPENWEATHERMAP_APP_ID"
#define OPEN_WATHER_URL       "http://api.openweathermap.org/data/2.5/weather?lat=36.409644940298534&lon=-142.11134718069599&appid=OPENWEATHERMAP_APP_ID&units=imperial"

#define IOT_INFLUX_DB   "weather"
#define LED_PIN         27 //GPIO27
 
typedef struct ow_main {
  float temparature;
  float feels_like;
  float min;
  float max;
  int pressure;
  int humidity;
} OPEN_WEATHER_MAIN;

typedef struct ow_wind {
  float speed;
  int deg;
} OPEN_WEATHER_WIND;

typedef struct ow_coord {
  float lon;
  float lat;
} OPEN_WEATHER_COORD;

typedef struct ow_sys {
  const char *country;
  long sunrise;
  long sunset;  
} OPEN_WEATHER_SYS;

typedef struct _indoor {
    uint8_t deviceID;
    float  temparature;
    float  humidity;
    uint16_t  indoor_co2_level;
    uint16_t  resistance;

    uint8_t afe_setting;
    uint8_t est_distance;
    float g_lightning_miles;
    String afe_mode;
    String event_info;
} INDOOR_WEATHER;


typedef struct open_weather {
  int timezone;
  int id;  
  const char *city_name;
  long visibility;

  const char *main;
  const char *description;
  OPEN_WEATHER_MAIN curr_temp;
  OPEN_WEATHER_WIND wind;
  OPEN_WEATHER_COORD coord;
  OPEN_WEATHER_SYS sys;
} OUTDOOR_WEATHER;

typedef struct _Weather {
  INDOOR_WEATHER  indoor;
  OUTDOOR_WEATHER outdoor;
} WEATHER;

#endif
