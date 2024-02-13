#include "suspend.hpp"
#include "../suspend.hpp"

namespace out::track::mdu {

///
esp_err_t suspend() { return track::suspend(); }

}  // namespace out::track::mdu