#pragma once

#include <esp_task.h>

template<auto Unique =
           [](void*) {
             for (;;) vTaskDelay(pdMS_TO_TICKS(1000u));
           }>
auto task_create_stub(TaskHandle_t& task_handle) {
  return xTaskCreate(
    Unique, NULL, 4096uz, NULL, tskIDLE_PRIORITY, &task_handle);
}

void stream_buffer_delete_clear_handle(
  StreamBufferHandle_t& stream_buffer_handle);

void queue_delete_clear_handle(QueueHandle_t& queue_handle);
