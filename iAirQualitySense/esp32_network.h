#ifndef ESP32_NETWORK_H_
#define ESP32_NETWORK_H_

#include <HTTPClient.h>

#define NULL            0x00
#define SSID            "SSID"
#define WIFI_PASSWORD   "PSW00001"
#define IOT_ENDPOINT    "http://192.168.0.115:8086/write?db=indoorSense"
#define IOT_INFLUX_DB   "indoorSense"



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

    //to send sensor value to iot endpoints
    int send_iAQ_influxDB (float temparature, float humidity, uint16_t iAQ);
  private:
    HTTPClient http;
    String McChipID;
    
};

#endif
