#include "freertos.hpp"
#include <gtest/gtest.h>

void stream_buffer_delete_clear_handle(
  StreamBufferHandle_t& stream_buffer_handle) {
  if (!stream_buffer_handle) return;
  vStreamBufferDelete(stream_buffer_handle);
  stream_buffer_handle = NULL;
}

void message_buffer_delete_clear_handle(
  MessageBufferHandle_t& message_buffer_handle) {
  if (!message_buffer_handle) return;
  vMessageBufferDelete(message_buffer_handle);
  message_buffer_handle = NULL;
}

void queue_delete_clear_handle(QueueHandle_t& queue_handle) {
  if (!queue_handle) return;
  vQueueDelete(queue_handle);
  queue_handle = NULL;
}

TEST(freertos, xMessageBufferSpacesAvailable) {
  using namespace drv::out;
  tx_message_buffer.front_handle = xMessageBufferCreate(tx_message_buffer.size);
  EXPECT_EQ(xMessageBufferSpacesAvailable(tx_message_buffer.front_handle),
            tx_message_buffer.size);
  std::array<uint8_t, 42uz> space{};
  xMessageBufferSend(
    drv::out::tx_message_buffer.front_handle, data(space), size(space), 0u);
  EXPECT_EQ(xMessageBufferSpacesAvailable(tx_message_buffer.front_handle),
            tx_message_buffer.size - (size(space) + sizeof(void*)));
  message_buffer_delete_clear_handle(tx_message_buffer.front_handle);
}
