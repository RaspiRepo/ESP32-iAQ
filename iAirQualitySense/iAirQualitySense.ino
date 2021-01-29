
#include "weather.h"
#include "esp32_network.h"
#include "esp32_iAQ.h"

esp32_iAQ     *envir_sensor = NULL;
esp32_network *network      = NULL;

String McChipID = "";
WEATHER weather;
    
void println (String message) 
/*----------------------------------------------------------------------------
    println     : send message string to serial out 
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
    Serial.println(message);

}


void setup () 
/*----------------------------------------------------------------------------
    setup       : one time device setup calls
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{   
    envir_sensor = new esp32_iAQ();
    envir_sensor->init_ENS210();
    Serial.begin(115200);   
    
    pinMode(LED_PIN, OUTPUT);

    //The chip ID is essentially its MAC address(length: 6 bytes).
    uint64_t chipid=ESP.getEfuseMac();
    McChipID = String("" + String((uint16_t)(chipid>>32), HEX));
    McChipID += String((uint32_t)chipid, HEX);
    Serial.println(McChipID);

    envir_sensor->lightning_sensor_start();

   //create network instance 
   network = new esp32_network(McChipID);
   uint8_t status = network->init_wifi();
   Serial.println("Wifi Connection status code " + String(status));
   memset(&weather, 0, sizeof(OUTDOOR_WEATHER));

   delay(1000);
   get_outdoor_weather();
}


int openapi_rate = OUTDOOR_WEATHER_CHECK_RATE; //per minute
int sensor_rate  = INDOOR_WEATHER_CHECK_RATE; //20 secs rate

void get_indoor_weather ()
/*----------------------------------------------------------------------------
    get_indoor_weather
                : Collect indoor temp/Rh/iAQ from sensor

    Written by  : RaspiRepo
------------------------------------------------------------------------------*/  
{
      if (envir_sensor) {
        float temperature_f = envir_sensor->get_temperature();
        float humidity      = envir_sensor->get_humidity();
        uint16_t iAQ_Co2_level = envir_sensor->get_iaq();

        uint8_t distance = envir_sensor->lightning_sensor_reading();
        String dist_info = String (distance) + String( " km");
        if (distance == 0x3f) {
            dist_info = String(" Out of range ");
        } else if (distance == 0x01) {
            dist_info = String("  Storm is Overhead ");
        }

        String opr_mode = envir_sensor->get_afe_sensor_mode();
        String event = envir_sensor->get_lighting_event();
       
        weather.indoor.temparature = temperature_f;
        weather.indoor.humidity = humidity;
        weather.indoor.indoor_co2_level = iAQ_Co2_level;
    }
}


int get_outdoor_weather () 
/*----------------------------------------------------------------------------
    get_outdoor_weather:
                 get local out door weather information from openweathermap
                 system.

    Written by  : RaspiRepo
------------------------------------------------------------------------------*/ 
{
  int ret_code = 0;
  
  //get outdoor weather information from open weathermap API
  if (network) {
    ret_code = network->get_outdoor_weather(&weather.outdoor);
    if (ret_code == 200) {   
      ret_code = 0;
    }
  }
  
  return ret_code;
}



void loop() {
/*----------------------------------------------------------------------------
    loop        : get sensor value process it if required, write values to 
                  write to serial device

    Written by  : RaspiRepo
------------------------------------------------------------------------------*/  
    //digitalWrite(LED_PIN, LOW);   // GET /H turns the LED off
    delay(500);
    //digitalWrite(LED_PIN, HIGH);   // GET /H turns the LED on    
    delay(500);
    int ret_val = DATA_COLLECTION_IN_PROGRESS;
    
    --openapi_rate;
    --sensor_rate;
    if (openapi_rate <= 0) {
      openapi_rate = OUTDOOR_WEATHER_CHECK_RATE;
      ret_val = get_outdoor_weather();      
      if (ret_val == 200) {   
        ret_val = DATA_COLLECTION_COMPLETED;
      }
    } 

    if (sensor_rate <= 0) {
      sensor_rate = INDOOR_WEATHER_CHECK_RATE;
      get_indoor_weather();
      ret_val = DATA_COLLECTION_COMPLETED;
    }
    
    if (ret_val == DATA_COLLECTION_COMPLETED && weather.indoor.temparature > 0.0f) {           
      ret_val = network->send_weather_report(&weather);
    }

}
