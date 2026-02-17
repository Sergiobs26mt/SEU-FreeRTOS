#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"
#include "esp_random.h"

#define TASK_STACK_SIZE                 2048
#define TASK_ACQ_PRIORITY               5
#define TASK_DISP_PRIORITY              3
#define NUM_ACQ_SENSORS                 5

#define HT_QUEUE_LENGTH                 10
#define HT_ACQ_RATE_MS                  1000

static const char* TAG = "HTQueueSet";

void HTAcquisition(void * param);
void HTDisplay(void * queueSet);

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
    QueueHandle_t xHTQueue[NUM_ACQ_SENSORS];
    TaskHandle_t xHandleAcq[NUM_ACQ_SENSORS];
    TaskHandle_t xHandleDisp = NULL;            

    /* Create the queue set */
    QueueSetHandle_t xQueueSet = xQueueCreateSet(NUM_ACQ_SENSORS * 10);
    if (xQueueSet == NULL)
    {
        ESP_LOGE(TAG, "Error creating queue set. Restarting...");
        exit(EXIT_FAILURE);
    }
    ESP_LOGI(TAG, "[app_main] Queue set created.");

    /* Create queues and tasks for each sensor */
    for (unsigned int i = 0; i < NUM_ACQ_SENSORS; i++)
    {
        xHTQueue[i] = xQueueCreate(HT_QUEUE_LENGTH, sizeof(t_HTreading));
        if (xHTQueue[i] == NULL)
        {
            ESP_LOGE(TAG, "Error creating queue %d. Restarting...", i);
            exit(EXIT_FAILURE);                
        }

        /* Add the queue to the set */
        if (xQueueAddToSet(xHTQueue[i], xQueueSet) != pdPASS)
        {
            ESP_LOGE(TAG, "Error adding queue %d to set. Restarting...", i);
            exit(EXIT_FAILURE);
        }

        /* Create acquisition task for sensor i */
        t_SensorParam param;
        param.sensorID = i;
        param.queue = xHTQueue[i];

        char taskName[16];
        snprintf(taskName, sizeof(taskName), "Task_Acq_%d", i);
        xTaskCreate(HTAcquisition, taskName, TASK_STACK_SIZE, 
                   (void *)&param, TASK_ACQ_PRIORITY, &xHandleAcq[i]);
        
        /* Small delay to ensure task parameter is copied */
        vTaskDelay(10 / portTICK_PERIOD_MS);
        
        ESP_LOGI(TAG, "[app_main] Task_Acq_%d created.", i);
    }

    /* Create display task */
    xTaskCreate(HTDisplay, "Task_Disp", TASK_STACK_SIZE, 
               (void *)xQueueSet, TASK_DISP_PRIORITY, &xHandleDisp);
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
        
        /* Simulate temperature and humidity readings */
        t_HTreading HT;
        HT.sensorID = SP.sensorID;        
        HT.temperature = esp_random() % 30 + 1;
        HT.humidity = esp_random() % 100;

        xQueueSendToBack(SP.queue, &HT, portMAX_DELAY);

        vTaskDelayUntil(&xLastWakeTime, HT_ACQ_RATE_MS / portTICK_PERIOD_MS);
    }
}

void HTDisplay(void * queueSet)
{
    QueueSetHandle_t xQueueSet = (QueueSetHandle_t)queueSet;    

    for (;;)
    {
        t_HTreading HTreceived;

        /* Select the queue */
        QueueHandle_t queue = (QueueHandle_t)xQueueSelectFromSet(xQueueSet, portMAX_DELAY);
        
        /* Receive from the queue */
        BaseType_t xStatus = xQueueReceive(queue, &HTreceived, 0);
        
        if (xStatus == pdPASS)
        {
            ESP_LOGI(TAG, "[HTDisplay] Sensor %d: Temp = %d°C, Humidity = %d%%", 
                    HTreceived.sensorID, HTreceived.temperature, HTreceived.humidity);
        }
        else
        {
            ESP_LOGW(TAG, "[HTDisplay] Failed to receive data from queue");
        }
    }
}