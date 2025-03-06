#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ledc_buzzer.h"

#define TAG "main"
#define CORE0  0
// only define xCoreID CORE1 as 1 if this is a multiple core processor target, else define it as tskNO_AFFINITY
#define CORE1  ((CONFIG_FREERTOS_NUMBER_OF_CORES > 1) ? 1 : tskNO_AFFINITY)


static TaskHandle_t songPlayTaskHandler = NULL;

static void play_song(const char *song);

static void song_play_task(void *pvParameters) {
    const char *song = (const char *)pvParameters;

    while(1) {
        play_song(song); 
        vTaskDelay(pdMS_TO_TICKS(2000)); // Repeat delay
    }
}

static void play_song(const char *song) {
    // Create a writable copy of the song
    char *song_buf = malloc(strlen(song) + 1);
    if (!song_buf) {
        ESP_LOGE(TAG, "Failed to allocate song buffer");
        return;
    }
    strcpy(song_buf, song);

    char *saveptr = NULL;
    char *token = strtok_r(song_buf, " ", &saveptr);
    
    while (token != NULL) {
        char note[4];
        int duration;
        if (sscanf(token, "%3[^-]-%d", note, &duration) == 2) {
            buzzer_play_note(note, duration);
        } else {
            ESP_LOGW(TAG, "Invalid note format: %s", token);
        }
        token = strtok_r(NULL, " ", &saveptr);
    }
    
    free(song_buf);  // Free the allocated buffer
    buzzer_deinit(); // De-initialize buzzer
    vTaskDelete(songPlayTaskHandler);  // Delete task, so the song is played only once.
}

void app_main(void) {
    ESP_ERROR_CHECK(buzzer_init());

    const char *twinkleTwinkleLittleStar = 
        "C4-500 C4-500 G4-500 G4-500 A4-500 A4-500 G4-1000 "
        "F4-500 F4-500 E4-500 E4-500 D4-500 D4-500 C4-1000";

    if (xTaskCreatePinnedToCore(
        song_play_task,                     // Task function
        "Music Player",                     // Task name
        4096,                               // Stack size
        (void *)twinkleTwinkleLittleStar,   // Parameter
        5,                                  // Priority (0-25)
        &songPlayTaskHandler,               // Task handle
        CORE1
    ) != pdPASS) 
    {
        ESP_LOGE(TAG, "failed to start the song_play_task.");
    } 

    // Main task continues running other code
    while(1) {
        // Handle other operations (WiFi, sensors, etc.)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}