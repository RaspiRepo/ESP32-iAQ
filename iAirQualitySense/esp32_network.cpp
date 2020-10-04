#include <WiFi.h>

#include "esp32_network.h"

esp32_network::esp32_network(String ChipID)
/*----------------------------------------------------------------------------
    esp32_network  :  Manage all networking (communication) calls
    
    Written by     : RaspiRepo
------------------------------------------------------------------------------*/
{
    McChipID = ChipID;
}


esp32_network::~esp32_network()
/*----------------------------------------------------------------------------
    ~esp32_network  : Manage all networking (communication) calls
    
    Written by      : RaspiRepo
------------------------------------------------------------------------------*/
{

}


uint8_t esp32_network::init_wifi ()
/*----------------------------------------------------------------------------
    init_wifi   : Initialize Wifi Network
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
    WiFi.begin(SSID, WIFI_PASSWORD);

    uint8_t wait_count = 30;
  
    //Check for the connection
    while (WiFi.status() != WL_CONNECTED && --wait_count) { 
        delay(1000);
    }
    return wait_count;
}


int esp32_network::send_iAQ_influxDB (float temparature, float humidity, uint16_t iAQ)
/*----------------------------------------------------------------------------
    send_iAQ_influxDB
                : Construct HTTP csv payload with current temp, Rh and iAQ value 
                  and send to IOT Endpoint (influxDB/telegraf/chronograf/kepacitor)
                  called TICK, check more infor here
                  https://github.com/RaspiRepo/jetsonNano-Tick
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
    //set default network unreachable 
    int16_t httpResponseCode = 503;

    //Check WiFi connection status
    if(WiFi.status() == WL_CONNECTED) {

        String message = String(IOT_INFLUX_DB) + String(",machine=") + String(McChipID) + String(",type=iAQ temperature=") 
                        + String(temparature) + String(",rel_humidity=") + String(humidity) 
                        + String(",iAQ=") + String(iAQ);

        http.begin(String(IOT_ENDPOINT));
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        httpResponseCode = http.POST(message);
        http.end();
    }

    return httpResponseCode;
}
