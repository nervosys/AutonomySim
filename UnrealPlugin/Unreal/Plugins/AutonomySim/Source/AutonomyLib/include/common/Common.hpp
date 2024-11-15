// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_common_Common_hpp
#define autonomylib_common_Common_hpp

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "VectorMath.hpp"
#include "common/utils/RandomGenerator.hpp"
#include "common/utils/Utils.hpp"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#define RpcLibPort 41451

namespace nervosys {
namespace autonomylib {

// numericals
typedef float real_T;
// this is not required for most compilers
typedef unsigned int uint;

// well known types
typedef nervosys::autonomylib::VectorMathf VectorMath;
typedef VectorMath::Vector3f Vector3r;
typedef VectorMath::Vector2f Vector2r;
typedef VectorMath::Vector1f Vector1r;
typedef VectorMath::Array3f Array3r;
typedef VectorMath::Pose Pose;
typedef VectorMath::Quaternionf Quaternionr;
typedef VectorMath::Matrix3x3f Matrix3x3r;
typedef VectorMath::AngleAxisf AngleAxisr;
typedef common_utils::RandomGeneratorF RandomGeneratorR;
typedef common_utils::RandomGeneratorGaussianF RandomGeneratorGausianR;
typedef std::string string;
typedef common_utils::Utils Utils;
typedef VectorMath::RandomVectorGaussianT RandomVectorGaussianR;
typedef VectorMath::RandomVectorT RandomVectorR;
typedef uint64_t TTimePoint;
typedef double TTimeDelta;

template <typename T> using vector = std::vector<T>;
template <typename TKey, typename TValue> using unordered_map = std::unordered_map<TKey, TValue>;
template <typename TKey> using unordered_set = std::unordered_set<TKey>;
template <typename T> using unique_ptr = std::unique_ptr<T>;
template <typename T> using shared_ptr = std::shared_ptr<T>;
template <typename T> using vector_size_type = typename std::vector<T>::size_type;

inline std::ostream &operator<<(std::ostream &os, Quaternionr const &q) {
    float p, r, y;
    VectorMath::toEulerianAngle(q, p, r, y);
    return os << "(" << r << "\t" << p << "\t" << y << ")" << q.w() << q.x() << "\t" << q.y() << "\t" << q.z() << "\t";
}

inline std::ostream &operator<<(std::ostream &os, Vector3r const &vec) {
    return os << vec.x() << "\t" << vec.y() << "\t" << vec.z() << "\t";
}

} // namespace autonomylib
} // namespace nervosys

#endif
