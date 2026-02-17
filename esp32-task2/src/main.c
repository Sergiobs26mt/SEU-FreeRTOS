#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TASK1_PRIORITY        1
#define TASK2_PRIORITY        2
#define TASK3_PRIORITY        3
#define APP_MAIN_PRIORITY     5
#define TASK_STACK_SIZE       2048   /* Words task stack size */

#define TASK_RUNNING_TIME_MS  5000   /* Time for tasks to run */

#define TASK1_CORE_ID         0
#define TASK2_CORE_ID         1
#define TASK3_CORE_ID         1

void vTask1(void * parameter);
void vTask2(void * parameter);
void vTask3(void * parameter);

void app_main() 
{
  TaskHandle_t xHandle1 = NULL;
  TaskHandle_t xHandle2 = NULL;
  TaskHandle_t xHandle3 = NULL;
  
  /* Set high priority for app_main */
  vTaskPrioritySet(NULL, APP_MAIN_PRIORITY);  
  
  /* Create a new task and add it to the list of tasks that are ready to run */
  xTaskCreatePinnedToCore(
      vTask1,           /* Task function */
      "Task1",          /* Name of task; for human use */
      TASK_STACK_SIZE,  /* Stack size of task */
      NULL,             /* Parameter of the task */
      TASK1_PRIORITY,   /* Priority of the task */
      &xHandle1,        /* Task handle to keep track of created task */
      TASK1_CORE_ID);   /* Core ID */
  configASSERT(xHandle1);

  /* Create a new task and add it to the list of tasks that are ready to run */
  xTaskCreatePinnedToCore(vTask2, "Task2", TASK_STACK_SIZE, NULL, TASK2_PRIORITY, &xHandle2, TASK2_CORE_ID); 
  configASSERT(xHandle2);

  /* Create a new task and add it to the list of tasks that are ready to run */
  xTaskCreatePinnedToCore(vTask3, "Task3", TASK_STACK_SIZE, NULL, TASK3_PRIORITY, &xHandle3, TASK3_CORE_ID);
  configASSERT(xHandle3);  

  /* Wait TASK_RUNNING_TIME_MS ms */
  vTaskDelay(TASK_RUNNING_TIME_MS / portTICK_PERIOD_MS);

  /* Use the handles to delete the tasks. */
  if (xHandle1 != NULL)
  {
    vTaskDelete(xHandle1);
  }  
  if (xHandle2 != NULL)
  {
    vTaskDelete(xHandle2);
  }
  if (xHandle3 != NULL)
  {
    vTaskDelete(xHandle3);
  }  
}

/* Task function */
void vTask1(void * parameter)
{
  int counter = 0;
  
  /* Change priority to match Task3 */
  vTaskPrioritySet(NULL, TASK3_PRIORITY);
  
  /* loop forever */
  for(;;)
  {
    printf("[Task1] Loop iteration %d, Priority: %d\n", ++counter, uxTaskPriorityGet(NULL));
  }
}

/* Task function */
void vTask2(void * parameter)
{
  int counter = 0;
  
  /* Change priority to match Task3 */
  vTaskPrioritySet(NULL, TASK3_PRIORITY);
  
  /* loop forever */
  for(;;)
  {
    printf("[Task2] Loop iteration %d, Priority: %d\n", ++counter, uxTaskPriorityGet(NULL));
  }
}

/* Task function */
void vTask3(void * parameter)
{
  int counter = 0;
  
  /* loop forever */
  for(;;)
  {
    printf("[Task3] Loop iteration %d, Priority: %d\n", ++counter, uxTaskPriorityGet(NULL));
  }
}