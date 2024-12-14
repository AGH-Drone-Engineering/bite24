#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mycamera.h"
#include "detector.h"
#include "gupidetektor.h"

static const char TAG[] = "main";

#define WIDTH 240
#define HEIGHT 240

static unsigned char rgb888_buf[HEIGHT][WIDTH][3];

extern "C" void app_main(void)
{
    mycamera_init();
    detector_init();
    // gupidetektor_init();

    for (;;)
    {
        ESP_LOGI(TAG, "Taking picture...");
        ESP_ERROR_CHECK(mycamera_grab((unsigned char*) rgb888_buf, HEIGHT, WIDTH));

        // for (int i = 0; i < HEIGHT; i++)
        // {
        //     for (int j = 0; j < WIDTH; j++)
        //     {
        //         printf("%u %u %u ", rgb888_buf[i][j][0], rgb888_buf[i][j][1], rgb888_buf[i][j][2]);
        //     }
        //     printf("\n");
        // }
        // printf("\n\n");

        ESP_LOGI(TAG, "Detecting piwo on the picture...");
        // int x = 0;
        detector_detect((const unsigned char*) rgb888_buf, HEIGHT, WIDTH);
        // gupidetektor_detect((const unsigned char*) rgb888_buf, HEIGHT, WIDTH, &x);

        vTaskDelay(1);
    }
}
