#ifdef SENSOR_H
#define SENSOR_H


// bmp208
extern void sensor_init(void);
extern float sensor_read_temperature(void);
extern float sensor_read_humidity(void);
extern float sensor_read_pressure(void);

#endif