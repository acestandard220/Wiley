#pragma once
#include <limits>
#include <concepts>

namespace Wiley {

	template<typename T>
	concept IsFloatingPoint = std::is_floating_point_v<T>;

	template<IsFloatingPoint T = float>
	constexpr T pi = T(3.141592654f);

	template<IsFloatingPoint T = float>
	constexpr T pi_div_2 = pi<T> / 2.0f;

	template<IsFloatingPoint T = float>
	constexpr T pi_div_4 = pi<T> / 4.0f;

	template<IsFloatingPoint T = float>
	constexpr T pi_times_2 = pi<T> *2.0f;

	template<IsFloatingPoint T = float>
	constexpr T pi_times_4 = pi<T> *4.0f;

	template<IsFloatingPoint T = float>
	constexpr T pi_squared = pi<T> *pi<T>;

	template<IsFloatingPoint T = float>
	constexpr T pi_div_180 = pi<T> / 180.0f;



}