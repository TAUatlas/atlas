/*
 * i2c_sensors.h
 *
 *  Created on: 3 ×‘×�×¤×¨ 2017
 *      Author: stoledo
 */

#ifndef I2C_SENSORS_H_
#define I2C_SENSORS_H_

void i2cSensorsInit();
uint8_t mpu9150WhoAmI(uint8_t* value, uint8_t address0);
void bmi160Setup_GRange(const uint8_t* sensorData);
void bmi160Setup_TicksFactor(const uint32_t* sensorData);

#endif /* I2C_SENSORS_H_ */
