#pragma once

namespace Entities {

struct Projectile {
    float x{0.0f};
    float y{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    int damage{5};
    bool isCritical{false};
    bool isHoming{false};
    bool isPiercing{false};
    bool active{false};
};

} // namespace Entities
