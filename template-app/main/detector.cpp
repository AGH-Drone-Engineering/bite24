#include "detector.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model.h"

static const char TAG[] = "detector";

static constexpr int kTensorArenaSize = 2097152;
EXT_RAM_BSS_ATTR static uint8_t tensor_arena[kTensorArenaSize];

static tflite::MicroMutableOpResolver<8> resolver;
static tflite::MicroInterpreter *interpreter;

static TfLiteTensor *input;
static TfLiteTensor *output;

void detector_init()
{
    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    const tflite::Model *model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        ESP_LOGE(TAG, "Model provided is schema version %ld not equal to supported "
                      "version %d.",
                 model->version(), TFLITE_SCHEMA_VERSION);
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    // Pull in only the operation implementations we need.
    resolver.AddConv2D();
    resolver.AddRelu6();
    resolver.AddDepthwiseConv2D();
    resolver.AddPad();
    resolver.AddAdd();
    resolver.AddMean();
    resolver.AddFullyConnected();
    resolver.AddLogistic();

    // Build an interpreter to run the model with.
    interpreter = new tflite::MicroInterpreter(
        model, resolver, tensor_arena, kTensorArenaSize);

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        ESP_LOGE(TAG, "AllocateTensors() failed");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    ESP_LOGI(TAG, "Used arena: %d bytes", interpreter->arena_used_bytes());

    input = interpreter->input(0);
    output = interpreter->output(0);
}

int detector_detect(const unsigned char *image, int height, int width)
{
    if (input->dims->data[1] != height || input->dims->data[2] != width)
    {
        ESP_LOGE(TAG, "Input tensor shape mismatch: %d != %d or %d != %d",
                    input->dims->data[1], height, input->dims->data[2], width);
        return -1;
    }

    // Copy image data to input tensor
    for (int i = 0; i < height * width * 3; i++)
    {
        input->data.int8[i] = (int8_t) (((int) image[i]) - 128);
    }

    int64_t start_us = esp_timer_get_time();
    TfLiteStatus invoke_status = interpreter->Invoke();
    int64_t latency = esp_timer_get_time() - start_us;
    if (invoke_status != kTfLiteOk)
    {
        ESP_LOGE(TAG, "Invoke failed");
        return -1;
    }
    int64_t fps = 1000000 / latency;
    ESP_LOGI(TAG, "Latency: %lld ms, FPS: %lld", latency / 1000, fps);

    // Get the output tensor
    int8_t *mask = output->data.int8;

    ESP_LOGW(TAG, "Mask output:");
    for (int i = 0; i < 17; i++)
    {
        for (int j = 0; j < 17; j++)
        {
            printf("%3d ", ((int) mask[i * 17 + j]) + 128);
        }
        printf("\n");
    }
    printf("\n");

    return 0;
}
