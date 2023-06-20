//
// Created by wangy on 2022/11/1.
//
#include "rand.h"
bool wyt_rand(double par) {
    std::bernoulli_distribution dist(par);
    return dist(rand_generator());
}
