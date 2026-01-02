// This file contains:
// - Vector 2D (F32, F64)
// - Vector 3D (F32, F64)
// - Vector 4D (F32, F64)
// Design considerations:
// - All the types are templated for the use F32 and F64; that's why they are implemented in the
//   header file directly.
// - All the Vec types are being passed by value; at least for now.

#pragma once

#include <raymath.h>

#include <cmath>
#include <format>

#include "base/type.hpp"
#include "math/impl.hpp"

namespace math {
template <FloatT F>
struct Vec2T {
    F x = 0.0;
    F y = 0.0;

    constexpr Vec2T() noexcept = default;
    constexpr Vec2T(F x_, F y_) noexcept : x(x_), y(y_) {}

    [[nodiscard]] static constexpr Vec2T zero() noexcept { return Vec2T{0.0, 0.0}; }
    [[nodiscard]] static constexpr Vec2T one() noexcept { return Vec2T{1.0, 1.0}; }
    [[nodiscard]] static constexpr Vec2T unit_x() noexcept { return Vec2T{1.0, 0.0}; }
    [[nodiscard]] static constexpr Vec2T unit_y() noexcept { return Vec2T{0.0, 1.0}; }

    [[nodiscard]] constexpr Vec2T add(Vec2T other) const noexcept {
        return Vec2T{x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vec2T sub(Vec2T other) const noexcept {
        return Vec2T{x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vec2T mul(Vec2T other) const noexcept {
        return Vec2T{x * other.x, y * other.y};
    }

    [[nodiscard]] constexpr Vec2T scale(F scalar) const noexcept {
        return Vec2T{x * scalar, y * scalar};
    }

    [[nodiscard]] constexpr F dot(Vec2T other) const noexcept { return x * other.x + y * other.y; }
    [[nodiscard]] constexpr F length_sq() const noexcept { return dot(*this); }
    [[nodiscard]] F length() const noexcept { return static_cast<F>(std::sqrt(length_sq())); }

    // NOTE: Returns zero vector when input length is zero.
    [[nodiscard]] Vec2T normalized() const noexcept {
        const F len = length();
        if (len == static_cast<F>(0)) return Vec2T::zero();
        return div_scalar(len);
    }

    [[nodiscard]] constexpr F distance_sq(Vec2T other) const noexcept {
        const Vec2T d = sub(other);
        return d.length_sq();
    }

    [[nodiscard]] F distance(Vec2T other) const noexcept {
        return static_cast<F>(std::sqrt(distance_sq(other)));
    }

    // NOTE: Approximate equality using an epsilon.
    [[nodiscard]] constexpr bool approx_equal(Vec2T other,
                                              F eps = impl::default_epsilon<F>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) && (std::fabs(y - other.y) <= eps);
    }

    [[nodiscard]] std::string fmt() const { return std::format("({:.16f}, {:.16f})", x, y); }

    [[nodiscard]] Vector2 raylib_vector2() const noexcept { return Vector2(x, y); }
};

template <FloatT F>
struct Vec3T {
    F x = 0.0;
    F y = 0.0;
    F z = 0.0;

    constexpr Vec3T() noexcept = default;
    constexpr Vec3T(F x_, F y_, F z_) noexcept : x(x_), y(y_), z(z_) {}

    [[nodiscard]] static constexpr Vec3T zero() noexcept { return Vec3T{0.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vec3T one() noexcept { return Vec3T{1.0, 1.0, 1.0}; }
    [[nodiscard]] static constexpr Vec3T unit_x() noexcept { return Vec3T{1.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vec3T unit_y() noexcept { return Vec3T{0.0, 1.0, 0.0}; }
    [[nodiscard]] static constexpr Vec3T unit_z() noexcept { return Vec3T{0.0, 0.0, 1.0}; }

    [[nodiscard]] constexpr Vec3T add(Vec3T other) const noexcept {
        return Vec3T{x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr Vec3T sub(Vec3T other) const noexcept {
        return Vec3T{x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Vec3T mul(Vec3T other) const noexcept {
        return Vec3T{x * other.x, y * other.y, z * other.z};
    }

    [[nodiscard]] constexpr Vec3T scale(F scalar) const noexcept {
        return Vec3T{x * scalar, y * scalar, z * scalar};
    }

    [[nodiscard]] constexpr F dot(Vec3T other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr Vec3T cross(Vec3T other) const noexcept {
        return Vec3T{y * other.z - z * other.y, z * other.x - x * other.z,
                     x * other.y - y * other.x};
    }

    [[nodiscard]] constexpr F length_sq() const noexcept { return dot(*this); }

    [[nodiscard]] F length() const noexcept { return static_cast<F>(std::sqrt(length_sq())); }

    [[nodiscard]] Vec3T normalized() const noexcept {
        const F len = length();
        if (len == static_cast<F>(0)) return Vec3T::zero();
        return div_scalar(len);
    }

    [[nodiscard]] constexpr F distance_sq(Vec3T other) const noexcept {
        const Vec3T d = sub(other);
        return d.length_sq();
    }

    [[nodiscard]] F distance(Vec3T other) const noexcept {
        return static_cast<F>(std::sqrt(distance_sq(other)));
    }

    [[nodiscard]] constexpr bool approx_equal(Vec3T other,
                                              F eps = impl::default_epsilon<F>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) && (std::fabs(y - other.y) <= eps) &&
               (std::fabs(z - other.z) <= eps);
    }

    [[nodiscard]] std::string fmt() const {
        return std::format("({:.16f}, {:.16f}, {:.16f})", x, y, z);
    }

    [[nodiscard]] Vector3 raylib_vector3() const noexcept { return Vector3(x, y, z); }
};

template <FloatT F>
struct Vec4T {
    F x = 0.0;
    F y = 0.0;
    F z = 0.0;
    F w = 0.0;

    constexpr Vec4T() noexcept = default;
    constexpr Vec4T(F x_, F y_, F z_, F w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

    [[nodiscard]] static constexpr Vec4T zero() noexcept { return Vec4T{0.0, 0.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vec4T one() noexcept { return Vec4T{1.0, 1.0, 1.0, 1.0}; }
    [[nodiscard]] static constexpr Vec4T unit_x() noexcept { return Vec4T{1.0, 0.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vec4T unit_y() noexcept { return Vec4T{0.0, 1.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vec4T unit_z() noexcept { return Vec4T{0.0, 0.0, 1.0, 0.0}; }
    [[nodiscard]] static constexpr Vec4T unit_w() noexcept { return Vec4T{0.0, 0.0, 0.0, 1.0}; }

    [[nodiscard]] constexpr Vec4T add(Vec4T other) const noexcept {
        return Vec4T{x + other.x, y + other.y, z + other.z, w + other.w};
    }
    [[nodiscard]] constexpr Vec4T sub(Vec4T other) const noexcept {
        return Vec4T{x - other.x, y - other.y, z - other.z, w - other.w};
    }
    [[nodiscard]] constexpr Vec4T mul(Vec4T other) const noexcept {
        return Vec4T{x * other.x, y * other.y, z * other.z, w * other.w};
    }
    [[nodiscard]] constexpr Vec4T scale(F scalar) const noexcept {
        return Vec4T{x * scalar, y * scalar, z * scalar, w * scalar};
    }

    [[nodiscard]] constexpr F dot(Vec4T other) const noexcept {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    [[nodiscard]] constexpr F length_sq() const noexcept { return dot(*this); }

    [[nodiscard]] F length() const noexcept { return static_cast<F>(std::sqrt(length_sq())); }

    [[nodiscard]] Vec4T normalized() const noexcept {
        const F len = length();
        if (len == static_cast<F>(0)) return Vec4T::zero();
        return div_scalar(len);
    }

    [[nodiscard]] constexpr F distance_sq(Vec4T other) const noexcept {
        const Vec4T d = sub(other);
        return d.length_sq();
    }

    [[nodiscard]] F distance(Vec4T other) const noexcept {
        return static_cast<F>(std::sqrt(distance_sq(other)));
    }

    [[nodiscard]] constexpr bool approx_equal(Vec4T other,
                                              F eps = impl::default_epsilon<F>()) const noexcept {
        return (std::fabs(x - other.x) <= eps) && (std::fabs(y - other.y) <= eps) &&
               (std::fabs(z - other.z) <= eps) && (std::fabs(w - other.w) <= eps);
    }

    [[nodiscard]] std::string fmt() const {
        return std::format("({:.16f}, {:.16f}, {:.16f}, {:.16f})", x, y, z, w);
    }

    [[nodiscard]] Vector4 raylib_vector4() const noexcept { return Vector4(x, y, z, w); }
};

// NOTE: convenient aliases
using Vec2F32 = Vec2T<F32>;
using Vec3F32 = Vec3T<F32>;
using Vec4F32 = Vec4T<F32>;
using Vec2F64 = Vec2T<F64>;
using Vec3F64 = Vec3T<F64>;
using Vec4F64 = Vec4T<F64>;

}  // namespace math
