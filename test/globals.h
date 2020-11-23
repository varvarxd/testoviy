/*
 * globals.h
 *
 *  Created on: 3 февр. 2020 г.
 *      Author: prepod
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <algorithm>
#include <memory>
#include <SDL2/SDL.h>
#include <cmath>

constexpr int AUDIO_SAMPLING_FREQUENCY = 44100;
static constexpr double Pi = acos(-1.);

template <typename T>
static inline T clamp(T a, T b, T v)
{
	return std::max(a, std::min(v, b));
}

#endif /* GLOBALS_H_ */
