/*
 * i2c_sensors.h
 *
 *  Created on: 3 באפר 2017
 *      Author: stoledo
 */

#ifndef I2C_SENSORS_H_
#define I2C_SENSORS_H_

void i2cSensorsInit();
uint8_t mpu9150WhoAmI(uint8_t* value, uint8_t address0);

#endif /* I2C_SENSORS_H_ */
