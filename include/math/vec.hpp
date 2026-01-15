// ==============================================================================
// vec.hpp
// 2D, 3D, and 4D vector primitives with basic arithmetic, normalization,
// distance, and utility functions. Templated for float types (F32, F64).
// ==============================================================================

#pragma once

#include <raylib.h>

#include <cmath>
#include <format>

#include "base/type.hpp"
#include "math/impl.hpp"

namespace nbody::math {
using namespace nbody::base::type;

// ==============================================================================
// Vector Types and Operations
// ==============================================================================
//
// The following templates define 2D, 3D, and 4D vector types for floating-point
// values (F32, F64). Each vector type provides basic arithmetic, scaling,
// dot/cross products, normalization, distance, and conversion utilities.
// All methods are constexpr or noexcept where possible for performance and
// usability in generic code.
//
// To avoid excessive repetition, only unique or non-obvious methods are
// commented individually. All vector types follow the same API pattern.
//

// 2D vector of templated float values.
template <FloatT Float>
struct Vec2T {
    Float x = 0.0;
    Float y = 0.0;

    constexpr Vec2T() noexcept = default;
    constexpr Vec2T(Float x, Float y) noexcept
        : x(static_cast<Float>(x)), y(static_cast<Float>(y)) {}

    // Factory: zero vector (0, 0)
    [[nodiscard]] static constexpr Vec2T make_zero() noexcept {
        return Vec2T{0.0, 0.0};
    }
    // Factory: one vector (1, 1)
    [[nodiscard]] static constexpr Vec2T make_one() noexcept {
        return Vec2T{1.0, 1.0};
    }
    // Factory: unit x (1, 0)
    [[nodiscard]] static constexpr Vec2T make_unit_x() noexcept {
        return Vec2T{1.0, 0.0};
    }
    // Factory: unit y (0, 1)
    [[nodiscard]] static constexpr Vec2T make_unit_y() noexcept {
        return Vec2T{0.0, 1.0};
    }

    // Add two vectors
    [[nodiscard]] constexpr Vec2T add(Vec2T other) const noexcept {
        return Vec2T{x + other.x, y + other.y};
    }
    // Subtract two vectors
    [[nodiscard]] constexpr Vec2T sub(Vec2T other) const noexcept {
        return Vec2T{x - other.x, y - other.y};
    }
    // Multiply two vectors component-wise
    [[nodiscard]] constexpr Vec2T mul(Vec2T other) const noexcept {
        return Vec2T{x * other.x, y * other.y};
    }
    // Scale vector by scalar
    [[nodiscard]] constexpr Vec2T scale(Float scalar) const noexcept {
        return Vec2T{x * scalar, y * scalar};
    }
    // Dot product
    [[nodiscard]] constexpr Float dot(Vec2T other) const noexcept {
        return x * other.x + y * other.y;
    }
    // Squared length
    [[nodiscard]] constexpr Float length_sq() const noexcept {
        return dot(*this);
    }
    // Euclidean length
    [[nodiscard]] Float length() const noexcept {
        return static_cast<Float>(std::sqrt(length_sq()));
    }
    // Normalized vector (returns zero vector if length is zero)
    // Returns zero vector when input length is zero.
    [[nodiscard]] Vec2T normalized() const noexcept {
        const Float l = length();
        if (l == 0.0) return Vec2T::make_zero();
        return scale(1 / l);
    }
    // Squared distance to another vector
    [[nodiscard]] constexpr Float distance_sq(Vec2T other) const noexcept {
        const Vec2T d = sub(other);
        return d.length_sq();
    }
    // Euclidean distance to another vector
    [[nodiscard]] Float distance(Vec2T other) const noexcept {
        return static_cast<Float>(std::sqrt(distance_sq(other)));
    }
    // Approximate equality using epsilon
    // Approximate equality using an epsilon.
    [[nodiscard]] constexpr bool is_approx_equal(
        Vec2T other,
        Float eps = impl::default_epsilon<Float>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) &&
               (std::fabs(y - other.y) <= eps);
    }
    // String representation
    [[nodiscard]] std::string to_string() const {
        return std::format("({:.16f}, {:.16f})", x, y);
    }
    // Convert to Raylib's Vector2
    [[nodiscard]] Vector2 as_raylib_vector() const noexcept {
        return Vector2{
            .x = static_cast<F32>(x),
            .y = static_cast<F32>(y),
        };
    }
};

// 3D vector of templated float values.
template <FloatT Float>
struct Vec3T {
    Float x = 0.0;
    Float y = 0.0;
    Float z = 0.0;

    constexpr Vec3T() noexcept = default;
    constexpr Vec3T(Float x, Float y, Float z) noexcept : x(x), y(y), z(z) {}

    [[nodiscard]] static constexpr Vec3T make_zero() noexcept {
        return Vec3T{0.0, 0.0, 0.0};
    }
    [[nodiscard]] static constexpr Vec3T make_one() noexcept {
        return Vec3T{1.0, 1.0, 1.0};
    }
    [[nodiscard]] static constexpr Vec3T make_unit_x() noexcept {
        return Vec3T{1.0, 0.0, 0.0};
    }
    [[nodiscard]] static constexpr Vec3T make_unit_y() noexcept {
        return Vec3T{0.0, 1.0, 0.0};
    }
    [[nodiscard]] static constexpr Vec3T make_unit_z() noexcept {
        return Vec3T{0.0, 0.0, 1.0};
    }

    [[nodiscard]] constexpr Vec3T add(Vec3T other) const noexcept {
        return Vec3T{x + other.x, y + other.y, z + other.z};
    }
    [[nodiscard]] constexpr Vec3T sub(Vec3T other) const noexcept {
        return Vec3T{x - other.x, y - other.y, z - other.z};
    }
    [[nodiscard]] constexpr Vec3T mul(Vec3T other) const noexcept {
        return Vec3T{x * other.x, y * other.y, z * other.z};
    }
    [[nodiscard]] constexpr Vec3T scale(Float scalar) const noexcept {
        return Vec3T{x * scalar, y * scalar, z * scalar};
    }
    [[nodiscard]] constexpr Float dot(Vec3T other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }
    // Cross product (returns vector perpendicular to both)
    // Cross product (returns vector perpendicular to both)
    [[nodiscard]] constexpr Vec3T cross(Vec3T other) const noexcept {
        return Vec3T{y * other.z - z * other.y, z * other.x - x * other.z,
                     x * other.y - y * other.x};
    }
    [[nodiscard]] constexpr Float length_sq() const noexcept {
        return dot(*this);
    }
    [[nodiscard]] Float length() const noexcept {
        return static_cast<Float>(std::sqrt(length_sq()));
    }
    [[nodiscard]] Vec3T normalized() const noexcept {
        const Float l = length();
        if (l == 0.0) return Vec3T::make_zero();
        return div_scalar(l);
    }
    [[nodiscard]] constexpr Float distance_sq(Vec3T other) const noexcept {
        const Vec3T d = sub(other);
        return d.length_sq();
    }
    [[nodiscard]] Float distance(Vec3T other) const noexcept {
        return static_cast<Float>(std::sqrt(distance_sq(other)));
    }
    // Approximate equality using an epsilon.
    [[nodiscard]] constexpr bool is_approx_equal(
        Vec3T other,
        Float eps = impl::default_epsilon<Float>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) &&
               (std::fabs(y - other.y) <= eps) &&
               (std::fabs(z - other.z) <= eps);
    }
    [[nodiscard]] std::string to_string() const {
        return std::format("({:.16f}, {:.16f}, {:.16f})", x, y, z);
    }
    [[nodiscard]] Vector3 as_raylib_vector() const noexcept {
        return Vector3{
            .x = static_cast<F32>(x),
            .y = static_cast<F32>(y),
            .z = static_cast<F32>(z),
        };
    }
};

// 4D vector of templated float values.
template <FloatT Float>
struct Vec4T {
    Float x = 0.0;
    Float y = 0.0;
    Float z = 0.0;
    Float w = 0.0;

    constexpr Vec4T() noexcept = default;
    constexpr Vec4T(Float x, Float y, Float z, Float w) noexcept
        : x(x), y(y), z(z), w(w) {}

    [[nodiscard]] static constexpr Vec4T make_zero() noexcept {
        return Vec4T{0.0, 0.0, 0.0, 0.0};
    }
    [[nodiscard]] static constexpr Vec4T make_one() noexcept {
        return Vec4T{1.0, 1.0, 1.0, 1.0};
    }
    [[nodiscard]] static constexpr Vec4T make_unit_x() noexcept {
        return Vec4T{1.0, 0.0, 0.0, 0.0};
    }
    [[nodiscard]] static constexpr Vec4T make_unit_y() noexcept {
        return Vec4T{0.0, 1.0, 0.0, 0.0};
    }
    [[nodiscard]] static constexpr Vec4T make_unit_z() noexcept {
        return Vec4T{0.0, 0.0, 1.0, 0.0};
    }
    [[nodiscard]] static constexpr Vec4T make_unit_w() noexcept {
        return Vec4T{0.0, 0.0, 0.0, 1.0};
    }

    [[nodiscard]] constexpr Vec4T add(Vec4T other) const noexcept {
        return Vec4T{x + other.x, y + other.y, z + other.z, w + other.w};
    }
    [[nodiscard]] constexpr Vec4T sub(Vec4T other) const noexcept {
        return Vec4T{x - other.x, y - other.y, z - other.z, w - other.w};
    }
    [[nodiscard]] constexpr Vec4T mul(Vec4T other) const noexcept {
        return Vec4T{x * other.x, y * other.y, z * other.z, w * other.w};
    }
    [[nodiscard]] constexpr Vec4T scale(Float scalar) const noexcept {
        return Vec4T{x * scalar, y * scalar, z * scalar, w * scalar};
    }
    [[nodiscard]] constexpr Float dot(Vec4T other) const noexcept {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
    [[nodiscard]] constexpr Float length_sq() const noexcept {
        return dot(*this);
    }
    [[nodiscard]] Float length() const noexcept {
        return static_cast<Float>(std::sqrt(length_sq()));
    }
    [[nodiscard]] Vec4T normalized() const noexcept {
        const Float l = length();
        if (l == 0.0) return Vec4T::make_zero();
        return div_scalar(l);
    }
    [[nodiscard]] constexpr Float distance_sq(Vec4T other) const noexcept {
        const Vec4T d = sub(other);
        return d.length_sq();
    }
    [[nodiscard]] Float distance(Vec4T other) const noexcept {
        return static_cast<Float>(std::sqrt(distance_sq(other)));
    }
    // Approximate equality using an epsilon.
    [[nodiscard]] constexpr bool is_approx_equal(
        Vec4T other,
        Float eps = impl::default_epsilon<Float>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) &&
               (std::fabs(y - other.y) <= eps) &&
               (std::fabs(z - other.z) <= eps) &&
               (std::fabs(w - other.w) <= eps);
    }
    [[nodiscard]] std::string to_string() const {
        return std::format("({:.16f}, {:.16f}, {:.16f}, {:.16f})", x, y, z, w);
    }
    [[nodiscard]] Vector4 as_raylib_vector() const noexcept {
        return Vector4{
            .x = static_cast<F32>(x),
            .y = static_cast<F32>(y),
            .z = static_cast<F32>(z),
            .w = static_cast<F32>(w),
        };
    }
};

// ==============================================================================
// Convenient Aliases
// ==============================================================================

// Aliases for common float vector types.
using Vec2F32 = Vec2T<F32>;
using Vec3F32 = Vec3T<F32>;
using Vec4F32 = Vec4T<F32>;
using Vec2F64 = Vec2T<F64>;
using Vec3F64 = Vec3T<F64>;
using Vec4F64 = Vec4T<F64>;

}  // namespace nbody::math
