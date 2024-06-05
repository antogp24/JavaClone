#pragma once

#include <stdint.h>
#include <stdarg.h>

#include "Token.h"

class JavaRuntimeError {
public:
	const Token &token;
	const char* fmt;
	va_list args;

	JavaRuntimeError(const Token &_token, const char *_fmt, ...):
		token(_token),
		fmt(_fmt),
		args(va_list{})
	{
		va_start(args, _fmt);
		va_end(args);
	}
};

namespace JavaError {
	extern bool had_error;
	extern bool had_runtime_error;

	void error(uint32_t line, uint32_t column, const char* fmt, ...);
	void error(const Token &token, const char* fmt, ...);
	void runtime_error(const JavaRuntimeError &error);
};
