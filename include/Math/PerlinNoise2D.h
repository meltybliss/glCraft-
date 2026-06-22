#pragma once
#include <cmath>
#include <stdint.h>

class PerlinNoise2D {
public:
	explicit PerlinNoise2D(uint64_t seed) : m_seed(seed) {}

	double Noise(double x, double z) const {
		const int64_t x0 = static_cast<int64_t>(std::floor(x));
		const int64_t z0 = static_cast<int64_t>(std::floor(z));

		const int64_t x1 = x0 + 1;
		const int64_t z1 = z0 + 1;

		// 格子マス内での位置。常に 0.0 ～ 1.0
		const double tx = x - static_cast<double>(x0);
		const double tz = z - static_cast<double>(z0);

		const double n00 = DotGradient(x0, z0, tx, tz);
		const double n10 = DotGradient(x1, z0, tx - 1.0, tz);
		const double n01 = DotGradient(x0, z1, tx, tz - 1.0);
		const double n11 = DotGradient(x1, z1, tx - 1.0, tz - 1.0);

		// 格子の中で、急に折れないよう滑らかに混ぜる割合
		const double u = Fade(tx);
		const double v = Fade(tz);

		const double bottom = Lerp(n00, n10, u);
		const double top = Lerp(n01, n11, u);

		return Lerp(bottom, top, v);
	}

private:
	uint64_t m_seed = 0;

	static double Fade(double t) {
		return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
	}

	static double Lerp(double a, double b, double t) {
		return a + (b - a) * t;
	}

	static uint64_t Mix(uint64_t value) {
		value += 0x9E3779B97F4A7C15ull;
		value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ull;
		value = (value ^ (value >> 27)) * 0x94D049BB133111EBull;
		return value ^ (value >> 31);
	}

	uint64_t HashGrid(int64_t gridX, int64_t gridZ) const {
		uint64_t h = m_seed;

		h ^= Mix(static_cast<uint64_t>(gridX));
		h ^= Mix(static_cast<uint64_t>(gridZ) + 0x517CC1B727220A95ull);

		return Mix(h);
	}

	double DotGradient(
		int64_t gridX,
		int64_t gridZ,
		double dx,
		double dz
	) const {
		constexpr double invSqrt2 = 0.7071067811865475;

		switch (HashGrid(gridX, gridZ) & 7ull) {
		case 0: return  dx;                           // →
		case 1: return -dx;                           // ←
		case 2: return  dz;                           // ↑
		case 3: return -dz;                           // ↓
		case 4: return (dx + dz) * invSqrt2;         // ↗
		case 5: return (-dx + dz) * invSqrt2;         // ↖
		case 6: return (dx - dz) * invSqrt2;         // ↘
		default:return (-dx - dz) * invSqrt2;         // ↙
		}


	}
};