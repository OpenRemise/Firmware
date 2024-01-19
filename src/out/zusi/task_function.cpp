/// ZUSI task function
///
/// \file   out/zusi/task_function.cpp
/// \author Vincent Hamp
/// \date   27/03/2023

#include "init.hpp"
#include "log.h"
#include "resume.hpp"
#include "suspend.hpp"
#include "zpp_load.hpp"

namespace out::zusi {

using namespace ::zusi;

/// TODO
std::optional<std::span<uint8_t>> receive_command(std::span<uint8_t> stack) {
  //
  if (auto const bytes_received{
        xMessageBufferReceive(tx_message_buffer.front_handle,
                              data(stack),
                              size(stack),
                              pdMS_TO_TICKS(task.timeout))})
    return stack.subspan(0uz, bytes_received);
  //
  else return std::nullopt;
}

/// TODO
void transmit(std::span<uint8_t const> stack) {
  xMessageBufferSend(out::rx_message_buffer.handle,
                     data(stack),
                     size(stack),
                     pdMS_TO_TICKS(task.timeout));
}

/// TODO
void loop() {
  ::zusi::Buffer<tx_message_buffer.size> stack;

  // Give decoder some time to boot...
  vTaskDelay(pdMS_TO_TICKS(1000u));

  ZppLoad zpp_load;
  zpp_load.enter();

  while (auto const cmd{receive_command(stack)}) {
    auto const i{data2uint32(&stack[2uz])};

    switch (static_cast<Command>(stack[0uz])) {
      case Command::None: break;

      case Command::CvRead:
        if (auto const cv{zpp_load.readCv(i)}) {
          stack[0uz] = ack;
          stack[1uz] = *cv;
          stack[2uz] = crc8(stack[1uz]);
          transmit({cbegin(stack), 3uz});
        } else transmit({&nak, 1uz});
        break;

      case Command::CvWrite:
        stack[0uz] = zpp_load.writeCv(i, stack[6uz]) ? ack : nak;
        transmit({cbegin(stack), 1uz});
        break;

      case Command::ZppErase:
        stack[0uz] = zpp_load.eraseZpp() ? ack : nak;
        transmit({cbegin(stack), 1uz});
        break;

      case Command::ZppWrite:
        stack[0uz] = zpp_load.writeZpp(i, {&stack[6uz], 256uz}) ? ack : nak;
        transmit({cbegin(stack), 1uz});
        break;

      case Command::Features:
        if (auto const features{zpp_load.features()}) {
          auto first{begin(stack)};
          auto last{first};
          *last++ = ack;
          last = std::copy(cbegin(*features), cend(*features), last);
          *last = crc8({first, last});
          transmit({first, 6uz});
        } else transmit({&nak, 1uz});
        break;

      case Command::Exit:
        stack[0uz] = zpp_load.exit(stack[3uz]) ? ack : nak;
        transmit({cbegin(stack), 1uz});
        if (stack[0uz] == ack) {
          vTaskDelay(pdMS_TO_TICKS(1000u));
          return;
        }
        break;

      case Command::Encrypt:
        LOGW("'Encrypt' command not implemented");  // TODO
        break;
    }
  }
}

/// TODO
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);
    ESP_ERROR_CHECK(resume());
    loop();
    ESP_ERROR_CHECK(suspend());
  }
}

}  // namespace out::zusi