#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task.h>
#include <math.h>
#include <stdio.h>

#define TASK1_PRIORITY        4
#define TASK2_PRIORITY        3
#define TASK3_PRIORITY        3
#define APP_MAIN_PRIORITY     5
#define TASK_STACK_SIZE       2048

#define TASK_RUNNING_TIME_MS  1000

#define LOOP_COUNT_TASK_1     1000
#define LOOP_COUNT_TASK_2     2000
#define LOOP_COUNT_TASK_3     2000

// Períodos de las tareas
#define TASK1_PERIOD_MS       200
#define TASK2_PERIOD_MS       100
#define TASK3_PERIOD_MS       100

void vTask1(void * parameter);
void vTask2(void * parameter);
void vTask3(void * parameter);

void app_main() 
{
  TaskHandle_t xHandle1 = NULL;
  TaskHandle_t xHandle2 = NULL;
  TaskHandle_t xHandle3 = NULL;
  
  vTaskPrioritySet(NULL, APP_MAIN_PRIORITY);  
  
  xTaskCreate(vTask1, "Task1", TASK_STACK_SIZE, NULL, TASK1_PRIORITY, &xHandle1);
  configASSERT(xHandle1);

  xTaskCreate(vTask2, "Task2", TASK_STACK_SIZE, NULL, TASK2_PRIORITY, &xHandle2); 
  configASSERT(xHandle2);

  xTaskCreate(vTask3, "Task3", TASK_STACK_SIZE, NULL, TASK3_PRIORITY, &xHandle3);
  configASSERT(xHandle3);  

  vTaskDelay(TASK_RUNNING_TIME_MS / portTICK_PERIOD_MS);
 
  char bufferStats[2048];
  vTaskGetRunTimeStats(bufferStats);
  printf("\n%s\n", bufferStats);  
  
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

/* Task 1 function - Periodic task: t = 200 ms */
void vTask1(void * parameter)
{
  double aux = acos(-1.0);
  TickType_t xLastWakeTime = xTaskGetTickCount();
 
  for(;;)
  {
    for (int i = 0; i < LOOP_COUNT_TASK_1; i++)
    {
        aux = sqrt(aux) * acos(-1.0) + aux * 0.1;
        if (i % 100 == 0)
        {
          printf("#");
          fflush(stdout);   
        }
    }    
    aux = acos(-1.0);
    
    // Esperar hasta el siguiente período
    vTaskDelayUntil(&xLastWakeTime, TASK1_PERIOD_MS / portTICK_PERIOD_MS);
  }
}

/* Task 2 function - Periodic task: t = 100 ms */
void vTask2(void * parameter)
{
  double aux = acos(-1.0);
  TickType_t xLastWakeTime = xTaskGetTickCount();
 
  for(;;)
  {
    for (int i = 0; i < LOOP_COUNT_TASK_2; i++)
    {
        aux = sqrt(aux) * acos(-1.0) + aux * 0.1;
        if (i % 100 == 0)
        {
          printf("-");
          fflush(stdout);    
        }
    }
    aux = acos(-1.0);
    
    // Esperar hasta el siguiente período
    vTaskDelayUntil(&xLastWakeTime, TASK2_PERIOD_MS / portTICK_PERIOD_MS);
  }
}

/* Task 3 function - Periodic task: t = 100 ms */
void vTask3(void * parameter)
{
  double aux = acos(-1.0);
  TickType_t xLastWakeTime = xTaskGetTickCount();
 
  for(;;)
  {
    for (int i = 0; i < LOOP_COUNT_TASK_3; i++)
    {
        aux = sqrt(aux) * acos(-1.0) + aux * 0.1;
        if (i % 100 == 0)
        {
          printf(".");
          fflush(stdout);    
        }
    }
    aux = acos(-1.0);
    
    // Esperar hasta el siguiente período
    vTaskDelayUntil(&xLastWakeTime, TASK3_PERIOD_MS / portTICK_PERIOD_MS);
  }
}