/// Config
///
/// \file   config.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <driver/rmt_tx.h>
#include <esp_task.h>
#include <freertos/message_buffer.h>
#include <freertos/queue.h>
#include <freertos/stream_buffer.h>
#include <hal/gpio_types.h>
#include <static_math/static_math.h>
#include <array>
#include <atomic>
#include <dcc_ein/dcc_ein.hpp>
#include <memory>
#include <string>
#include <ztl/implicit_wrapper.hpp>
#include <ztl/limits.hpp>

#if CONFIG_IDF_TARGET_ESP32S3
#  include <driver/gptimer.h>
#  include <hal/adc_types.h>
#elif CONFIG_IDF_TARGET_LINUX
#  define ADC_CHANNEL_7 7
#  define ADC_CHANNEL_9 9
#  define ADC_ATTEN_DB_0 0
#  define ADC_ATTEN_DB_2_5 1
#  define ADC_ATTEN_DB_6 2
#  define ADC_ATTEN_DB_11 3
#  define SOC_ADC_PATT_LEN_MAX 24
#  define SOC_ADC_DIGI_MAX_BITWIDTH 12
#  define SOC_ADC_DIGI_DATA_BYTES_PER_CONV 4
#  define SOC_ADC_SAMPLE_FREQ_THRES_HIGH 83333
#  define GPIO_NUM_45 GPIO_NUM_MAX
#  define GPIO_NUM_46 GPIO_NUM_MAX
#  define GPIO_NUM_47 GPIO_NUM_MAX
#  define GPIO_NUM_48 GPIO_NUM_MAX
#else
#  error "Unsupported SOC"
#endif

/// Default task notification index
///
/// Index 0 is used by the Stream and Message Buffer implementation as indicated
/// by the following warning from the [Task Notifications
/// documentation](https://www.freertos.org/RTOS-task-notifications.html).
///
///
/// \warning  FreeRTOS Stream and Message Buffers use the task notification at
///           array index 0. If you want to maintain the state of a task
///           notification across a call to a Stream or Message Buffer API
///           function then use a task notification at an array index greater
///           than 0.
inline constexpr auto default_notify_index{tskDEFAULT_INDEX_TO_NOTIFY + 1u};
static_assert(configTASK_NOTIFICATION_ARRAY_ENTRIES > 1);

inline constexpr auto a0_gpio_num{GPIO_NUM_5};
inline constexpr auto a1_gpio_num{GPIO_NUM_6};
inline constexpr auto a2_gpio_num{GPIO_NUM_7};
inline constexpr auto a3_gpio_num{GPIO_NUM_15};
inline constexpr auto a4_gpio_num{GPIO_NUM_16};
inline constexpr auto a5_gpio_num{GPIO_NUM_17};
inline constexpr auto d2_gpio_num{GPIO_NUM_10};
inline constexpr auto d3_gpio_num{GPIO_NUM_11};
inline constexpr auto d4_gpio_num{GPIO_NUM_12};
inline constexpr auto d5_gpio_num{GPIO_NUM_13};
inline constexpr auto d6_gpio_num{GPIO_NUM_14};
inline constexpr auto d7_gpio_num{GPIO_NUM_21};
inline constexpr auto d8_gpio_num{GPIO_NUM_45};
inline constexpr auto d9_gpio_num{GPIO_NUM_38};
inline constexpr auto d10_gpio_num{GPIO_NUM_39};
inline constexpr auto d11_gpio_num{GPIO_NUM_40};
inline constexpr auto d12_gpio_num{GPIO_NUM_41};
inline constexpr auto d13_gpio_num{GPIO_NUM_42};
inline constexpr auto d20_gpio_num{GPIO_NUM_2};
inline constexpr auto d21_gpio_num{GPIO_NUM_1};

inline constexpr auto bug_led_gpio_num{GPIO_NUM_48};

enum class Mode : uint8_t {
  //
  Suspended,
  Blocked,

  // USB protocols
  DCC_EIN,
  DECUP_EIN,
  MDUSNDPREP,
  SUSIV2,

  //
  DCCOperations,
  DCCService,

  //
  ZUSI,

  //
  MDUFirmware,
  MDUZpp,

  //
  OTA
};

/// Restricts access to low-level tasks
inline std::atomic<Mode> mode{Mode::Suspended};

///
constexpr uint8_t ack{0x06u};

///
constexpr uint8_t nak{0x15u};

namespace analog {

inline constexpr auto r1{14300};
inline constexpr auto r2{470};
inline constexpr auto rimon{120};
inline constexpr auto kimon{500};

inline constexpr auto vref{1000};
inline constexpr auto max_measurement{smath::pow(2, SOC_ADC_DIGI_MAX_BITWIDTH) -
                                      1};
inline constexpr auto current_channel{ADC_CHANNEL_4};
inline constexpr auto voltage_channel{ADC_CHANNEL_7};
inline constexpr auto attenuation{ADC_ATTEN_DB_0};
inline constexpr std::array channels{current_channel, voltage_channel};

inline constexpr auto conversion_frame_samples{200uz};
inline constexpr auto conversion_frame_size{conversion_frame_samples *
                                            SOC_ADC_DIGI_DATA_BYTES_PER_CONV};
inline constexpr auto conversion_frame_samples_per_channel{
  conversion_frame_samples / size(channels)};
static_assert(size(channels) < SOC_ADC_PATT_LEN_MAX);

/// Sample frequency [Hz] (sample takes 100µs, conversion frame 20ms)
inline constexpr auto sample_freq_hz{10000u};

///
inline struct AdcTask {
  static constexpr auto name{"analog::adc"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 2u};
  static constexpr auto timeout{200u};
  TaskHandle_t handle{};
} adc_task;

///
inline struct TempTask {
  static constexpr auto name{"analog::temp"};
  static constexpr auto stack_depth{2048uz};
  static constexpr UBaseType_t priority{tskIDLE_PRIORITY};
  static constexpr auto timeout{200u};
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

///
inline struct Task {
  static constexpr auto name{"dcc"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{2u};
  TaskHandle_t handle{};
} task;

}  // namespace dcc

namespace http {

namespace sta {

/// Size of internal RAM reserved for serving files from SPIFFS
inline constexpr auto file_buffer_size{16384uz};

class Server;
inline std::shared_ptr<Server> server;

}  // namespace sta

}  // namespace http

namespace mdu {

///
inline struct Task {
  static constexpr auto name{"mdu"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{2u};
  TaskHandle_t handle{};
} task;

}  // namespace mdu

namespace ota {

///
inline struct Task {
  static constexpr auto name{"ota"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 1u};
  TaskHandle_t handle{};
} task;

}  // namespace ota

namespace zusi {

///
inline struct Task {
  static constexpr auto name{"zusi"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{2u};  // TODO ESP_TASK_PRIO_MAX
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

enum class CurrentLimit {
  _500mA = 0b00u,
  _1600mA = 0b01u,
  _3000mA = 0b10u,
  _4100mA = 0b11u
};

/// Continuous transmission requires at least a depth of 2
inline constexpr auto trans_queue_depth{2uz};

inline constexpr auto ack_gpio_num{a1_gpio_num};
inline constexpr auto nsleep_gpio_num{d5_gpio_num};
inline constexpr auto isel0_gpio_num{d7_gpio_num};
inline constexpr auto isel1_gpio_num{d8_gpio_num};
inline constexpr auto in_gpio_num{d10_gpio_num};
inline constexpr auto force_low_gpio_num{d11_gpio_num};
inline constexpr auto nfault_gpio_num{d6_gpio_num};
inline constexpr auto enable_gpio_num{d12_gpio_num};

///
inline struct RxQueue {
  using value_type = dcc_ein::AddressedDatagram;
  static constexpr auto size{32uz};
  QueueHandle_t handle{};
} rx_queue;

inline rmt_channel_handle_t channel{};
inline rmt_encoder_handle_t encoder{};

namespace dcc {

inline constexpr auto bidi_rx_gpio_num{d9_gpio_num};
inline constexpr auto bidi_en_gpio_num{d13_gpio_num};

///
inline struct Task {
  static constexpr auto name{"out::track::dcc"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 1u};
  static constexpr auto timeout{500u};
  TaskHandle_t handle{};
} task;

}  // namespace dcc

namespace mdu {

///
inline struct Task {
  static constexpr auto name{"out::track::mdu"};
  static constexpr auto stack_depth{4096uz};
  // Must be lower than ADC to not starve it
  static constexpr UBaseType_t priority{analog::adc_task.priority - 1u};
  static constexpr auto timeout{10'000u};
  TaskHandle_t handle{};
} task;

}  // namespace mdu

}  // namespace track

namespace zusi {

inline constexpr auto enable_gpio_num{GPIO_NUM_18};
inline constexpr auto clock_gpio_num{GPIO_NUM_9};
inline constexpr auto data_gpio_num{GPIO_NUM_46};

///
inline struct Task {
  static constexpr auto name{"out::zusi"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{ESP_TASK_PRIO_MAX - 1u};
  static constexpr auto timeout{10'000u};
  TaskHandle_t handle{};
} task;

}  // namespace zusi

}  // namespace out

namespace udp {

inline struct Task {
  static constexpr auto name{"udp"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{5u};
  TaskHandle_t handle{};
} task;

}  // namespace udp

namespace usb {

inline constexpr auto vbus_gpio_num{GPIO_NUM_4};
inline constexpr auto buffer_size{512uz};

///
inline struct RxTask {
  static constexpr auto name{"usb::rx"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{5u};
  static constexpr auto timeout{100u};
  TaskHandle_t handle{};
} rx_task;

///
inline struct TxTask {
  static constexpr auto name{"usb::tx"};
  static constexpr auto stack_depth{4096uz};
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
inline struct RxTask {
  static constexpr auto name{"usb::dcc_ein::rx"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{::usb::rx_task.priority};
  static constexpr auto timeout{out::track::dcc::task.timeout};
  TaskHandle_t handle{};
} rx_task;

///
inline struct TxTask {
  static constexpr auto name{"usb::dcc_ein::tx"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{::usb::tx_task.priority - 1u};
  static constexpr auto timeout{100u};
  TaskHandle_t handle{};
} tx_task;

}  // namespace dcc_ein

namespace susiv2 {

/// SUSIV2 buffer size
///
/// Biggest command received by SUSIV2. Don't change since receive logic relies
/// on this.
inline constexpr auto buffer_size{268uz};

///
inline struct RxTask {
  static constexpr auto name{"usb::susiv2::rx"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{::usb::rx_task.priority};
  static constexpr auto timeout{::out::zusi::task.timeout};
  TaskHandle_t handle{};
} rx_task;

///
inline struct TxTask {
  static constexpr auto name{"usb::susiv2::tx"};
  static constexpr auto stack_depth{4096uz};
  static constexpr UBaseType_t priority{::usb::tx_task.priority - 1u};
  static constexpr auto timeout{100u};
  TaskHandle_t handle{};
} tx_task;

}  // namespace susiv2

}  // namespace usb

namespace wifi {

inline constexpr auto force_ap_init_gpio_num{GPIO_NUM_3};
inline constexpr auto led_gpio_num{GPIO_NUM_47};

inline std::string ip_str;
inline std::array<uint8_t, 6uz> mac;
inline std::string mac_str(2uz * 6uz + 5uz + sizeof('\n'), '\0');

///
inline struct ApRecordsQueue {
  QueueHandle_t handle{};
} ap_records_queue;

}  // namespace wifi