#include "gupidetektor.h"

#include "esp_log.h"

static const char TAG[] = "gupidetektor";

void gupidetektor_init()
{

}

// void gupidetektor_detect(const uint8_t *image, int width, int height, int *x)
// {
//     int elem_size = 3;
//     int row_size = width * elem_size;

//     int64_t j_sum = 0;
//     int64_t weight_sum = 0;
//     for (int i = 0; i < height; i++)
//     {
//         for (int j = 0; j < width; j++)
//         {
//             uint8_t r = image[i * row_size + j * elem_size + 0];
//             uint8_t g = image[i * row_size + j * elem_size + 1];
//             uint8_t b = image[i * row_size + j * elem_size + 2];

//             int64_t value = ((int64_t) g) - ((int64_t) r) - ((int64_t) b);

//             j_sum += j * value;
//             weight_sum += value;
//         }
//     }

//     int64_t j_avg = j_sum / weight_sum;
//     *x = j_avg;

//     ESP_LOGI(TAG, "x: %llu", j_avg);
// }

void gupidetektor_detect(const uint8_t *image, int width, int height, int *x)
{
    int elem_size = 3;
    int row_size = width * elem_size;

    int best_j = 0;
    int best_value = -10000;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            uint8_t r = image[i * row_size + j * elem_size + 0];
            uint8_t g = image[i * row_size + j * elem_size + 1];
            uint8_t b = image[i * row_size + j * elem_size + 2];

            int64_t value = ((int64_t) g) - ((int64_t) r) - ((int64_t) b);

            if (value > best_value)
            {
                best_value = value;
                best_j = j;
            }
        }
    }

    ESP_LOGI(TAG, "x: %d ", best_j);
}

// void gupidetektor_detect(const uint8_t *image, int width, int height, int *x)
// {
//     int elem_size = 3;
//     int row_size = width * elem_size;

//     double j_sum = 0;
//     double weight_sum = 0;
//     for (int i = 0; i < height; i++)
//     {
//         for (int j = 0; j < width; j++)
//         {
//             double r = (1.0 / 255.0) * image[i * row_size + j * elem_size + 0];
//             double g = (1.0 / 255.0) * image[i * row_size + j * elem_size + 1];
//             double b = (1.0 / 255.0) * image[i * row_size + j * elem_size + 2];

//             double r_error = r - 0.0;
//             double g_error = g - 1.0;
//             double b_error = b - 0.0;

//             double value = 1 - (r_error * r_error + g_error * g_error + b_error * b_error) * 0.333333333333333;

//             j_sum += j * value;
//             weight_sum += value;
//         }
//     }

//     double j_avg = j_sum / weight_sum;
//     // *x = j_avg;

//     ESP_LOGI(TAG, "x: %f", j_avg);
// }
