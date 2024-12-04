// Copyright (C) 2024 Vincent Hamp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/// Config
///
/// \file   config.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <driver/rmt_tx.h>
#include <esp_http_server.h>
#include <esp_task.h>
#include <freertos/message_buffer.h>
#include <freertos/queue.h>
#include <freertos/stream_buffer.h>
#include <hal/gpio_types.h>
#include <static_math/static_math.h>
#include <array>
#include <atomic>
#include <climits>
#include <dcc/dcc.hpp>
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <string>
#include <ztl/enum.hpp>
#include <ztl/implicit_wrapper.hpp>
#include <ztl/limits.hpp>

#if CONFIG_IDF_TARGET_ESP32S3
#  include <driver/gptimer.h>
#  include <hal/adc_types.h>
#elif CONFIG_IDF_TARGET_LINUX
#  define ADC_CHANNEL_1 1
#  define ADC_CHANNEL_2 2
#  define ADC_CHANNEL_3 3
#  define ADC_CHANNEL_4 4
#  define ADC_CHANNEL_5 5
#  define ADC_CHANNEL_6 6
#  define ADC_CHANNEL_7 7
#  define ADC_CHANNEL_8 8
#  define ADC_CHANNEL_9 9
#  define ADC_ATTEN_DB_0 0
#  define ADC_ATTEN_DB_2_5 1
#  define ADC_ATTEN_DB_6 2
#  define ADC_ATTEN_DB_11 3
#  define SOC_ADC_PATT_LEN_MAX 24
#  define SOC_ADC_DIGI_MAX_BITWIDTH 12
#  define SOC_ADC_DIGI_DATA_BYTES_PER_CONV 4
#  define SOC_ADC_SAMPLE_FREQ_THRES_HIGH 83333
#  define GPIO_NUM_1 static_cast<gpio_num_t>(1)
#  define GPIO_NUM_2 static_cast<gpio_num_t>(2)
#  define GPIO_NUM_3 static_cast<gpio_num_t>(3)
#  define GPIO_NUM_4 static_cast<gpio_num_t>(4)
#  define GPIO_NUM_5 static_cast<gpio_num_t>(5)
#  define GPIO_NUM_6 static_cast<gpio_num_t>(6)
#  define GPIO_NUM_7 static_cast<gpio_num_t>(7)
#  define GPIO_NUM_8 static_cast<gpio_num_t>(8)
#  define GPIO_NUM_9 static_cast<gpio_num_t>(9)
#  define GPIO_NUM_10 static_cast<gpio_num_t>(10)
#  define GPIO_NUM_11 static_cast<gpio_num_t>(11)
#  define GPIO_NUM_12 static_cast<gpio_num_t>(12)
#  define GPIO_NUM_13 static_cast<gpio_num_t>(13)
#  define GPIO_NUM_14 static_cast<gpio_num_t>(14)
#  define GPIO_NUM_15 static_cast<gpio_num_t>(15)
#  define GPIO_NUM_16 static_cast<gpio_num_t>(16)
#  define GPIO_NUM_17 static_cast<gpio_num_t>(17)
#  define GPIO_NUM_18 static_cast<gpio_num_t>(18)
#  define GPIO_NUM_19 static_cast<gpio_num_t>(19)
#  define GPIO_NUM_20 static_cast<gpio_num_t>(20)
#  define GPIO_NUM_21 static_cast<gpio_num_t>(21)
#  define GPIO_NUM_22 static_cast<gpio_num_t>(22)
#  define GPIO_NUM_23 static_cast<gpio_num_t>(23)
#  define GPIO_NUM_24 static_cast<gpio_num_t>(24)
#  define GPIO_NUM_25 static_cast<gpio_num_t>(25)
#  define GPIO_NUM_26 static_cast<gpio_num_t>(26)
#  define GPIO_NUM_27 static_cast<gpio_num_t>(27)
#  define GPIO_NUM_28 static_cast<gpio_num_t>(28)
#  define GPIO_NUM_29 static_cast<gpio_num_t>(29)
#  define GPIO_NUM_30 static_cast<gpio_num_t>(30)
#  define GPIO_NUM_31 static_cast<gpio_num_t>(31)
#  define GPIO_NUM_32 static_cast<gpio_num_t>(32)
#  define GPIO_NUM_33 static_cast<gpio_num_t>(33)
#  define GPIO_NUM_34 static_cast<gpio_num_t>(34)
#  define GPIO_NUM_35 static_cast<gpio_num_t>(35)
#  define GPIO_NUM_36 static_cast<gpio_num_t>(36)
#  define GPIO_NUM_37 static_cast<gpio_num_t>(37)
#  define GPIO_NUM_38 static_cast<gpio_num_t>(38)
#  define GPIO_NUM_39 static_cast<gpio_num_t>(39)
#  define GPIO_NUM_40 static_cast<gpio_num_t>(40)
#  define GPIO_NUM_41 static_cast<gpio_num_t>(41)
#  define GPIO_NUM_42 static_cast<gpio_num_t>(42)
#  define GPIO_NUM_43 static_cast<gpio_num_t>(43)
#  define GPIO_NUM_44 static_cast<gpio_num_t>(44)
#  define GPIO_NUM_45 static_cast<gpio_num_t>(45)
#  define GPIO_NUM_46 static_cast<gpio_num_t>(46)
#  define GPIO_NUM_47 static_cast<gpio_num_t>(47)
#  define GPIO_NUM_48 static_cast<gpio_num_t>(48)
#else
#  error "Unsupported SOC"
#endif

/// Default task notification index
///
/// Index 0 is used by the Stream and Message Buffer implementation as indicated
/// by the following warning from the [Task Notifications
/// documentation](https://www.freertos.org/RTOS-task-notifications.html).
///
/// \warning
/// FreeRTOS Stream and Message Buffers use the task notification at array
/// index 0. If you want to maintain the state of a task notification across a
/// call to a Stream or Message Buffer API function then use a task notification
/// at an array index greater than 0.
inline constexpr auto default_notify_index{tskDEFAULT_INDEX_TO_NOTIFY + 1u};
static_assert(configTASK_NOTIFICATION_ARRAY_ENTRIES > 1);

inline constexpr auto bug_led_gpio_num{GPIO_NUM_48};

enum class State : uint16_t {
  // Flags (8 bits)
  Suspended = 0u << 0u,          ///<
  Suspend = !Suspended << 0u,    ///<
  ShortCircuit = Suspend << 1u,  ///<

  // Outputs
  DCCOperations = 1u << CHAR_BIT,  ///<
  DCCService = 2u << CHAR_BIT,     ///<
  DECUPZpp = 3u << CHAR_BIT,       ///<
  DECUPZsu = 4u << CHAR_BIT,       ///<
  MDUZpp = 5u << CHAR_BIT,         ///<
  MDUZsu = 6u << CHAR_BIT,         ///<
  ZUSI = 7u << CHAR_BIT,           ///<

  // USB protocols
  DCC_EIN = 8u << CHAR_BIT,    ///<
  DECUP_EIN = 9u << CHAR_BIT,  ///<
  MDU_EIN = 10u << CHAR_BIT,   ///<
  SUSIV2 = 11u << CHAR_BIT,    ///<

  // System
  OTA = 12u << CHAR_BIT,  ///<
};
static_assert(std::to_underlying(State::OTA) < MAGIC_ENUM_RANGE_MAX);

ZTL_MAKE_ENUM_CLASS_FLAGS(State)

/// Restricts access to low-level tasks
inline std::atomic<State> state{State::Suspended};

namespace analog {

inline constexpr auto ol_on_gpio_num{GPIO_NUM_17};

/// Voltage divider upper resistor for voltage measurement
inline constexpr auto voltage_upper_r{14300};

/// Voltage divider lower resistor for voltage measurement
inline constexpr auto voltage_lower_r{470};

/// Current sense resistor
inline constexpr auto current_r{180};

/// Current sense ratio
inline constexpr auto current_k{800};

inline constexpr auto vref{1000};
inline constexpr auto max_measurement{smath::pow(2, SOC_ADC_DIGI_MAX_BITWIDTH) -
                                      1};
inline constexpr auto voltage_channel{ADC_CHANNEL_2};
inline constexpr auto current_channel{ADC_CHANNEL_9};
inline constexpr auto attenuation{ADC_ATTEN_DB_0};
inline constexpr std::array channels{current_channel, voltage_channel};

/// Sample frequency [Hz] (sample takes 100µs, conversion frame 20ms)
inline constexpr auto sample_freq_hz{10000u};

/// Number of samples per frame
inline constexpr auto conversion_frame_samples{200uz};

/// Time per frame [ms]
inline constexpr auto conversion_frame_time{(conversion_frame_samples * 1000u) /
                                            sample_freq_hz};
static_assert(conversion_frame_time == 20u);

inline constexpr auto conversion_frame_size{conversion_frame_samples *
                                            SOC_ADC_DIGI_DATA_BYTES_PER_CONV};
inline constexpr auto conversion_frame_samples_per_channel{
  conversion_frame_samples / size(channels)};
static_assert(size(channels) < SOC_ADC_PATT_LEN_MAX);

///
inline struct AdcTask {
  static constexpr auto name{"analog::adc"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 2u};
  static constexpr auto timeout{200u};
  TaskHandle_t handle{};
} adc_task;

///
inline struct TempTask {
  static constexpr auto name{"analog::temp"};
  static constexpr auto stack_size{2048uz};
  static constexpr UBaseType_t priority{tskIDLE_PRIORITY};
  static constexpr auto timeout{1000u};
  TaskHandle_t handle{};
} temp_task;

using VoltageMeasurement =
  ztl::implicit_wrapper<ztl::smallest_signed_t<0, max_measurement>,
                        struct VoltageMeasurementTag>;

using Voltage =
  ztl::implicit_wrapper<VoltageMeasurement::value_type, struct VoltageTag>;

using CurrentMeasurement =
  ztl::implicit_wrapper<ztl::smallest_signed_t<0, max_measurement>,
                        struct CurrentMeasurementTag>;

using Current =
  ztl::implicit_wrapper<CurrentMeasurement::value_type, struct CurrentTag>;

///
inline struct VoltagesQueue {
  using value_type =
    std::array<VoltageMeasurement, conversion_frame_samples_per_channel>;
  static constexpr auto size{1uz};
  QueueHandle_t handle{};
} voltages_queue;

///
inline struct CurrentsQueue {
  using value_type =
    std::array<CurrentMeasurement, conversion_frame_samples_per_channel>;
  static constexpr auto size{1uz};
  QueueHandle_t handle{};
} currents_queue;

///
inline struct TemperatureQueue {
  using value_type = float;
  static constexpr auto size{1uz};
  QueueHandle_t handle{};
} temperature_queue;

}  // namespace analog

namespace dcc {

inline constexpr auto priority_bits{5u};

class Service;
inline std::shared_ptr<Service> service;

///
inline struct Task {
  static constexpr auto name{"dcc"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{2u};
  static constexpr auto timeout{100u};
  TaskHandle_t handle{};
} task;

}  // namespace dcc

namespace decup {

///
inline struct Task {
  static constexpr auto name{"decup"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{2u};
  TaskHandle_t handle{};
} task;

}  // namespace decup

namespace http {

/// Handle to server instance
inline httpd_handle_t handle{};

///
inline constexpr auto stack_size{6144uz};

namespace sta {

class Server;
inline std::shared_ptr<Server> server;

}  // namespace sta

}  // namespace http

namespace mdu {

///
inline struct Task {
  static constexpr auto name{"mdu"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{2u};
  TaskHandle_t handle{};
} task;

}  // namespace mdu

namespace ota {

///
inline constexpr uint8_t ack{0x06u};

///
inline constexpr uint8_t nak{0x15u};

///
inline struct Task {
  static constexpr auto name{"ota"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 1u};
  TaskHandle_t handle{};
} task;

}  // namespace ota

namespace zusi {

/// \todo ESP_TASK_PRIO_MAX
inline struct Task {
  static constexpr auto name{"zusi"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{2u};
  TaskHandle_t handle{};
} task;

}  // namespace zusi

namespace out {

#if !CONFIG_IDF_TARGET_LINUX
inline gptimer_handle_t gptimer{};
#endif

///
inline struct RxMessageBuffer {
  static constexpr auto size{320uz};
  MessageBufferHandle_t handle{};
} rx_message_buffer;

///
inline struct TxMessageBuffer {
  static constexpr auto size{320uz};
  MessageBufferHandle_t front_handle{};
  MessageBufferHandle_t back_handle{};
} tx_message_buffer;

namespace track {

enum class CurrentLimit : uint8_t {
  _500mA = 0b00u,
  _1300mA = 0b01u,
  _2700mA = 0b10u,
  _4100mA = 0b11u
};

/// Continuous transmission requires at least a depth of 2
inline constexpr auto trans_queue_depth{2uz};

inline constexpr auto p_gpio_num{GPIO_NUM_11};
inline constexpr auto n_force_low_gpio_num{GPIO_NUM_9};

inline constexpr auto ack_gpio_num{GPIO_NUM_18};
inline constexpr auto nsleep_gpio_num{GPIO_NUM_8};
inline constexpr auto ilim0_gpio_num{GPIO_NUM_15};
inline constexpr auto ilim1_gpio_num{GPIO_NUM_16};
inline constexpr auto nfault_gpio_num{GPIO_NUM_46};
inline constexpr auto enable_gpio_num{GPIO_NUM_12};

///
inline struct RxQueue {
  struct value_type {
    dcc::Packet packet{};
    dcc::bidi::Datagram<> datagram{};
  };
  static constexpr auto size{32uz};
  QueueHandle_t handle{};
} rx_queue;

inline rmt_channel_handle_t channel{};
inline rmt_encoder_handle_t encoder{};

namespace dcc {

inline constexpr auto bidi_rx_gpio_num{GPIO_NUM_14};
inline constexpr auto bidi_en_gpio_num{GPIO_NUM_13};

///
inline struct Task {
  static constexpr auto name{"out::track::dcc"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 1u};
  TaskHandle_t handle{};
} task;

}  // namespace dcc

namespace decup {

///
inline struct Task {
  static constexpr auto name{"out::track::decup"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 1u};
  TaskHandle_t handle{};
} task;

}  // namespace decup

namespace mdu {

///
inline struct Task {
  static constexpr auto name{"out::track::mdu"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 1u};
  TaskHandle_t handle{};
} task;

}  // namespace mdu

}  // namespace track

namespace zusi {

inline constexpr auto enable_gpio_num{GPIO_NUM_4};
inline constexpr auto clock_gpio_num{GPIO_NUM_6};
inline constexpr auto data_gpio_num{GPIO_NUM_5};

///
inline struct Task {
  static constexpr auto name{"out::zusi"};
  static constexpr auto stack_size{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 1u};
  TaskHandle_t handle{};
} task;

}  // namespace zusi

}  // namespace out

namespace udp {

inline constexpr uint16_t port{21105u};
inline int sock_fd;

}  // namespace udp

namespace usb {

inline constexpr auto vbus_gpio_num{GPIO_NUM_7};
inline constexpr auto buffer_size{512uz};

///
inline struct RxTask {
  static constexpr auto name{"usb::rx"};
  static constexpr auto stack_size{3072uz};
  static constexpr UBaseType_t priority{5u};
  static constexpr auto timeout{100u};
  TaskHandle_t handle{};
} rx_task;

///
inline struct TxTask {
  static constexpr auto name{"usb::tx"};
  static constexpr auto stack_size{3072uz};
  static constexpr UBaseType_t priority{1u};
  static constexpr auto timeout{20u};
  TaskHandle_t handle{};
} tx_task;

///
inline struct RxStreamBuffer {
  static constexpr auto size{buffer_size};
  StreamBufferHandle_t handle{};
} rx_stream_buffer;

///
inline struct TxStreamBuffer {
  static constexpr auto size{buffer_size};
  StreamBufferHandle_t handle{};
} tx_stream_buffer;

namespace dcc_ein {

///
inline struct Task {
  static constexpr auto name{"usb::dcc_ein"};
  static constexpr auto stack_size{3072uz};
  static constexpr UBaseType_t priority{::usb::rx_task.priority};
  static constexpr auto timeout{100u};
  TaskHandle_t handle{};
} task;

}  // namespace dcc_ein

namespace decup_ein {

///
inline struct Task {
  static constexpr auto name{"usb::decup_ein"};
  static constexpr auto stack_size{3072uz};
  static constexpr UBaseType_t priority{::usb::rx_task.priority};
  TaskHandle_t handle{};
} task;

}  // namespace decup_ein

namespace susiv2 {

/// SUSIV2 buffer size
///
/// Biggest command received by SUSIV2. Don't change since receive logic relies
/// on this.
inline constexpr auto buffer_size{268uz};

///
inline struct Task {
  static constexpr auto name{"usb::susiv2"};
  static constexpr auto stack_size{3072uz};
  static constexpr UBaseType_t priority{::usb::rx_task.priority};
  static constexpr auto timeout{100u};
  TaskHandle_t handle{};
} task;

}  // namespace susiv2

}  // namespace usb

namespace wifi {

inline constexpr auto led_gpio_num{GPIO_NUM_47};

inline std::string mdns_str;
inline std::string ip_str;
inline std::array<uint8_t, 6uz> mac;
inline std::string mac_str(2uz * 6uz + 5uz + sizeof('\n'), '\0');

///
inline struct ApRecordsQueue {
  QueueHandle_t handle{};
} ap_records_queue;

}  // namespace wifi

namespace z21 {

///
inline struct RxTask {
  static constexpr auto name{"z21"};
  static constexpr auto stack_size{6144uz};
  static constexpr UBaseType_t priority{5u};
  static constexpr auto timeout{500u};
  TaskHandle_t handle{};
} task;

class Service;
inline std::shared_ptr<Service> service;

}  // namespace z21