
// Koodi on tehty kokonaan yhdessä tasavertaisesti eli halutaan jakaa 6 pistettä tasan. Tekijät: Adele Wahlberg, Sini Korin ja Emilia Kamula

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
#define X_POS_THRESHOLD 0.7f
#define X_NEG_THRESHOLD -0.7f
#define Z_POS_THRESHOLD 0.7f
#define Z_NEG_THRESHOLD -0.7f



// ==============================================
// SENSOR_TASK
// =============================================
static void sensor_task(void *arg){
    (void)arg;

    // Alustetaan IMU
    if (init_ICM42670() != 0) {
        printf("Failed to start ICM42670.\n");
        return;
    }

    // Käynnistetään IMU:n kiihtyvyys mittaus oletusarvoilla.
    ICM42670_start_with_default_values();

    // Luodaan muuttujat datan lukemista varten
    float ax, ay, az, gx, gy, gz, t;

    // Pääsilmukka, joka lukee jatkuvasti IMU-anturin dataa
    // ja tulkitsee laitteen asennon akselien arvojen perusteella
    while(1){
        tight_loop_contents();
        // Luetaan sensorin mittausdata: 
        // ax, ay, az = kiihtyvyydet (g) ja gx, gy, gz = kulmannopeudet (°/s), t = lämpötila
        ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t);

        //printf("ax: %.2f\tay: %.2f\taz: %.2f\n", ax, ay, az);

        // Jos laite on kallistettu positiiviseen x-akselin suuntaan --> tulostetaan piste
        if (ax > X_POS_THRESHOLD) {
            printf(".");
        }
        // Jos laite on kallistettuna negatiivisen x-akselin suuntaa --> tulostetaan viiva
        else if (ax < X_NEG_THRESHOLD) {
            printf("-");
        }
        // Jos laite on kallistettu postiivisen z-akselin suuntaan --> tulostetaan välilyönti
        else if (az > Z_POS_THRESHOLD) {
            printf(" ");
        }
        // Jos laite on kallistunut negatiivisen z-akselin suuntaan --> tulostetana kaksi välilyöntiä ja rivinvaihto.
        else if (az > Z_NEG_THRESHOLD) {
            printf("  \n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// =============================================
// PRINT_TASK (Testi)
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
    sleep_ms(3000);          // Pieni viive, jotta alustukset ehtivät valmistua
    printf("Aloitetaan uusi viesti\n");

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

    if (false /* printTask päälle/pois */) {
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
    }

    // Käynnistetään FreeRTOS:n ajastin, joka suorittaa tehtävät
    vTaskStartScheduler();

    return 0;
}

