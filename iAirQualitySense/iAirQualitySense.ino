#include "esp32_iAQ.h"

esp32_iAQ *envir_sensor = NULL;
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

    uint64_t chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
    McChipID = String("ChipID:" + String((uint16_t)(chipid>>32), HEX));
    McChipID += String((uint32_t)chipid, HEX);
}

void loop() {
/*----------------------------------------------------------------------------
    loop        : get sensor value process it if required, write values to 
                  write to serial device

    Written by  : RaspiRepo
------------------------------------------------------------------------------*/  
    delay(2000);

    float temperature_f = envir_sensor->get_temperature();
    float humidity      = envir_sensor->get_humidity();
    float iAQ_Co2_level = envir_sensor->get_iaq();

    Serial.print(temperature_f);
    Serial.print("\t");
    Serial.print(humidity);
    Serial.print("\t");
    Serial.println(iAQ_Co2_level);
    
    //read sensor value & write out serial device 
//    println(McChipID + String(" temperature:") + String(temperature_f) 
//        + String(" Humidity:") + String(humidity) + String(" Co2Level:") 
//        + String(iAQ_Co2_level) + String(" ppm"));
}
