#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

void vTaskCode(void * parameter);

static const char* TAG = "esp32-task1";

void app_main() 
{
  TaskHandle_t xHandle = NULL;
  
  /* Create a new task and add it to the list of tasks that are ready to run */
  xTaskCreate(
      vTaskCode,                              /* Task function */
      "Task",                                 /* Name of task; for human use */
      configMINIMAL_STACK_SIZE,               /* Stack size of task */
      NULL,                                   /* Parameter of the task */
      CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, /* Priority of the task */
      &xHandle);                              /* Task handle to keep track of created task */

  configASSERT( xHandle );

  /* Wait 1 sec */
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  /* Use the handle to delete the task. */
  if (xHandle != NULL)
  {
    vTaskDelete( xHandle );
  }  
}

/* Task function */
void vTaskCode(void * parameter)
{
  int counter = 0;
  
  /* loop forever */
  for(;;)
  {
    ESP_LOGI(TAG, "Loop iteration %d\n", ++counter);
  }
}