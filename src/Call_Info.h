#pragma once

#include "JavaObject.h"

struct ArgumentInfo {
	JavaObject object;
	uint32_t line, column;
};

