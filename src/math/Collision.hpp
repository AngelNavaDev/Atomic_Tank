#pragma once

namespace Math {

/**
 * @brief Funciones de colisión puras y constexpr sin usar std::sqrt (Distancia al cuadrado).
 * Cumple la política de C++17 de alto rendimiento para el bucle de simulación.
 */

[[nodiscard]] inline constexpr bool checkCircleCircle(
    float x1, float y1, float r1,
    float x2, float y2, float r2) noexcept
{
    const float dx = x1 - x2;
    const float dy = y1 - y2;
    const float radiusSum = r1 + r2;
    return (dx * dx + dy * dy) <= (radiusSum * radiusSum);
}

[[nodiscard]] inline constexpr bool checkCircleAABB(
    float cx, float cy, float cr,
    float rx, float ry, float rw, float rh) noexcept
{
    // Clamp manual constexpr sin dependencias externas
    const float closestX = (cx < rx) ? rx : ((cx > rx + rw) ? (rx + rw) : cx);
    const float closestY = (cy < ry) ? ry : ((cy > ry + rh) ? (ry + rh) : cy);

    const float dx = cx - closestX;
    const float dy = cy - closestY;
    return (dx * dx + dy * dy) <= (cr * cr);
}

[[nodiscard]] inline constexpr bool checkAABBAABB(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) noexcept
{
    return (x1 < x2 + w2) && (x1 + w1 > x2) &&
           (y1 < y2 + h2) && (y1 + h1 > y2);
}

[[nodiscard]] inline constexpr bool checkPointAABB(
    float px, float py,
    float rx, float ry, float rw, float rh) noexcept
{
    return (px >= rx) && (px <= rx + rw) && (py >= ry) && (py <= ry + rh);
}

} // namespace Math

