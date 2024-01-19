#pragma once

#include <rmt_mdu_encoder.h>
#include <mdu/mdu.hpp>

namespace out::track::mdu {

/// TODO
constexpr std::pair<size_t, size_t>
ack1_indices(mdu_encoder_config_t const& encoder_config, size_t count) {
  auto const num_ackreq{encoder_config.num_ackreq};
  auto const ackreq{::mdu::timings[encoder_config.transfer_rate].ackreq};
  auto const countf{static_cast<float>(count)};
  auto const first{
    ceilf(countf - static_cast<float>((num_ackreq - 2u) * ackreq) /
                     analog::sample_time_per_channel_us)};
  auto const second{
    ceilf(countf - static_cast<float>(((num_ackreq - 5u)) * ackreq) /
                     analog::sample_time_per_channel_us)};
  return {first, second};
}

/// TODO
constexpr std::pair<size_t, size_t>
ack2_indices(mdu_encoder_config_t const& encoder_config, size_t count) {
  auto const num_ackreq{encoder_config.num_ackreq};
  auto const ackreq{::mdu::timings[encoder_config.transfer_rate].ackreq};
  auto const countf{static_cast<float>(count)};
  auto const first{
    ceilf(countf - static_cast<float>((num_ackreq - 6u) * ackreq) /
                     analog::sample_time_per_channel_us)};
  auto const second{
    ceilf(countf - static_cast<float>((num_ackreq - 9u) * ackreq) /
                     analog::sample_time_per_channel_us)};
  return {first, second};
}

void task_function(void*);

}  // namespace out::track::mdu