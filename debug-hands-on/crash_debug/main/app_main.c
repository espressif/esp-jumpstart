/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void crash()
{
    volatile int *myptr = NULL;
    printf("Myval is %d\n", *myptr);
}

void do_something_else()
{
    crash();
}

void do_something()
{
    do_something_else();
}

void app_main()
{
    printf("App main entered!\n");
    do_something();
}
