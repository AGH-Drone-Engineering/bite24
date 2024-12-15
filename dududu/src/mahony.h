#ifndef __FUSION_MAHONY_H__
#define __FUSION_MAHONY_H__

#include <cmath>

class Mahony
{
public:
    Mahony();

    Mahony(float prop_gain, float int_gain);

    void update(float gx, float gy, float gz, float ax, float ay, float az,
                float mx, float my, float mz, float dt);

    void updateIMU(float gx, float gy, float gz, float ax, float ay, float az,
                   float dt);

    float getKp()
    {
        return twoKp / 2.0f;
    }

    void setKp(float Kp)
    {
        twoKp = 2.0f * Kp;
    }

    float getKi()
    {
        return twoKi / 2.0f;
    }

    void setKi(float Ki)
    {
        twoKi = 2.0f * Ki;
    }

    float getRoll()
    {
        if (!anglesComputed)
            computeAngles();
        return roll * 57.29578f;
    }

    float getPitch()
    {
        if (!anglesComputed)
            computeAngles();
        return pitch * 57.29578f;
    }

    float getYaw()
    {
        if (!anglesComputed)
            computeAngles();
        return yaw * 57.29578f + 180.0f;
    }

    float getRollRadians()
    {
        if (!anglesComputed)
            computeAngles();
        return roll;
    }

    float getPitchRadians()
    {
        if (!anglesComputed)
            computeAngles();
        return pitch;
    }

    float getYawRadians()
    {
        if (!anglesComputed)
            computeAngles();
        return yaw;
    }

    void getQuaternion(float *w, float *x, float *y, float *z)
    {
        *w = q0;
        *x = q1;
        *y = q2;
        *z = q3;
    }

    void setQuaternion(float w, float x, float y, float z)
    {
        q0 = w;
        q1 = x;
        q2 = y;
        q3 = z;
    }

    void getGravityVector(float *x, float *y, float *z)
    {
        if (!anglesComputed)
            computeAngles();
        *x = grav[0];
        *y = grav[1];
        *z = grav[2];
    }

private:
    float twoKp;                                 // 2 * proportional gain (Kp)
    float twoKi;                                 // 2 * integral gain (Ki)
    float q0, q1, q2, q3;                        // quaternion of sensor frame relative to auxiliary frame
    float integralFBx, integralFBy, integralFBz; // integral error terms scaled by Ki
    float roll, pitch, yaw;
    float grav[3];
    bool anglesComputed = false;
    static float invSqrt(float x);
    void computeAngles();
};

#endif
