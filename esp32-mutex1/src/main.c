#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "esp_log.h"
#include "esp_random.h"

#define TASK_STACK_SIZE                 2048
#define NUM_TASKS                       5
#define TASK_PRIORITY                   3
#define TASK_MAIN_PRIORITY              5

#define TASK_RUNNING_TIME_MS            5000

static const char* TAG = "Mutex";

struct TaskParam
{
    SemaphoreHandle_t xSemaphore;
    int taskID;
};
typedef struct TaskParam t_TaskParam;

void vTask(void * param);
void UseResource(int id);

void app_main()
{
    TaskHandle_t TaskHandles[NUM_TASKS];

    vTaskPrioritySet(NULL, TASK_MAIN_PRIORITY);

    /* Create mutex */
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();
    if (xSemaphore == NULL)
    {
        ESP_LOGE(TAG, "[app_main] Error creating mutex.");
        exit(EXIT_FAILURE);
    }
    ESP_LOGI(TAG, "[app_main] Mutex created.");    

    for (int i = 0; i < NUM_TASKS; i++)
    {
        t_TaskParam param;
        param.xSemaphore = xSemaphore;
        param.taskID = i;
        xTaskCreate(vTask, "Task", TASK_STACK_SIZE, (void *)&param, TASK_PRIORITY, &TaskHandles[i]);
        ESP_LOGI(TAG, "[app_main] Task %d created.", i);
    }

    /* NOTE: Unlike binary semaphores, mutexes are created in an unlocked state */
    
    /* Wait TASK_RUNNING_TIME_MS ms */
    ESP_LOGI(TAG, "[app_main] Entering blocked state...");
    vTaskDelay(TASK_RUNNING_TIME_MS / portTICK_PERIOD_MS);

    for (int i = 0; i < NUM_TASKS; i++)
    {
        if (TaskHandles[i] != NULL)
        {
            vTaskDelete(TaskHandles[i]);
            ESP_LOGI(TAG, "[app_main] Task %d deleted.", i);
        }
    }

    if (xSemaphore != NULL)
    {
        vSemaphoreDelete(xSemaphore);
        ESP_LOGI(TAG, "[app_main] Mutex deleted.");
    }
}

void vTask(void * param)
{
    t_TaskParam TaskData = *(t_TaskParam *)param;

    for (;;)
    {
        ESP_LOGI(TAG, "[vTask] Task %d attempts to use resource...", TaskData.taskID);
        /* Wait for the semaphore */
        if (xSemaphoreTake(TaskData.xSemaphore, portMAX_DELAY) == pdTRUE)
        {
            UseResource(TaskData.taskID);
            /* Signal the semaphore */
            xSemaphoreGive(TaskData.xSemaphore);
        }
        else
        {
            ESP_LOGI(TAG, "[vTask] Cannot take the semaphore.");
        }
    }
}

void UseResource(int id)
{
    int wait_ms = esp_random() % 500;
    ESP_LOGI(TAG, "[UseResource] Using resource: Task %d", id);
    vTaskDelay(wait_ms / portTICK_PERIOD_MS);
}