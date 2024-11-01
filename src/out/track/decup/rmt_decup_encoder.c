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

/// RMT DECUP encoder
///
/// \file   rmt_decup_encoder.c
/// \author Vincent Hamp
/// \date   14/08/2024

#include "rmt_decup_encoder.h"
#include <esp_attr.h>
#include <esp_check.h>
#include <esp_heap_caps.h>
#include <limits.h>

#if __has_include(<esp_linux_helper.h>)
#  include <esp_linux_helper.h>
#endif

// https://github.com/espressif/esp-idf/issues/13032
#if !defined(RMT_MEM_ALLOC_CAPS)
#  if CONFIG_RMT_ISR_IRAM_SAFE || CONFIG_RMT_RECV_FUNC_IN_IRAM
#    define RMT_MEM_ALLOC_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#  else
#    define RMT_MEM_ALLOC_CAPS MALLOC_CAP_DEFAULT
#  endif
#endif

static char const* TAG = "rmt";

/// DECUP encoder
typedef struct {
  rmt_encoder_t base;
  rmt_encoder_t* copy_encoder;
  rmt_symbol_word_t one_symbol;
  rmt_symbol_word_t zero_symbol;
  size_t num_symbols;
} rmt_decup_encoder_t;

/// Encode the user data into RMT symbols and write into RMT memory
///
/// \param  encoder       Encoder handle
/// \param  channel       RMT TX channel handle
/// \param  primary_data  App data to be encoded into RMT symbols
/// \param  data_size     Size of primary_data, in bytes
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_decup(rmt_encoder_t* encoder,
                                         rmt_channel_handle_t channel,
                                         void const* primary_data,
                                         size_t data_size,
                                         rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  rmt_encode_state_t session_state = RMT_ENCODING_RESET;
  rmt_decup_encoder_t* decup_encoder =
    __containerof(encoder, rmt_decup_encoder_t, base);
  rmt_encoder_handle_t copy_encoder = decup_encoder->copy_encoder;

  while (decup_encoder->num_symbols < data_size * (1u + CHAR_BIT + 1u)) {
    size_t const i = decup_encoder->num_symbols % (1u + CHAR_BIT + 1u);

    // Start
    if (i == 0u) {
      size_t const tmp = copy_encoder->encode(copy_encoder,
                                              channel,
                                              &decup_encoder->zero_symbol,
                                              sizeof(rmt_symbol_word_t),
                                              &session_state);
      encoded_symbols += tmp;
      decup_encoder->num_symbols += tmp;
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
    }
    // Data
    else if (i < (1u + CHAR_BIT)) {
      uint8_t const* data = (uint8_t const*)primary_data;
      size_t const byte_index =
        decup_encoder->num_symbols / (1u + CHAR_BIT + 1u);
      bool const bit = data[byte_index] & (1u << (i - 1u));
      size_t const tmp = copy_encoder->encode(copy_encoder,
                                              channel,
                                              bit ? &decup_encoder->one_symbol
                                                  : &decup_encoder->zero_symbol,
                                              sizeof(rmt_symbol_word_t),
                                              &session_state);
      encoded_symbols += tmp;
      decup_encoder->num_symbols += tmp;
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
    }
    // Stop
    else {
      size_t const tmp = copy_encoder->encode(copy_encoder,
                                              channel,
                                              &decup_encoder->one_symbol,
                                              sizeof(rmt_symbol_word_t),
                                              &session_state);
      encoded_symbols += tmp;
      decup_encoder->num_symbols += tmp;
      if (session_state & RMT_ENCODING_COMPLETE &&
          decup_encoder->num_symbols >= data_size * (1u + CHAR_BIT + 1u)) {
        state |= RMT_ENCODING_COMPLETE;
        decup_encoder->num_symbols = 0u;
        break;
      }
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
    }
  }

  *ret_state = state;
  return encoded_symbols;
}

/// Delete RMT DECUP encoder
///
/// \param  encoder             RMT encoder handle
/// \retval ESP_OK              Delete RMT DECUP encoder successfully
/// \retval ESP_ERR_INVALID_ARG Delete RMT DECUP encoder failed because of
///                             invalid argument
/// \retval ESP_FAIL            Delete RMT DECUP encoder failed because of other
///                             error
static esp_err_t rmt_del_decup_encoder(rmt_encoder_t* encoder) {
  rmt_decup_encoder_t* decup_encoder =
    __containerof(encoder, rmt_decup_encoder_t, base);
  rmt_del_encoder(decup_encoder->copy_encoder);
  free(decup_encoder);
  return ESP_OK;
}

/// Reset RMT DECUP encoder
///
/// \param  encoder             RMT encoder handle
/// \retval ESP_OK              Reset RMT DECUP encoder successfully
/// \retval ESP_ERR_INVALID_ARG Reset RMT DECUP encoder failed because of
///                             invalid argument
/// \retval ESP_FAIL            Reset RMT DECUP encoder failed because of other
///                             error
static esp_err_t rmt_decup_encoder_reset(rmt_encoder_t* encoder) {
  rmt_decup_encoder_t* decup_encoder =
    __containerof(encoder, rmt_decup_encoder_t, base);
  rmt_encoder_reset(decup_encoder->copy_encoder);
  decup_encoder->num_symbols = 0u;
  return ESP_OK;
}

/// Create RMT DECUP encoder which encodes DECUP byte stream into RMT symbols
///
/// \param  config              DECUP encoder configuration
/// \param  ret_encoder         Returned encoder handle
/// \retval ESP_OK              Create RMT DECUP encoder successfully
/// \retval ESP_ERR_INVALID_ARG Create RMT DECUP encoder failed because of
///                             invalid argument
/// \retval ESP_ERR_NO_MEM      Create RMT DECUP encoder failed because out of
///                             memory
/// \retval ESP_FAIL            Create RMT DECUP encoder failed because of other
///                             error
esp_err_t rmt_new_decup_encoder(decup_encoder_config_t const* config,
                                rmt_encoder_handle_t* ret_encoder) {
  esp_err_t ret = ESP_OK;
  rmt_decup_encoder_t* decup_encoder = NULL;
  ESP_GOTO_ON_FALSE(
    config && ret_encoder, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");

  decup_encoder =
    heap_caps_calloc(1, sizeof(rmt_decup_encoder_t), RMT_MEM_ALLOC_CAPS);
  ESP_GOTO_ON_FALSE(
    decup_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for decup encoder");

  decup_encoder->base.encode = rmt_encode_decup;
  decup_encoder->base.del = rmt_del_decup_encoder;
  decup_encoder->base.reset = rmt_decup_encoder_reset;

  rmt_copy_encoder_config_t copy_encoder_config = {};
  ESP_GOTO_ON_ERROR(
    rmt_new_copy_encoder(&copy_encoder_config, &decup_encoder->copy_encoder),
    err,
    TAG,
    "create copy encoder failed");

  // Setup RMT symbols
  decup_encoder->one_symbol = (rmt_symbol_word_t){
    .duration0 = (uint16_t)(1e6 / 38400 / 2),
    .level0 = 1u ^ config->flags.invert,
    .duration1 = (uint16_t)(1e6 / 38400 / 2),
    .level1 = 1u ^ config->flags.invert,
  };
  decup_encoder->zero_symbol = (rmt_symbol_word_t){
    .duration0 = (uint16_t)(1e6 / 38400 / 2),
    .level0 = 0u ^ config->flags.invert,
    .duration1 = (uint16_t)(1e6 / 38400 / 2),
    .level1 = 0u ^ config->flags.invert,
  };

  *ret_encoder = &decup_encoder->base;
  return ESP_OK;
err:
  if (decup_encoder) {
    if (decup_encoder->copy_encoder)
      rmt_del_encoder(decup_encoder->copy_encoder);
    free(decup_encoder);
  }
  return ret;
}