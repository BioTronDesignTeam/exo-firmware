#pragma once
struct AccelerationData {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct RotationData {
    float i = 0.0f;
    float j = 0.0f;
    float k = 0.0f;
    float real = 0.0f;
    float accuracyRadians = 0.0f;
    unsigned char status = 0;
};

struct SensorData {
    AccelerationData msa311;
    AccelerationData bno085Acceleration;
    RotationData bno085Rotation;
};
extern SensorData sensorData;