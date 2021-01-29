#include <WiFi.h>
#include <ArduinoJson.h>
#include "weather.h"
#include "esp32_network.h"


#define USE_SERIAL Serial
DynamicJsonDocument doc(600);
    
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
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
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

        String message = String(IOT_INFLUX_DB) + String(",machine=") 
                        + String(McChipID) + String(",type=iAQ temperature=") 
                        + String(temparature) + String(",rel_humidity=") 
                        + String(humidity) 
                        + String(",iAQ=") + String(iAQ);
        USE_SERIAL.println(message);

        http.begin(String(IOT_ENDPOINT));
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        httpResponseCode = http.POST(message);
        http.end();
    }

    return httpResponseCode;
}



int esp32_network::send_weather_report (WEATHER *weather)
/*----------------------------------------------------------------------------
    send_weather_report
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
        String message =  String(" type=weather ") + String("temperature=") + String(weather->indoor.temparature) 
                        + String(",rel_humidity=") + String(weather->indoor.humidity) 
                        + String(",iAQ=") + String(weather->indoor.indoor_co2_level) 
                        + String(",out_temp=") + String(weather->outdoor.curr_temp.temparature) 
                        + String(",out_humidity=") + String(weather->outdoor.curr_temp.humidity) 
                        + String(",pressure=") + String(weather->outdoor.curr_temp.pressure)
                        + String(",min=") + String(weather->outdoor.curr_temp.min)
                        + String(",max=") + String(weather->outdoor.curr_temp.max)
                        + String(",feels_like=") + String(weather->outdoor.curr_temp.feels_like)                        
                        + String(",wind_speed=") + String(weather->outdoor.wind.speed)
                        + String(",wind_dir=") + String(weather->outdoor.wind.deg)  
                        + String(",visibility=") + String((weather->outdoor.visibility / 1069));
                        /*
                        + String(",sunrise=") + String(weather->outdoor.sys.sunrise)
                        + String(",sunset=") + String(weather->outdoor.sys.sunset)
                        + String(",city_name=") + String(weather->outdoor.city_name)   
                        + String(",country=") + String(weather->outdoor.sys.country) + String("");
                        */
        USE_SERIAL.println(message);
        
        http.begin(String(IOT_ENDPOINT));
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        httpResponseCode = http.POST(message);
        http.end();
    }

    return httpResponseCode;
}

int esp32_network::get_outdoor_weather (OUTDOOR_WEATHER *weather)
/*----------------------------------------------------------------------------
    get_outdoor_weather
                : Get current outdoor weather information from openweathermap 
                  API
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
    int16_t httpResponseCode = 503;

    // wait for WiFi connection
    if((wifiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;
        
        http.begin(OPEN_WATHER_URL);

        // start connection and send HTTP header
        httpResponseCode = http.GET();

        // httpCode will be negative on error
        if(httpResponseCode > 0) {

            // response received success
            if(httpResponseCode == HTTP_CODE_OK) {
    
                memset(weather, 0, sizeof(OUTDOOR_WEATHER));
              
                String payload = http.getString();                
                deserializeJson(doc, payload);
                JsonObject obj = doc.as<JsonObject>();

                weather->curr_temp.temparature = obj["main"]["temp"];
                weather->curr_temp.humidity = obj["main"]["humidity"];
                weather->curr_temp.pressure = obj["main"]["pressure"];
                //weather->descrp = obj["weather"]["id"];

                weather->curr_temp.feels_like = obj["main"]["feels_like"];
                weather->curr_temp.min = obj["main"]["temp_min"];
                weather->curr_temp.max = obj["main"]["temp_max"];
                
                weather->wind.speed = obj["wind"]["speed"];
                weather->wind.deg = obj["wind"]["deg"];

                weather->coord.lon = obj["coord"]["lon"];
                weather->coord.lat = obj["coord"]["lat"];
                weather->sys.sunrise = obj["sys"]["sunrise"];
                weather->sys.sunset = obj["sys"]["sunset"];
                weather->sys.country = obj["sys"]["country"];
                weather->city_name = obj["name"];
                weather->visibility = obj["visibility"];
                                  
            } else {
              USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpResponseCode);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        }

        http.end();
    }

    return httpResponseCode;
}
