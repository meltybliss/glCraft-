#pragma once
#include <stdint.h>

struct RaycastHit {
	bool isHit = false;

	int64_t hitX = 0;
	int64_t hitY = 0;
	int64_t hitZ = 0;

	int64_t previousX = 0;
	int64_t previousY = 0;
	int64_t previousZ = 0;

};