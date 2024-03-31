#include "marker_event_loop.h"

static const char* TAG = "marker_event_loop";

esp_event_loop_handle_t marker_event_loop;

TaskHandle_t event_loop_task_handle;

ESP_EVENT_DEFINE_BASE(MARKER_EVENT);


static void event_loop_task(void* arg)
{
    ESP_LOGI(TAG, "Event loop task started");

    while (1)
    {
        vTaskDelay(10);
    }
}


esp_err_t marker_event_loop_init()
{
    /* esp_event_loop_args_t marker_event_loop_args = {
        .queue_size = 10,
        .task_name = "marker_event_loop",
        .task_stack_size = 3072,
        .task_priority = uxTaskPriorityGet(NULL) + CONFIG_MARKER_EVENT_LOOP_TASK_PRIORITY,
        .task_core_id = tskNO_AFFINITY
    }; */

    esp_event_loop_args_t marker_event_loop_args = {
        .queue_size = 10,
        .task_name = NULL
    };

    // create events loop for marker events loops.
    ESP_RETURN_ON_ERROR(esp_event_loop_create(&marker_event_loop_args, &marker_event_loop), TAG, "Failed to create marker event loop");
    ESP_LOGI(TAG, "Marker event loop created");

    // start task for event loop. event loop generates events by calling underlying event sources.
    ESP_LOGI(TAG, "Starting event loop task");
    xTaskCreate(event_loop_task, "event_loop_task", 3072, NULL, uxTaskPriorityGet(NULL) + CONFIG_MARKER_EVENT_LOOP_TASK_PRIORITY, &event_loop_task_handle);

    return ESP_OK;
}

esp_event_loop_handle_t marker_event_loop_get()
{
    assert(marker_event_loop != NULL);
    
    return marker_event_loop;
}

esp_err_t marker_event_loop_trigger(int32_t event_id, const void* event_data, size_t event_data_size, TickType_t ticks_to_wait)
{
    if (marker_event_loop == NULL) {
        ESP_LOGE(TAG, "Marker event loop is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    return esp_event_post_to(marker_event_loop, MARKER_EVENT, event_id, event_data, event_data_size, ticks_to_wait);
}

esp_err_t marker_event_loop_on(int32_t event_id, esp_event_handler_t event_handler, void *event_handler_arg)
{
    if (marker_event_loop == NULL) {
        ESP_LOGE(TAG, "Marker event loop is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    return esp_event_handler_instance_register_with(marker_event_loop, MARKER_EVENT, event_id, event_handler, event_handler_arg, NULL);
}
