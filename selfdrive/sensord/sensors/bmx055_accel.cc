#include "common/swaglog.h"

#include "bmx055_accel.hpp"


BMX055_Accel::BMX055_Accel(I2CBus *bus) : I2CSensor(bus) {}

int BMX055_Accel::init(){
  int ret = 0;
  uint8_t buffer[1];

  ret = read_register(BMX055_ACCEL_I2C_REG_ID, buffer, 1);
  if(ret < 0){
    LOGE("Reading chip ID failed: %d", ret);
    goto fail;
  }

  if(buffer[0] != BMX055_ACCEL_CHIP_ID){
    LOGE("Chip ID wrong. Got: %d, Expected %d", buffer[0], BMX055_ACCEL_CHIP_ID);
    ret = -1;
    goto fail;
  }

fail:
  return ret;
}

void BMX055_Accel::get_event(cereal::SensorEventData::Builder &event){
  event.setSource(cereal::SensorEventData::SensorSource::ANDROID);
  event.setVersion(1);
  event.setSensor(SENSOR_ACCELEROMETER);
  event.setType(SENSOR_TYPE_ACCELEROMETER);

  // event.setTimestamp(data.timestamp);
}
