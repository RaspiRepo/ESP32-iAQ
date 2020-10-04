#ifndef ESP32_IAQ_H_
#define ESP32_IAQ_H_

#define NULL  0x00


#define ENS210_I2C_DEVICE_ADDRESS  0x43
#define ENS210_SENS_RUN            0x21
#define ENS210_DATA_REGISTER       0x30

#define iAQ_I2C_DEVICE_ADDRESS     0x5A

class esp32_iAQ
/*----------------------------------------------------------------------------
    esp32_iAQ  : Sensor class defination, methods init, start and read read 
                 I2c sensor ENS210, iAQ-Core 
    
    Written by : RaspiRepo
------------------------------------------------------------------------------*/
{
  public:
    esp32_iAQ();
    ~esp32_iAQ();

    //to issue start command to ENS210
    void init_ENS210(uint8_t deviceID);

    //public functions to get current sensor's value
    float  get_temperature();
    float  get_humidity();
    uint16_t get_iaq();

  private:
    uint8_t deviceID;
    float  temperature;
    float  humidity;
    uint16_t  indoor_co2_level;
    uint16_t  resistance;

    //micro controler chip ID
    String McChipID;

    uint8_t ens210_data_buf[6] = { 0x0,0x0,0x0,0x0,0x0,0x0 };

    //reading functions
    void read_from_ens210 ();
    uint8_t read_from_iAQc ();

    void write2device(uint8_t deviceID, uint8_t data);

};
#endif
