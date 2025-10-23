
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"                 // Kurssin oma kirjasto JTKJ Hat -lisäosille


#define DEFAULT_STACK_SIZE 2048
#define CDC_ITF_TX      1               // Sarjaliikenteen liitäntäkanava

// Tilakoneen määrittely
enum state { WAITING=1};
enum state programState = WAITING;

// =============================================
// SENSOR_TASK
// =============================================
static void sensor_task(void *arg){
    (void)arg;

    // Alustetaan IMU
    init_ICM42670();

    // Käynnistetään mittaukset oletusarvoilla
    ICM42670_start_with_default_values();

    // Luodaan muuttujat datan lukemista varten
    float ax, ay, az, gx, gy, gz, t;

    for(;;){
        ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t);
        printf("X-akseli: %.2f | Y-akseli: %.2f | Z-akseli: %.2f\n", ax, ay, az);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// =============================================
// PRINT_TASK
// =============================================
static void print_task(void *arg){
    (void)arg;
    while(1){
        tight_loop_contents();
        printf("printTask\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ================================================
// MAIN_FUNKTION TOIMINTA
// ================================================

int main() {
    stdio_init_all();       // Alustaa standarditulosteen
    init_hat_sdk();         // JTKJ Hat -lisäosat
    sleep_ms(300);          // Pieni viive, jotta alustukset ehtivät valmistua

    // Määritellään tehtävien hallintakahvat
    TaskHandle_t hSensorTask, hPrintTask, hUSB = NULL;

    // Luodaan sensor_task
    BaseType_t result = xTaskCreate(sensor_task, 
                "sensor",                        
                DEFAULT_STACK_SIZE,              
                NULL,                            
                2,                               
                &hSensorTask);                   

    if(result != pdPASS) {
        printf("Sensor task creation failed\n");
        return 0;
    }

    // Luodaan print_task
    result = xTaskCreate(print_task,  
                "print",               
                DEFAULT_STACK_SIZE,   
                NULL,                 
                2,                    
                &hPrintTask);         

    if(result != pdPASS) {
        printf("Print Task creation failed\n");
        return 0;
    }

    // Käynnistetään FreeRTOS:n ajastin, joka suorittaa tehtävät
    vTaskStartScheduler();

    return 0;
}

