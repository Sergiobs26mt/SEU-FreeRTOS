#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"

#define TASK_STACK_SIZE                 2048
#define TASK_ACQ_PRIORITY               5
#define TASK_DISP_PRIORITY              3

#define TEMPERATURE_QUEUE_LENGTH        10
#define TEMPERATURE_ACQ_RATE_MS         1000

static const char* TAG = "TempQueue";

void TemperatureAcquisition(void * queue);
void TemperatureDisplay(void * queue);

void app_main()
{
    QueueHandle_t xTempQueue = NULL;
    TaskHandle_t xHandleAcq = NULL;
    TaskHandle_t xHandleDisp = NULL;    
    
    // Tarea 1: Crear cola de TEMPERATURE_QUEUE_LENGTH elementos de tipo int
    xTempQueue = xQueueCreate(TEMPERATURE_QUEUE_LENGTH, sizeof(int));
    if (xTempQueue == NULL)
    {
        ESP_LOGE(TAG, "Error creating queue. Restarting...");
        exit(EXIT_FAILURE);                
    }

    // Tarea 2: Crear tarea de adquisición con la cola como parámetro
    xTaskCreate(
        TemperatureAcquisition,     // Función de la tarea
        "Task_Acq",                 // Nombre de la tarea
        TASK_STACK_SIZE,            // Tamaño del stack
        (void *)xTempQueue,         // Parámetro: cola
        TASK_ACQ_PRIORITY,          // Prioridad
        &xHandleAcq);               // Handle de la tarea
    configASSERT( xHandleAcq );
    ESP_LOGI(TAG, "[app_main] Task_Acq created.");

    // Tarea 2: Crear tarea de visualización con la cola como parámetro
    xTaskCreate(
        TemperatureDisplay,         // Función de la tarea
        "Task_Disp",                // Nombre de la tarea
        TASK_STACK_SIZE,            // Tamaño del stack
        (void *)xTempQueue,         // Parámetro: cola
        TASK_DISP_PRIORITY,         // Prioridad
        &xHandleDisp);              // Handle de la tarea
    configASSERT( xHandleDisp );
    ESP_LOGI(TAG, "[app_main] Task_Disp created.");

    /* Enter suspend state forever */
    vTaskSuspend(NULL);

    if (xHandleAcq != NULL)
    {
        vTaskDelete(xHandleAcq);
    }

    if (xHandleDisp != NULL)
    {
        vTaskDelete(xHandleDisp);
    }
}

void TemperatureAcquisition(void * queue)
{
    TickType_t xLastWakeTime;

    // Tarea 3: Obtener el manejador de la cola desde el parámetro
    QueueHandle_t xQueue = (QueueHandle_t)queue;

    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        /* Simulate temperature reading. Any value in the range [1-30] */
        int temp_reading = esp_random() % 30 + 1;

        // Tarea 4: Insertar el valor en la cola de forma bloqueante
        xQueueSendToBack(xQueue, &temp_reading, portMAX_DELAY);

        vTaskDelayUntil(&xLastWakeTime, TEMPERATURE_ACQ_RATE_MS / portTICK_PERIOD_MS);
    }
}

void TemperatureDisplay(void * queue)
{
    // Tarea 3: Obtener el manejador de la cola desde el parámetro
    QueueHandle_t xQueue = (QueueHandle_t)queue;

    for (;;)
    {
        int received;

        // Tarea 5: Recibir un valor de la cola de forma bloqueante
        BaseType_t xStatus = xQueueReceive(xQueue, &received, portMAX_DELAY);
        if (xStatus == pdPASS)
        {
            printf("Temperature measurement: %d°C\n", received);
        }
        else
        {
            ESP_LOGW(TAG, "Could not receive from the queue.");
        }
    }
}