#pragma once

#include <cmath>

namespace Math {

struct Vector2D {
    float x{0.0f};
    float y{0.0f};

    constexpr Vector2D() = default;
    constexpr Vector2D(float p_x, float p_y) noexcept : x{p_x}, y{p_y} {}

    [[nodiscard]] constexpr Vector2D operator+(const Vector2D& other) const noexcept {
        return {x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vector2D operator-(const Vector2D& other) const noexcept {
        return {x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vector2D operator*(float scalar) const noexcept {
        return {x * scalar, y * scalar};
    }

    [[nodiscard]] constexpr Vector2D operator/(float scalar) const noexcept {
        return {x / scalar, y / scalar};
    }

    Vector2D& operator+=(const Vector2D& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2D& operator-=(const Vector2D& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2D& operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    [[nodiscard]] constexpr float lengthSquared() const noexcept {
        return x * x + y * y;
    }

    [[nodiscard]] float length() const noexcept {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]] Vector2D normalized() const noexcept {
        const float lenSq = lengthSquared();
        if (lenSq > 0.000001f) {
            const float invLen = 1.0f / std::sqrt(lenSq);
            return {x * invLen, y * invLen};
        }
        return {0.0f, 0.0f};
    }

    [[nodiscard]] constexpr float dot(const Vector2D& other) const noexcept {
        return x * other.x + y * other.y;
    }
};

} // namespace Math

