

#ifndef DFBGSP_WYZ_RAND_H
#define DFBGSP_WYZ_RAND_H

//
// Created by wangy on 2022/11/1.
//

#include <random>
#include <cfloat>


// A function to return a seeded random number generator.
inline std::mt19937 &rand_generator() {
    // the generator will only be seeded once (per thread) since it's static
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}

// A function to generate integers in the range [min, max]
template<typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
T wyt_rand(T min, T max) {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(rand_generator());
}

// A function to generate integers in the range [0, max-1]
template<typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
T wyt_rand(T max) {
    std::uniform_int_distribution<T> dist(0, max - 1);
    return dist(rand_generator());
}

// A function to generate floats in the range [min, max)
template<typename T, std::enable_if_t<std::is_floating_point_v<T>> * = nullptr>
T wyt_rand(T min, T max) {
    std::uniform_real_distribution<T> dist(min, max);
    return dist(rand_generator());
}

// A function to generate floats in the range [min, max]
template<typename T, std::enable_if_t<std::is_floating_point_v<T>> * = nullptr>
T wyt_rand_include_right(T min, T max) {
    std::uniform_real_distribution<T> dist(min, std::nextafter(max, DBL_MAX));
    return dist(rand_generator());
}

// A function to generate bools
bool wyt_rand(double par = 0.5);

#endif //DFBGSP_WYZ_RAND_H
