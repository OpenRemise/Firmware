///
///
/// \file   analog/convert.cpp
/// \author Vincent Hamp
/// \date   03/05/2023

#include "convert.hpp"
#include <driver/gpio.h>

namespace analog {

namespace {

CurrentRange current_range{CurrentRange::Invalid};

/// TODO
void gpio_output(gpio_num_t gpio_num) {
  gpio_config_t const io_conf{.pin_bit_mask = 1ull << gpio_num,
                              .mode = GPIO_MODE_OUTPUT,
                              .pull_up_en = GPIO_PULLUP_DISABLE,
                              .pull_down_en = GPIO_PULLDOWN_DISABLE,
                              .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);
  gpio_set_level(gpio_num, false);
}

/// TODO
void gpio_disable(gpio_num_t gpio_num) {
  gpio_config_t const io_conf{.pin_bit_mask = 1ull << gpio_num,
                              .mode = GPIO_MODE_DISABLE,
                              .pull_up_en = GPIO_PULLUP_DISABLE,
                              .pull_down_en = GPIO_PULLDOWN_DISABLE,
                              .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);
}

/// TODO
int raw2mV(int meas) {
  int retval;
  adc_cali_raw_to_voltage(cali_handle, meas, &retval);
  return retval;
}

}  // namespace

/// TODO
CurrentRange get_current_range() { return current_range; }

/// TODO
void set_current_range(CurrentRange range) {
  assert(range != CurrentRange::Invalid);
  if (range == current_range) return;
  else if (range == CurrentRange::High) {
    gpio_output(current_high_gpio_num);
    gpio_disable(current_low_gpio_num);
  } else {
    gpio_output(current_low_gpio_num);
    gpio_disable(current_high_gpio_num);
  }
  current_range = range;
}

/// TODO
Voltage measurement2mV(VoltageMeasurement meas) {
  return static_cast<Voltage>((raw2mV(meas) * (r1 + r2)) / r2);
}

/// TODO
VoltageMeasurement mV2measurement(Voltage mV) {
  return static_cast<VoltageMeasurement>((mV * r2 * max_measurement) /
                                         ((r1 + r2) * vref));
}

/// TODO
Current measurement2mA(CurrentMeasurement meas) {
  auto const ripropi{current_range == CurrentRange::High ? r13 : r14};
  return static_cast<Current>((raw2mV(meas) * 1'000'000) / (ripropi * aipropi));
}

/// TODO
CurrentMeasurement mA2measurement(Current mA) {
  auto const ripropi{current_range == CurrentRange::High ? r13 : r14};
  return static_cast<CurrentMeasurement>(
    (((mA * ripropi * aipropi) / 1'000'000) * max_measurement) / vref);
}

}  // namespace analog