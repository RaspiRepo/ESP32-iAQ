
#include "esp32_network.h"
#include "esp32_iAQ.h"

esp32_iAQ     *envir_sensor = NULL;
esp32_network *network      = NULL;

String McChipID = "";

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
    envir_sensor->init_ENS210(ENS210_I2C_DEVICE_ADDRESS);
    Serial.begin(115200);   

    //The chip ID is essentially its MAC address(length: 6 bytes).
    uint64_t chipid=ESP.getEfuseMac();
    McChipID = String("ChipID:" + String((uint16_t)(chipid>>32), HEX));
    McChipID += String((uint32_t)chipid, HEX);
    Serial.println(McChipID);

   //create network instance 
   network = new esp32_network(McChipID);
   uint8_t status = network->init_wifi();
   Serial.println("Wifi Connection status code " + String(status));
}

void loop() {
/*----------------------------------------------------------------------------
    loop        : get sensor value process it if required, write values to 
                  write to serial device

    Written by  : RaspiRepo
------------------------------------------------------------------------------*/  
    delay(2000);

    if (envir_sensor) {
        float temperature_f = envir_sensor->get_temperature();
        float humidity      = envir_sensor->get_humidity();
        uint16_t iAQ_Co2_level = envir_sensor->get_iaq();
  
        Serial.print(temperature_f);
        Serial.print("\t");
        Serial.print(humidity);
        Serial.print("\t");
        Serial.println(iAQ_Co2_level);

        //send current sense value to remote endpoint 
        if (network) {
            network->send_iAQ_influxDB(temperature_f, humidity, iAQ_Co2_level);
        }
    }
        
    // read sensor value & write out serial device 
    println(McChipID + String(" temperature:") + String(temperature_f) 
        + String(" Humidity:") + String(humidity) + String(" Co2Level:") 
        + String(iAQ_Co2_level) + String(" ppm"));
}
