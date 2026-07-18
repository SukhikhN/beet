#pragma once

#include <cstdint>

using std::uint8_t, std::int32_t, std::uint32_t;

namespace esp_utils {

// Returns the maximum unsigned value representable by bit width.
constexpr uint32_t max_for_bits(uint8_t bits) {
  return (static_cast<uint32_t>(1U) << bits) - 1U;
}

// Linearly remaps a value from one range to another.
// Returns 0 when input range is invalid (in_min == in_max).
constexpr int32_t map_range(int32_t x, int32_t in_min, int32_t in_max,
                            int32_t out_min, int32_t out_max) {
  const int32_t run = in_max - in_min;
  if (run == 0) {
    return 0;
  }

  const int32_t rise = out_max - out_min;
  const int32_t delta = x - in_min;

  // Use 64-bit intermediate math to avoid overflow during multiplication.
  return static_cast<int32_t>((static_cast<int64_t>(delta) * rise) / run +
                              out_min);
}

}  // namespace esp_utils