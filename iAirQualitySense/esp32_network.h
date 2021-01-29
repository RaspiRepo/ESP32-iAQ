#ifndef ESP32_NETWORK_H_
#define ESP32_NETWORK_H_
#include <WiFiMulti.h>
#include <HTTPClient.h>

class esp32_network
/*----------------------------------------------------------------------------
    esp32_network  
                : ESP32 Wifi networking setup and send/recv sensor value to
                  IOT endpoints.
                  
    Written by : RaspiRepo
------------------------------------------------------------------------------*/
{
  public:
    esp32_network(String McChipID);
    ~esp32_network();

    uint8_t init_wifi ();

    //to send sensor value to iot endpointstry 
    int send_iAQ_influxDB (float temparature, float humidity, uint16_t iAQ);
    int get_outdoor_weather(OUTDOOR_WEATHER *weather);
    int send_weather_report (WEATHER *weather);
    
  private:
    HTTPClient http;
    String McChipID;
    WiFiMulti wifiMulti;    
};

#endif
