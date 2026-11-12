#pragma once

#include <algorithm>

namespace Math {

inline constexpr float PI{3.14159265358979323846f};
inline constexpr float RAD_TO_DEG{180.0f / PI};
inline constexpr float DEG_TO_RAD{PI / 180.0f};

template <typename T>
[[nodiscard]] constexpr T clamp(T val, T minVal, T maxVal) noexcept {
    return (val < minVal) ? minVal : ((val > maxVal) ? maxVal : val);
}

[[nodiscard]] constexpr float lerp(float a, float b, float t) noexcept {
    return a + t * (b - a);
}

} // namespace Math

