#include "Util/RandomUtil.h"


uint64_t RandomUtil::CreateRandomSeed() {

	std::random_device rd;

	std::mt19937_64 rng(
		(static_cast<uint64_t>(rd()) << 32) |
		static_cast<uint64_t>(rd())

	);

	return rng();
}