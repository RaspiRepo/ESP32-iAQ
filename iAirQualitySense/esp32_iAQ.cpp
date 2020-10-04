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
}

esp32_iAQ::~esp32_iAQ()
/*----------------------------------------------------------------------------
    ~esp32_iAQ  : class destructor to clean if any alloctated resource
    
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
}

void esp32_iAQ::init_ENS210(uint8_t deviceID)
/*----------------------------------------------------------------------------
    init_ENS210 : Initialize and start Temp/Hum senssor ENS210. Refer Datasheet 
                  for more details 
                  
    Written by  : RaspiRepo
------------------------------------------------------------------------------*/
{
  write2device(deviceID, (uint8_t)ENS210_SENS_RUN);
  write2device(deviceID, (uint8_t)0x3);
  write2device(deviceID, (uint8_t)0x3);
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

  //set data register of sensor
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
