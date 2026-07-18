// Data structs
typedef struct __attribute__((packed)){
    float accel_x, accel_y, accel_z; //units: g
    float gyro_x, gyro_y, gyro_z; //units: deg/s
    float temp; //units: celsius
} mpu6050_data_t; // units: g
