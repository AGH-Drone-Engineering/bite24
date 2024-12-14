#include "mycamera.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "img_converters.h"

// XIAO ESP32-S3
#define CAM_PIN_PWDN    -1 //power down is not used
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    10
#define CAM_PIN_SIOD    40
#define CAM_PIN_SIOC    39

#define CAM_PIN_D7      48
#define CAM_PIN_D6      11
#define CAM_PIN_D5      12
#define CAM_PIN_D4      14
#define CAM_PIN_D3      16
#define CAM_PIN_D2      18
#define CAM_PIN_D1      17
#define CAM_PIN_D0      15
#define CAM_PIN_VSYNC   38
#define CAM_PIN_HREF    47
#define CAM_PIN_PCLK    13

static const char TAG[] = "mycamera";

static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 20000000,//EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_240X240,//QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_LATEST//CAMERA_GRAB_LATEST. Sets when buffers should be filled
};

void mycamera_init(void)
{
    ESP_ERROR_CHECK(esp_camera_init(&camera_config));
}

esp_err_t mycamera_grab(unsigned char *image, int height, int width)
{
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) 
    {
        ESP_LOGE(TAG, "Failed capturing a picture!");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Picture taken! It's size was: %zu bytes", pic->len);

    if (pic->width != width || pic->height != height) 
    {
        ESP_LOGE(TAG, "Picture size is not correct! Expected: %dx%d, got: %dx%d", width, height, pic->width, pic->height);
        esp_camera_fb_return(pic);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Converting picture...");

    if (!fmt2rgb888(pic->buf, pic->len, pic->format, image))
    {
        ESP_LOGE(TAG, "Failed converting picture!");
        esp_camera_fb_return(pic);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Picture converted!");

    // printf("\n\n");
    // uint8_t *jpeg_buf = NULL;
    // size_t jpeg_len = 0;
    // if (!fmt2jpg(pic->buf, pic->len, pic->width, pic->height, pic->format, 90, &jpeg_buf, &jpeg_len))
    // {
    //     ESP_LOGE(TAG, "Failed converting picture to JPEG!");
    //     esp_camera_fb_return(pic);
    //     return ESP_FAIL;
    // }
    // for (int i = 0; i < jpeg_len; i++)
    // {
    //     printf("%02x", jpeg_buf[i]);
    // }
    // printf("\n\n");
    // free(jpeg_buf);

    esp_camera_fb_return(pic);

    return ESP_OK;
}
