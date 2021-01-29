#include<Wire.h>
#include "esp32_iAQ.h"

esp32_iAQ::esp32_iAQ()
/*----------------------------------------------------------------------------
    esp32_iAQ  : class constructor to set defaults 
    
    Written by : RaspiRepo
------------------------------------------------------------------------------*/
{
  Wire.begin();
  Wire.setClock(100000);
  
  temperature      = 0.0;
  humidity         = 0.0;
  indoor_co2_level = 0;
  resistance       = 0; 
  afe_mode = " Indoor ";
  event_info = "";
}

esp32_iAQ::~esp32_iAQ()
/*----------------------------------------------------------------------------
    ~esp32_iAQ  : class destructor to clean if any alloctated resource
    
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
}

void esp32_iAQ::init_ENS210()
/*----------------------------------------------------------------------------
    init_ENS210 : Initialize and start Temp/Hum senssor ENS210. Refer Datasheet 
                  for more details 
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
  Wire.beginTransmission(ENS210_I2C_DEVICE_ADDRESS);
  Wire.write((uint8_t)ENS210_SENS_RUN);
  Wire.write((uint8_t)0x0);
  Wire.write((uint8_t)0x0);
  Wire.endTransmission();   
}

void esp32_iAQ::start_ENS210()
/*----------------------------------------------------------------------------
    init_ENS210 : Initialize and start Temp/Hum senssor ENS210. Refer Datasheet 
                  for more details 
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
  Wire.beginTransmission(ENS210_I2C_DEVICE_ADDRESS);
  Wire.write((uint8_t)ENS210_SENS_START);
  Wire.write((uint8_t)0x03);
  Wire.endTransmission();
}


float esp32_iAQ::get_temperature()
/*----------------------------------------------------------------------------
    get_temperature
                : return current reading of temperature in unit F
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
  read_from_ens210();
  return (temperature);
}

float esp32_iAQ::get_humidity()
/*----------------------------------------------------------------------------
    get_humidity
                : return current reading of humidity
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
  //Relative humidity operating range (0% to 100%) %RH
  return (humidity);
}

uint16_t esp32_iAQ::get_iaq()
/*----------------------------------------------------------------------------
    get_iaq     : return current reading of indoor Air Quality (Co2 level) in
                  ppm
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
   uint8_t status = read_from_iAQc ();
   switch (status) {
   case 0x10:
       //println(String("iAQ sensor RUNNING ") + String(" Status ") + String(status, HEX));   
       break;
   case 0x01:
//      println("iAQ sensor BUSY");
      break;
   case 0x80:
     indoor_co2_level = 0;
//     println("iAQ sensor ERROR");
     break;
  }
  return (indoor_co2_level);
}


void esp32_iAQ::write2device(uint8_t deviceID, uint8_t data)
/*----------------------------------------------------------------------------
    write2device
                : write data bytes to I2C device
    deviceID    : Sensor I2C address 
    data        : byte to be sent to sensor like command

    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
  Wire.beginTransmission(deviceID);
  Wire.write(data);
  Wire.endTransmission();

}


void esp32_iAQ::read_from_ens210 ()
/*----------------------------------------------------------------------------
    read_from_ens210
                : Reading current Temperature, humidity from sensor  
                  calculate Kelvin into fahrenheit(F) and Humidity. Refer Datasheet
                  sensor ENS210 for more details of reponse data format

    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
  uint8_t bytes_ret = 0; 
  uint8_t *data_buf = ens210_data_buf; 

  //single shot reading
  start_ENS210();

  //set data register of sensor for reading
  write2device(ENS210_I2C_DEVICE_ADDRESS, ENS210_DATA_REGISTER);

  // wait for bytes from slave device and add received bytes to buffer
  //max 6 bytes request
  Wire.requestFrom(ENS210_I2C_DEVICE_ADDRESS, 0x6); 
  while (Wire.available() && bytes_ret < 0x07) { 
    *data_buf = Wire.read();   
    ++data_buf;
    ++bytes_ret;
  }
  
  int32_t t_val = (ens210_data_buf[2]<<16) + (ens210_data_buf[1]<<8) + (ens210_data_buf[0]<<0);
  int32_t h_val = (ens210_data_buf[5]<<16) + (ens210_data_buf[4]<<8) + (ens210_data_buf[3]<<0);

  int32_t t_data = (t_val>>0 ) & 0xffff;

  float TinK = (float)t_data / 64; // Temperature in Kelvin
  float TinC = (float)(TinK - 273.15f); // Temperature in Celsius
  
  temperature = (float)(TinC * 1.8f + 32.0f); // Temperature in Fahrenheit

  uint32_t h_data = (h_val>>0 ) & 0xffff;
  /*
  uint32_t h_valid= (h_val>>16) & 0x1;
  uint32_t h_crc = (h_val>>17) & 0x7f;
  uint32_t h_payl = (h_val>>0 ) & 0x1ffff;
  */
   humidity = (float)((float)h_data/512.0f); // relative humidity (in %)
}

uint8_t esp32_iAQ::read_from_iAQc ()
/*----------------------------------------------------------------------------
    read_from_iAQc
                : read current indoor Air Quality from sensor (co2 level in ppm)
                  as per Datasheet warmup time 5 minutes from cold start.
                  Return status code 

    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
    uint8_t bytes_ret = 0;  
    uint8_t aiq_data_buf[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    uint8_t *data_buf = aiq_data_buf;
  
    //requesr 9 bytes only 3 bytes used here. Details on Datasheet
    Wire.requestFrom(iAQ_I2C_DEVICE_ADDRESS, 0x09);
    while (Wire.available() && bytes_ret < 0x0a) { 
        *data_buf = Wire.read();   
        ++data_buf;
        ++bytes_ret;
    }
    uint8_t  status = (uint8_t)aiq_data_buf[3];
    if (0x00 == status) {
        indoor_co2_level = (uint16_t) ((aiq_data_buf[0] << 8) + aiq_data_buf[1]);
        //uint32_t resistance = (aiq_data_buf[4] << 16) | (aiq_data_buf[5] << 8) | aiq_data_buf[6];
    }
    return status;
}



void esp32_iAQ::lightning_sensor_start ()
/*----------------------------------------------------------------------------
    lightning_sensor_start:
                          Initialize Lightning Sensor AS3935
    Written By          : RaspiRepo (mariya.k@gmail.com)
                          https://github.com/RaspiRepo
    Address             : Mountain View, CA 94040
    Date                : May 2017
------------------------------------------------------------------------------*/
{
  Wire.beginTransmission(AS3936_LIGHTING_SENSOR_ADDRESS);
  Wire.write((uint8_t)0x3c);
  Wire.write((uint8_t)0x96);
  Wire.write((uint8_t)0x96);
  Wire.endTransmission(); 
}


String esp32_iAQ::get_afe_sensor_mode ()
/*----------------------------------------------------------------------------
    get_afe_sensor_mode:
                          AS3935 Franklin Lightning Sensor IC: Lightning sensor
                          warns of lightning storm activity within a radius
                          of 40km
    Written By          : RaspiRepo (mariya.k@gmail.com)
                          https://github.com/RaspiRepo
    Address             : Mountain View, CA 94040
    Date                : May 2017
------------------------------------------------------------------------------*/
{   
  return afe_mode;
}

String esp32_iAQ::get_lighting_event ()
/*----------------------------------------------------------------------------
    get_lighting_event:
                          AS3935 Franklin Lightning Sensor IC: Lightning sensor
                          warns of lightning storm activity within a radius
                          of 40km
    Written By          : RaspiRepo (mariya.k@gmail.com)
                          https://github.com/RaspiRepo
    Address             : Mountain View, CA 94040
    Date                : May 2017
------------------------------------------------------------------------------*/
{   
  return event_info;
}


uint8_t esp32_iAQ::lightning_sensor_reading ()
/*----------------------------------------------------------------------------
    lightning5_sensor_reading:
                          AS3935 Franklin Lightning Sensor IC: Lightning sensor
                          warns of lightning storm activity within a radius
                          of 40km
    Written By          : RaspiRepo (mariya.k@gmail.com)
                          https://github.com/RaspiRepo
    Address             : Mountain View, CA 94040
    Date                : May 2017
------------------------------------------------------------------------------*/
{
    //Statistical Distance Estimation:
    //The AS3935 generates an assessment of the estimated distance to the head of an approaching storm.
    //This assessment is done based on statistical calculation. The energy of the single event (lightning) 
    //provided by the Energy Calculation block is stored in an internal memory, together with timing
    //information, in the AS3935. The events stored in the memory are then correlated with a look-up table by the statistical
    //distance estimation block, which provides a final estimation of the distance to the head of the storm. 
    //The algorithm automatically deletes events, which are older than a certain time. R7=0x01 means that 
    //the storm is right overhead, while R7=0x3F is displayed when the storm is out of range.
    //This algorithm is hardwired and not accessible from outside.
    //The estimated distance is directly represented in km in the register REG0x07[5:0] (binary encoded).
    //The distance estimation can change also if no new event triggers the AS3935, as older events can be purged.

    uint8_t bytes_ret = 0; 
    uint8_t *data_buf = as3935_data_buf; 
    event_info = "";
      
    //Issue read command
    write2device(AS3936_LIGHTING_SENSOR_ADDRESS, AS3935_AFE_REG);
        
    //requesr 9 bytes only 3 bytes used here. Details on Datasheet
    Wire.requestFrom(AS3936_LIGHTING_SENSOR_ADDRESS, 0x08);
    
    // wait for bytes from slave device and add received bytes to buffer
    //max 6 bytes request
    while (Wire.available() && bytes_ret < 0x08) { 
      *data_buf = Wire.read();
      ++data_buf;
      ++bytes_ret;
    }
    
    //REG0x00[5:1]   - 0x24 - means indoor :  10010
    //Outdoor 01110  - 0x1c = means outdoor
    afe_setting = as3935_data_buf[0] & 0x3E;

    if (afe_setting == 0x1c) {
      afe_mode = " Outdoor ";
    }
    
    uint8_t noise_floor_level = NoiseFloorThreshold[(as3935_data_buf[1] & 0x70 >> 4)];
    uint8_t detection_efficiency = as3935_data_buf[2] & 0x0F;

    uint32_t energy = (as3935_data_buf[6] & 0x1f) << 0x08;
    energy = energy | (as3935_data_buf[5] << 0x08);
    energy = energy | (uint8_t)(as3935_data_buf[4]);

    uint8_t distance = as3935_data_buf[7] & 0x3f;

    //even information deom sensor
    uint8_t intrpt_event = as3935_data_buf[3] & 0x0f;
    if (intrpt_event & 0x01) {
      event_info = " Noise level too high ";
    } else if (intrpt_event & 0x04) {
      event_info = " Disturber detected ";
    } else if (intrpt_event & 0x08) {
      event_info = " Lightning interrupt ";
    }
    
//    Serial.print(" Mode ");
//    Serial.print(afe_setting);
//    Serial.print(" NseFLvel ");
//    Serial.print(noise_floor_level);
//    Serial.print(" DF ");
//    Serial.print(detection_efficiency);   
//    Serial.print(" ERGY ");
//    Serial.print(energy);   
//    Serial.print(" Distance ");
//    if (distance == 0x3f) {
//      Serial.print(" Out of range ");
//    } else if (distance == 0x01) {
//      Serial.print("  Storm is Overhead ");
//    } else {
//      Serial.print(distance);
//    }
//    

    return distance;

} 
