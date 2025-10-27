
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"                 // Kurssin oma kirjasto JTKJ Hat -lisäosille


#define DEFAULT_STACK_SIZE 2048
#define CDC_ITF_TX      1               // Sarjaliikenteen liitäntäkanava

//Raja-arvot kallistukselle

#define X_POS_THRESHOLD 0.35f
#define X_NEG_THRESHOLD -0.35f
#define Z_POS_THRESHOLD 0.35f
#define Z_NEG_THRESHOLD -0.35f

// Tilakoneen määrittely
enum state { WAITING=1};
enum state programState = WAITING;

// ==============================================
// SENSOR_TASK
// =============================================
static void sensor_task(void *arg){
    (void)arg;

    // Alustetaan IMU
    init_ICM42670();

    // Käynnistetään IMU:n kiihtyvyys ja gyroskooppi mittaukset. Valitaan sopivat parametrit.
    ICM42670_startAccel(ICM42670_ACCEL_ODR_100HZ, ICM42670_ACCEL_FSR_4G);       // Valittiin 4G tarkkuus kiihtyvyydelle, ei liian tarkka eikä liian karkea tekoäly ehdotti arvoja.
    ICM42670_startGyro(ICM42670_GYRO_ODR_100HZ, ICM42670_GYRO_FSR_500DPS);      // Valittin 500°/s, tarpeeksi tarkka mutta ei liian herkkä.Tekoäly ehdotti arvoja


    // Luodaan muuttujat datan lukemista varten
    float ax, ay, az, gx, gy, gz, t;

    for(;;){
        ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t);

        if (ax > X_POS_THRESHOLD) {
            printf("Piste\n");
        }
        else if (ax < X_NEG_THRESHOLD) {
            printf("Viiva\n");
        }
        else if (az > Z_POS_THRESHOLD) {
            printf("Välilyönti\n");
        }
        else if (az < Z_NEG_THRESHOLD) {
            printf("Lähetä\n");
        }
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

