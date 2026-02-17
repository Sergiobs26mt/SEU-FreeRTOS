#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"
#include "esp_random.h"

#define TASK_STACK_SIZE                 2048
#define TASK_ACQ_PRIORITY               5
#define TASK_DISP_PRIORITY              3
#define NUM_ACQ_SENSORS                 5

#define HT_QUEUE_LENGTH                 NUM_ACQ_SENSORS * 10
#define HT_ACQ_RATE_MS                  1000

static const char* TAG = "HTQueue";

void HTAcquisition(void * queue);
void HTDisplay(void * queue);

struct HTreading
{
    unsigned int sensorID;
    int temperature;
    unsigned int humidity;
};
typedef struct HTreading t_HTreading;

struct SensorParam
{
    unsigned int sensorID;
    QueueHandle_t queue;
};
typedef struct SensorParam t_SensorParam;

void app_main()
{
    QueueHandle_t xHTQueue = NULL;
    TaskHandle_t xHandleAcq[NUM_ACQ_SENSORS];
    TaskHandle_t xHandleDisp = NULL;    
    
    xHTQueue = xQueueCreate(HT_QUEUE_LENGTH, sizeof(t_HTreading));
    if (xHTQueue == NULL)
    {
        ESP_LOGE(TAG, "Error creating queue. Restarting...");
        exit(EXIT_FAILURE);                
    }

    for (unsigned int i = 0; i < NUM_ACQ_SENSORS; i++)
    {
        t_SensorParam param;
        param.sensorID = i;
        param.queue = xHTQueue;

        xTaskCreate(HTAcquisition, "Task_Acq", TASK_STACK_SIZE, (void *)&param, TASK_ACQ_PRIORITY, &xHandleAcq[i]);
        configASSERT( xHandleAcq[i] );
        ESP_LOGI(TAG, "[app_main] Task_Acq %d created.", i);
    }

    xTaskCreate(HTDisplay, "Task_Disp", TASK_STACK_SIZE, (void *)xHTQueue, TASK_DISP_PRIORITY, &xHandleDisp);
    configASSERT( xHandleDisp );
    ESP_LOGI(TAG, "[app_main] Task_Disp created.");

    /* Enter suspend state forever */
    vTaskSuspend(NULL);

    for (unsigned int i = 0; i < NUM_ACQ_SENSORS; i++)
    {
        if (xHandleAcq[i] != NULL)
        {
            vTaskDelete(xHandleAcq[i]);
        }
    }
       
    if (xHandleDisp != NULL)
    {
        vTaskDelete(xHandleDisp);
    }
}

void HTAcquisition(void * param)
{
    TickType_t xLastWakeTime;

    t_SensorParam SP = *(t_SensorParam *)param;    

    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();
        
        /* Simulate temperature and humidity readings. */
        t_HTreading HT;
        HT.sensorID = SP.sensorID;        
        HT.temperature = esp_random() % 30 + 1;
        HT.humidity = esp_random() % 100;

        xQueueSendToBack(SP.queue, &HT, portMAX_DELAY);

        vTaskDelayUntil(&xLastWakeTime, HT_ACQ_RATE_MS / portTICK_PERIOD_MS);
    }
}

void HTDisplay(void * queue)
{
    QueueHandle_t xQueue = (QueueHandle_t)queue;

    for (;;)
    {
        t_HTreading HTreceived;

        BaseType_t xStatus = xQueueReceive(xQueue, &HTreceived, portMAX_DELAY);
        if (xStatus == pdPASS)
        {
            printf("Sensor ID %d: Temperature %d°C, humidity %d%%\n", 
                HTreceived.sensorID, HTreceived.temperature, HTreceived.humidity);
        }
        else
        {
            ESP_LOGW(TAG, "Could not receive from the queue.");
        }
    }
}