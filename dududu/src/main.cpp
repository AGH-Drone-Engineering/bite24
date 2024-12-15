#include <Arduino.h>
#include <MPU9250.h>

#include "mahony.h"

#define DISTANCE_PIN 14

#define MOT_L_B_PIN 17
#define MOT_L_A_PIN 18
#define MOT_L_E_PIN 16

#define MOT_R_B_PIN 25
#define MOT_R_A_PIN 26
#define MOT_R_E_PIN 33

#define I2C_SDA 22
#define I2C_SCL 23

#define S3_RX 13
#define S3_TX 27

#define SERVO_A_PIN 21
#define SERVO_B_PIN 4

#define INIT_TIME_MS 5000

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

static MPU9250 IMU(Wire, 0x68);
static Mahony mahony;

static float g_nav_target_yaw = 0.0f;

static char g_s3_recv_buffer[256];
static int g_s3_recv_buffer_index = 0;

static unsigned long g_boot_time_ms;

static void set_motor(int pin_a, int pin_b, int pin_e, float cmd)
{
    if (cmd > 0.0f)
    {
        digitalWrite(pin_a, HIGH);
        digitalWrite(pin_b, LOW);
        analogWrite(pin_e, CLAMP((int)(cmd * 255.0f), 0, 255));
    }
    else if (cmd < 0.0f)
    {
        digitalWrite(pin_a, LOW);
        digitalWrite(pin_b, HIGH);
        analogWrite(pin_e, CLAMP((int)(-cmd * 255.0f), 0, 255));
    }
    else
    {
        digitalWrite(pin_a, HIGH);
        digitalWrite(pin_b, LOW);
        analogWrite(pin_e, 0);
    }
}

static void set_motors(float forward_cmd, float turn_cmd)
{
    set_motor(MOT_L_A_PIN, MOT_L_B_PIN, MOT_L_E_PIN, forward_cmd + turn_cmd);
    set_motor(MOT_R_A_PIN, MOT_R_B_PIN, MOT_R_E_PIN, forward_cmd - turn_cmd);
}

static void try_parse_s3_command(const char *command)
{
    int is_detected;
    int x;
    int y;
    int area;
    int sum;

    if (sscanf(command, "%d %d %d %d %d", &is_detected, &x, &y, &area, &sum) == 5)
    {
        Serial.print("Got S3 command: ");
        Serial.print(is_detected);
        Serial.print(" ");
        Serial.print(x);
        Serial.print(" ");
        Serial.print(y);
        Serial.print(" ");
        Serial.print(area);
        Serial.print(" ");
        Serial.print(sum);
        if (is_detected + x + y + area == sum)
        {
            Serial.println(" - Checksum ok");
        }
        else
        {
            Serial.println(" - Checksum failed");
        }
    }
    else
    {
        Serial.println("Failed to parse S3 command");
    }
}

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, S3_RX, S3_TX);

    pinMode(DISTANCE_PIN, INPUT);

    pinMode(MOT_L_A_PIN, OUTPUT);
    pinMode(MOT_L_B_PIN, OUTPUT);
    pinMode(MOT_L_E_PIN, OUTPUT);

    pinMode(MOT_R_A_PIN, OUTPUT);
    pinMode(MOT_R_B_PIN, OUTPUT);
    pinMode(MOT_R_E_PIN, OUTPUT);

    digitalWrite(MOT_L_A_PIN, LOW);
    digitalWrite(MOT_L_B_PIN, LOW);
    digitalWrite(MOT_L_E_PIN, LOW);

    digitalWrite(MOT_R_A_PIN, LOW);
    digitalWrite(MOT_R_B_PIN, LOW);
    digitalWrite(MOT_R_E_PIN, LOW);

    Wire.begin(I2C_SDA, I2C_SCL);
    int status = IMU.begin();
    if (status < 0)
    {
        Serial.println("IMU initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        Serial.print("Status: ");
        Serial.println(status);
        esp_restart();
    }

    g_boot_time_ms = millis();
}

void loop()
{
    delay(1);

    IMU.readSensor();

    static unsigned long last_time = 0;
    unsigned long now = micros();
    if (last_time = 0)
    {
        last_time = now;
    }
    float dt = (now - last_time) * 1e-6f;
    last_time = now;

    float gx = IMU.getGyroX_rads() * ((float)(180.0 / M_PI));
    float gy = IMU.getGyroY_rads() * ((float)(180.0 / M_PI));
    float gz = IMU.getGyroZ_rads() * ((float)(180.0 / M_PI)) - (99.0f / 120.0f);
    float ax = IMU.getAccelX_mss();
    float ay = IMU.getAccelY_mss();
    float az = IMU.getAccelZ_mss();

    mahony.updateIMU(
        gx, gy, gz,
        ax, ay, az,
        dt);

    float current_yaw = mahony.getYaw();

    if (millis() - g_boot_time_ms < INIT_TIME_MS)
    {
        g_nav_target_yaw = current_yaw;
    }

    float yaw_error = g_nav_target_yaw - current_yaw;
    if (yaw_error > 180.0f)
    {
        yaw_error -= 360.0f;
    }
    else if (yaw_error < -180.0f)
    {
        yaw_error += 360.0f;
    }

    float turn_cmd = 0.0f;
         if (yaw_error > 10.0f) turn_cmd = 0.7f;
    else if (yaw_error > 1.0f) turn_cmd = 0.5f;
    else if (yaw_error < -10.0f) turn_cmd = -0.7f;
    else if (yaw_error < -1.0f) turn_cmd = -0.5f;

    // Serial.print("Yaw error: ");
    // Serial.println(yaw_error);
    // Serial.print("Turn cmd: ");
    // Serial.println(turn_cmd);

    set_motors(0.0f, turn_cmd);

    while (Serial1.available())
    {
        char c = Serial1.read();
        if (c == '\n')
        {
            g_s3_recv_buffer[g_s3_recv_buffer_index] = '\0';
            try_parse_s3_command(g_s3_recv_buffer);
            g_s3_recv_buffer_index = 0;
        }
        else
        {
            g_s3_recv_buffer[g_s3_recv_buffer_index] = c;
            g_s3_recv_buffer_index = (g_s3_recv_buffer_index + 1) % sizeof(g_s3_recv_buffer);
        }
    }
}
