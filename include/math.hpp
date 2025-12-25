#pragma once

// NOTE: Basic math types and functions.
// NOTE: Design considerations.
// - All the types are templated for the use F32 and F64.
// - All the Vec types are being passed by value for now.
//
// TODO: Create a SIMD implementation for Vec types.

#include <cmath>
#include <type_traits>

#include "base.hpp"

namespace nbody {

// NOTE: Helper to pick a reasonable default epsilon for approximate comparisons.
template <FloatingPointT T>
constexpr T default_epsilon() noexcept {
    if constexpr (std::is_same_v<T, F32>) {
        return static_cast<T>(1e-6f);
    } else {
        return static_cast<T>(1e-12);
    }
}

template <FloatingPointT T>
struct Vec2T {
    T x = static_cast<T>(0);
    T y = static_cast<T>(0);

    constexpr Vec2T() noexcept = default;
    constexpr Vec2T(T x_, T y_) noexcept : x(x_), y(y_) {}

    [[nodiscard]] static constexpr Vec2T zero() noexcept {
        return Vec2T{static_cast<T>(0), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec2T one() noexcept {
        return Vec2T{static_cast<T>(1), static_cast<T>(1)};
    }
    [[nodiscard]] static constexpr Vec2T unit_x() noexcept {
        return Vec2T{static_cast<T>(1), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec2T unit_y() noexcept {
        return Vec2T{static_cast<T>(0), static_cast<T>(1)};
    }

    [[nodiscard]] constexpr Vec2T scale(T scalar) const noexcept {
        return Vec2T{x * scalar, y * scalar};
    }
    [[nodiscard]] constexpr Vec2T add(Vec2T other) const noexcept {
        return Vec2T{x + other.x, y + other.y};
    }
    [[nodiscard]] constexpr Vec2T sub(Vec2T other) const noexcept {
        return Vec2T{x - other.x, y - other.y};
    }
    [[nodiscard]] constexpr Vec2T mul(Vec2T other) const noexcept {
        return Vec2T{x * other.x, y * other.y};
    }
    [[nodiscard]] constexpr Vec2T div_scalar(T scalar) const noexcept {
        return Vec2T{x / scalar, y / scalar};
    }

    constexpr void mut_scale(T scalar) noexcept {
        x *= scalar;
        y *= scalar;
    }
    constexpr void mut_add(Vec2T other) noexcept {
        x += other.x;
        y += other.y;
    }
    constexpr void mut_sub(Vec2T other) noexcept {
        x -= other.x;
        y -= other.y;
    }
    constexpr void mut_mul(Vec2T other) noexcept {
        x *= other.x;
        y *= other.y;
    }
    constexpr void mut_div(Vec2T other) noexcept {
        x /= other.x;
        y /= other.y;
    }

    [[nodiscard]] constexpr T dot(Vec2T other) const noexcept { return x * other.x + y * other.y; }
    [[nodiscard]] constexpr T length_sq() const noexcept { return dot(*this); }
    [[nodiscard]] T length() const noexcept { return static_cast<T>(std::sqrt(length_sq())); }

    // NOTE: Returns zero vector when input length is zero.
    [[nodiscard]] Vec2T normalized() const noexcept {
        const T len = length();
        if (len == static_cast<T>(0)) return Vec2T::zero();
        return div_scalar(len);
    }

    [[nodiscard]] constexpr T distance_sq(Vec2T other) const noexcept {
        const Vec2T d = sub(other);
        return d.length_sq();
    }

    [[nodiscard]] T distance(Vec2T other) const noexcept {
        return static_cast<T>(std::sqrt(distance_sq(other)));
    }

    // NOTE: Approximate equality using an epsilon.
    [[nodiscard]] constexpr bool approx_equal(Vec2T other,
                                              T     eps = default_epsilon<T>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) && (std::fabs(y - other.y) <= eps);
    }
};

template <FloatingPointT T>
struct Vec3T {
    T x = static_cast<T>(0);
    T y = static_cast<T>(0);
    T z = static_cast<T>(0);

    constexpr Vec3T() noexcept = default;
    constexpr Vec3T(T x_, T y_, T z_) noexcept : x(x_), y(y_), z(z_) {}

    [[nodiscard]] static constexpr Vec3T zero() noexcept {
        return Vec3T{static_cast<T>(0), static_cast<T>(0), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec3T one() noexcept {
        return Vec3T{static_cast<T>(1), static_cast<T>(1), static_cast<T>(1)};
    }
    [[nodiscard]] static constexpr Vec3T unit_x() noexcept {
        return Vec3T{static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec3T unit_y() noexcept {
        return Vec3T{static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec3T unit_z() noexcept {
        return Vec3T{static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
    }

    [[nodiscard]] constexpr Vec3T scale(T scalar) const noexcept {
        return Vec3T{x * scalar, y * scalar, z * scalar};
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
    [[nodiscard]] constexpr Vec3T div_scalar(T s) const noexcept {
        return Vec3T{x / s, y / s, z / s};
    }

    constexpr void mut_scale(T scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }
    constexpr void mut_add(Vec3T other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
    }
    constexpr void mut_sub(Vec3T other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }
    constexpr void mut_mul(Vec3T other) noexcept {
        x *= other.x;
        y *= other.y;
        z *= other.z;
    }
    constexpr void mut_div_scalar(T s) noexcept {
        x /= s;
        y /= s;
        z /= s;
    }

    [[nodiscard]] constexpr T dot(Vec3T other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr Vec3T cross(Vec3T other) const noexcept {
        return Vec3T{y * other.z - z * other.y, z * other.x - x * other.z,
                     x * other.y - y * other.x};
    }

    [[nodiscard]] constexpr T length_sq() const noexcept { return dot(*this); }

    [[nodiscard]] T length() const noexcept { return static_cast<T>(std::sqrt(length_sq())); }

    [[nodiscard]] Vec3T normalized() const noexcept {
        const T len = length();
        if (len == static_cast<T>(0)) return Vec3T::zero();
        return div_scalar(len);
    }

    [[nodiscard]] constexpr T distance_sq(Vec3T other) const noexcept {
        const Vec3T d = sub(other);
        return d.length_sq();
    }

    [[nodiscard]] T distance(Vec3T other) const noexcept {
        return static_cast<T>(std::sqrt(distance_sq(other)));
    }

    [[nodiscard]] constexpr bool approx_equal(Vec3T other,
                                              T     eps = default_epsilon<T>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) && (std::fabs(y - other.y) <= eps) &&
               (std::fabs(z - other.z) <= eps);
    }
};

template <FloatingPointT T>
struct Vec4T {
    T x = static_cast<T>(0);
    T y = static_cast<T>(0);
    T z = static_cast<T>(0);
    T w = static_cast<T>(0);

    constexpr Vec4T() noexcept = default;
    constexpr Vec4T(T x_, T y_, T z_, T w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

    [[nodiscard]] static constexpr Vec4T zero() noexcept {
        return Vec4T{static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec4T one() noexcept {
        return Vec4T{static_cast<T>(1), static_cast<T>(1), static_cast<T>(1), static_cast<T>(1)};
    }
    [[nodiscard]] static constexpr Vec4T unit_x() noexcept {
        return Vec4T{static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec4T unit_y() noexcept {
        return Vec4T{static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec4T unit_z() noexcept {
        return Vec4T{static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)};
    }
    [[nodiscard]] static constexpr Vec4T unit_w() noexcept {
        return Vec4T{static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
    }

    // non-mutating component-wise ops
    [[nodiscard]] constexpr Vec4T scale(T scalar) const noexcept {
        return Vec4T{x * scalar, y * scalar, z * scalar, w * scalar};
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
    [[nodiscard]] constexpr Vec4T div_scalar(T s) const noexcept {
        return Vec4T{x / s, y / s, z / s, w / s};
    }

    // mutating component-wise ops
    constexpr void mut_scale(T scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
    }
    constexpr void mut_add(Vec4T other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
    }
    constexpr void mut_sub(Vec4T other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
    }
    constexpr void mut_mul(Vec4T other) noexcept {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
    }
    constexpr void mut_div_scalar(T s) noexcept {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
    }

    [[nodiscard]] constexpr T dot(Vec4T other) const noexcept {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    [[nodiscard]] constexpr T length_sq() const noexcept { return dot(*this); }

    [[nodiscard]] T length() const noexcept { return static_cast<T>(std::sqrt(length_sq())); }

    [[nodiscard]] Vec4T normalized() const noexcept {
        const T len = length();
        if (len == static_cast<T>(0)) return Vec4T::zero();
        return div_scalar(len);
    }

    [[nodiscard]] constexpr T distance_sq(Vec4T other) const noexcept {
        const Vec4T d = sub(other);
        return d.length_sq();
    }

    [[nodiscard]] T distance(Vec4T other) const noexcept {
        return static_cast<T>(std::sqrt(distance_sq(other)));
    }

    [[nodiscard]] constexpr bool approx_equal(Vec4T other,
                                              T     eps = default_epsilon<T>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) && (std::fabs(y - other.y) <= eps) &&
               (std::fabs(z - other.z) <= eps) && (std::fabs(w - other.w) <= eps);
    }
};

// convenient aliases
using Vec2F32 = Vec2T<F32>;
using Vec3F32 = Vec3T<F32>;
using Vec4F32 = Vec4T<F32>;
using Vec2F64 = Vec2T<F64>;
using Vec3F64 = Vec3T<F64>;
using Vec4F64 = Vec4T<F64>;

}  // namespace nbody
