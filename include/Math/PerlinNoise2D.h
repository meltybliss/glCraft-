#pragma once
#include <cmath>
#include <stdint.h>
#include <algorithm>

struct NoiseAxis {
	int64_t grid;
	double t;
};

static int64_t FloorDiv(int64_t value, int64_t divisor) {
	int64_t q = value / divisor;
	int64_t r = value % divisor;

	if (r < 0) {
		--q;
	}

	return q;
}

static int64_t FloorMod(int64_t value, int64_t divisor) {
	int64_t r = value % divisor;

	if (r < 0) {
		r += divisor;
	}

	return r;
}

static NoiseAxis SplitWorldCoordinate(
	int64_t world,
	double offset,
	int64_t scale
) {
	const int64_t baseGrid =
		FloorDiv(world, scale);

	const int64_t remainder =
		FloorMod(world, scale);

	// ここは小さい値。
	// 例: 45 + 23.7
	const double local =
		static_cast<double>(remainder) + offset;

	// warpによって隣のnoise cellまで行く可能性がある
	const double shiftD =
		std::floor(
			local / static_cast<double>(scale)
		);

	const int64_t shift =
		static_cast<int64_t>(shiftD);

	const double inCell =
		local -
		shiftD * static_cast<double>(scale);

	return {
		baseGrid + shift,
		inCell / static_cast<double>(scale)
	};
}



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


	double NoiseWorld(
		int64_t worldX,
		int64_t worldZ,
		double offsetX,
		double offsetZ,
		int64_t scale
	) const {
		const NoiseAxis ax =
			SplitWorldCoordinate(
				worldX,
				offsetX,
				scale
			);

		const NoiseAxis az =
			SplitWorldCoordinate(
				worldZ,
				offsetZ,
				scale
			);

		const int64_t x0 = ax.grid;
		const int64_t z0 = az.grid;

		const int64_t x1 = x0 + 1;
		const int64_t z1 = z0 + 1;

		const double tx = ax.t;
		const double tz = az.t;

		const double n00 =
			DotGradient(x0, z0, tx, tz);

		const double n10 =
			DotGradient(x1, z0, tx - 1.0, tz);

		const double n01 =
			DotGradient(x0, z1, tx, tz - 1.0);

		const double n11 =
			DotGradient(x1, z1, tx - 1.0, tz - 1.0);

		const double u = Fade(tx);
		const double v = Fade(tz);

		const double bottom =
			Lerp(n00, n10, u);

		const double top =
			Lerp(n01, n11, u);

		return Lerp(bottom, top, v);
	}

private:
	uint64_t m_seed = 0;

	static double Fade(double t) {
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
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


class RealisticTerrain2D {
public:
	struct Settings {
		double seaLevel = 64.0;

		double minHeight = 4.0;
		double maxHeight = 248.0;

		// 山の最大規模。
		// World heightを512などにするなら200～400くらいまで上げられる。
		double mountainHeight = 145.0;

		// 山脈を曲げる強さ
		double warpStrength = 220.0;
	};

	explicit RealisticTerrain2D(uint64_t seed)
		: RealisticTerrain2D(seed, Settings{}) {}

	RealisticTerrain2D(uint64_t seed, Settings settings)
		:
		m_settings(settings),

		m_continent(seed + 0x1000),
		m_base(seed + 0x2000),
		m_hills(seed + 0x3000),

		m_mountainRegion(seed + 0x4000),
		m_ridges(seed + 0x5000),
		m_valleys(seed + 0x6000),

		m_warpX(seed + 0x7000),
		m_warpZ(seed + 0x8000),

		m_detail(seed + 0x9000)
	{
	}

	double GetHeight(int64_t worldX, int64_t worldZ) const {

		// ----------------------------------------
		// 1. Domain Warp
		// ----------------------------------------

		const double warpX =
			FBmWorld(
				m_warpX,
				worldX,
				worldZ,
				0.0,       // offsetX
				0.0,       // offsetZ
				900,       // scale
				3,
				0.5
			);

		const double warpZ =
			FBmWorld(
				m_warpZ,
				worldX,
				worldZ,
				173.5,
				-91.7,
				900,
				3,
				0.5
			);

		const double warpOffsetX =
			warpX * m_settings.warpStrength;

		const double warpOffsetZ =
			warpZ * m_settings.warpStrength;


		// ----------------------------------------
		// 2. Continentalness
		// ----------------------------------------

		const double continent =
			FBmWorld(
				m_continent,
				worldX,
				worldZ,
				warpOffsetX,
				warpOffsetZ,
				3200,
				5,
				0.5
			);

		const double continent01 =
			continent * 0.5 + 0.5;

		const double landMask =
			Smoothstep(
				0.43,
				0.60,
				continent01
			);


		// ----------------------------------------
		// 3. 平野 / 高原
		// ----------------------------------------

		const double macro =
			FBmWorld(
				m_base,
				worldX,
				worldZ,
				warpOffsetX,
				warpOffsetZ,
				1100,
				4,
				0.50
			);

		double height =
			m_settings.seaLevel
			+ continent * 72.0
			+ macro * 24.0;


		// ----------------------------------------
		// 4. 丘
		// ----------------------------------------

		const double hills =
			FBmWorld(
				m_hills,
				worldX,
				worldZ,
				warpOffsetX,
				warpOffsetZ,
				420,
				4,
				0.48
			);

		height +=
			hills *
			18.0 *
			landMask;


		// ----------------------------------------
		// 5. 山岳地域
		// ----------------------------------------

		const double mountainRegionNoise =
			FBmWorld(
				m_mountainRegion,
				worldX,
				worldZ,
				warpOffsetX,
				warpOffsetZ,
				1900,
				3,
				0.55
			);

		const double mountainRegion01 =
			mountainRegionNoise * 0.5 + 0.5;

		double mountainMask =
			Smoothstep(
				0.48,
				0.72,
				mountainRegion01
			);

		mountainMask *= landMask;


		// ----------------------------------------
		// 6. 山の尾根
		// ----------------------------------------

		const double ridges =
			RidgedFBmWorld(
				m_ridges,
				worldX,
				worldZ,
				warpOffsetX,
				warpOffsetZ,
				512,
				6,
				0.52
			);

		const double ridgeShape =
			std::pow(
				std::clamp(ridges, 0.0, 1.0),
				1.55
			);

		height +=
			mountainMask *
			(
				24.0 +
				ridgeShape *
				m_settings.mountainHeight
				);


		// ----------------------------------------
		// 7. 谷
		// ----------------------------------------

		const double valleyNoise =
			std::abs(
				FBmWorld(
					m_valleys,
					worldX,
					worldZ,
					warpOffsetX,
					warpOffsetZ,
					720,
					4,
					0.52
				)
			);

		const double valley =
			1.0 -
			Smoothstep(
				0.025,
				0.19,
				valleyNoise
			);

		height -=
			valley *
			mountainMask *
			72.0;


		// ----------------------------------------
		// 8. 細かい凹凸
		// ----------------------------------------

		const double detail =
			FBmWorld(
				m_detail,
				worldX,
				worldZ,
				0.0,
				0.0,
				75,
				3,
				0.43
			);

		height +=
			detail *
			5.5 *
			landMask;


		return std::clamp(
			height,
			m_settings.minHeight,
			m_settings.maxHeight
		);
	}


private:

	Settings m_settings;

	PerlinNoise2D m_continent;
	PerlinNoise2D m_base;
	PerlinNoise2D m_hills;

	PerlinNoise2D m_mountainRegion;
	PerlinNoise2D m_ridges;
	PerlinNoise2D m_valleys;

	PerlinNoise2D m_warpX;
	PerlinNoise2D m_warpZ;

	PerlinNoise2D m_detail;


	static double Smoothstep(
		double edge0,
		double edge1,
		double x
	) {
		x =
			std::clamp(
				(x - edge0) /
				(edge1 - edge0),
				0.0,
				1.0
			);

		return
			x * x *
			(3.0 - 2.0 * x);
	}


	static double FBm(
		const PerlinNoise2D& noise,
		double x,
		double z,
		double scale,
		int octaves,
		double lacunarity,
		double persistence
	) {
		double frequency = 1.0 / scale;
		double amplitude = 1.0;

		double result = 0.0;
		double amplitudeSum = 0.0;

		for (int i = 0; i < octaves; ++i) {

			result +=
				noise.Noise(
					x * frequency,
					z * frequency
				) *
				amplitude;

			amplitudeSum += amplitude;

			frequency *= lacunarity;
			amplitude *= persistence;
		}

		if (amplitudeSum == 0.0) {
			return 0.0;
		}

		return result / amplitudeSum;
	}

	double FBmWorld(
		const PerlinNoise2D& noise,
		int64_t worldX,
		int64_t worldZ,
		double offsetX,
		double offsetZ,
		int64_t baseScale,
		int octaves,
		double persistence
	) const {
		double result = 0.0;
		double amplitude = 1.0;
		double amplitudeSum = 0.0;

		int64_t scale = baseScale;

		for (int i = 0; i < octaves; ++i) {

			result +=
				noise.NoiseWorld(
					worldX,
					worldZ,
					offsetX,
					offsetZ,
					scale
				) *
				amplitude;

			amplitudeSum += amplitude;

			amplitude *= persistence;

			// frequency *= 2 と同じ意味
			scale /= 2;

			if (scale < 1) {
				scale = 1;
			}
		}

		return result / amplitudeSum;
	}


	static double RidgedFBm(
		const PerlinNoise2D& noise,
		double x,
		double z,
		double scale,
		int octaves,
		double lacunarity,
		double persistence
	) {
		double frequency = 1.0 / scale;
		double amplitude = 1.0;

		double result = 0.0;
		double amplitudeSum = 0.0;

		double previous = 1.0;

		for (int i = 0; i < octaves; ++i) {

			double n =
				noise.Noise(
					x * frequency,
					z * frequency
				);

			/*
			 * -1..1
			 *
			 * ↓ abs
			 *
			 * 0..1
			 *
			 * ↓ invert
			 *
			 * noise=0 の場所が尾根になる
			 */

			double ridge =
				1.0 - std::abs(n);

			ridge *= ridge;

			/*
			 * 前octaveの大きなridge付近だけ
			 * 細かいridgeを強くする。
			 *
			 * 山の表面全体が均等に
			 * ギザギザするのを防ぐ。
			 */

			ridge *= previous;

			previous =
				std::clamp(
					ridge * 2.0,
					0.0,
					1.0
				);

			result +=
				ridge *
				amplitude;

			amplitudeSum += amplitude;

			frequency *= lacunarity;
			amplitude *= persistence;
		}

		if (amplitudeSum == 0.0) {
			return 0.0;
		}

		return result / amplitudeSum;
	}

	double RidgedFBmWorld(
		const PerlinNoise2D& noise,
		int64_t worldX,
		int64_t worldZ,
		double offsetX,
		double offsetZ,
		int64_t baseScale,
		int octaves,
		double persistence
	) const {
		double result = 0.0;
		double amplitude = 1.0;
		double amplitudeSum = 0.0;

		double previous = 1.0;

		int64_t scale = baseScale;

		for (int i = 0; i < octaves; ++i) {

			const double n =
				noise.NoiseWorld(
					worldX,
					worldZ,
					offsetX,
					offsetZ,
					scale
				);

			// 普通のPerlinを尾根型にする
			double ridge =
				1.0 - std::abs(n);

			ridge *= ridge;

			// 大きな尾根の周辺に細かい尾根を乗せる
			ridge *= previous;

			previous =
				std::clamp(
					ridge * 2.0,
					0.0,
					1.0
				);

			result +=
				ridge * amplitude;

			amplitudeSum += amplitude;

			// 次のoctaveでは2倍細かくする
			scale /= 2;

			if (scale < 1) {
				scale = 1;
			}

			amplitude *= persistence;
		}

		return result / amplitudeSum;
	}
};

