/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

void my_task(void *pvParameter)
{
    char myarray[512];
    memset(myarray, 0, sizeof(myarray));
    while (1) {
        vTaskDelay(5000);
    }
}

void app_main()
{
    printf("App main entered!\n");
    xTaskCreate(&my_task, "my_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}
