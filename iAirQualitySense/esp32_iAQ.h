#ifndef ESP32_IAQ_H_
#define ESP32_IAQ_H_

#define NULL  0x00


#define ENS210_I2C_DEVICE_ADDRESS  0x43
#define ENS210_SENS_RUN            0x21
#define ENS210_SENS_START          0x22
#define ENS210_DATA_REGISTER       0x30
#define iAQ_I2C_DEVICE_ADDRESS     0x5A

//Lighting sensor
#define AS3936_LIGHTING_SENSOR_ADDRESS 0x02
#define AS3935_AFE_REG                 0x00
#define AS3935_ESTIMATED_DISTANCE_REG  0x07


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
    void init_ENS210();
    void start_ENS210();

    //public functions to get current sensor's value
    float  get_temperature();
    float  get_humidity();
    uint16_t get_iaq();

    void lightning_sensor_start ();
    //return distance in km
    uint8_t lightning_sensor_reading();
    String get_afe_sensor_mode ();
    String get_lighting_event ();
    
  private:
    uint8_t deviceID;
    float  temperature;
    float  humidity;
    uint16_t  indoor_co2_level;
    uint16_t  resistance;

    uint8_t afe_setting;
    uint8_t est_distance;
    float g_lightning_miles;
    String afe_mode;
    String event_info;

    //micro controler chip ID
    String McChipID;

    uint8_t ens210_data_buf[6] = { 0x0,0x0,0x0,0x0,0x0,0x0 };
    uint8_t as3935_data_buf[9] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    uint8_t NoiseFloorThreshold[8] = {28, 45, 62, 78, 95, 112, 130, 146};

    //reading functions
    void read_from_ens210 ();
    uint8_t read_from_iAQc ();

    void write2device(uint8_t deviceID, uint8_t data);

};
#endif
