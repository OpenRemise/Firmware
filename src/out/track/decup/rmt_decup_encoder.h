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
/// \file   rmt_decup_encoder.h
/// \author Vincent Hamp
/// \date   14/08/2024

#pragma once

#include <driver/rmt_encoder.h>

#ifdef __cplusplus
extern "C" {
#endif

/// DECUP encoder configuration
typedef struct {
  struct {
    /// Invert RMT symbol level
    ///
    /// This boolean value corresponds to the level of the startbit.
    bool invert : 1;
  } flags;
} decup_encoder_config_t;

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
                                rmt_encoder_handle_t* ret_encoder);

#ifdef __cplusplus
}
#endif