#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "Token.h"
#include "Color.h"

#define ERROR_MSG_END COLOR_END"\n\n"

class JavaRuntimeError {
private:
	unsigned int line;
	const char* file;
	const char* fmt;
public:
	const Token &token;
	char message[1024] = {0};

	JavaRuntimeError(const Token &_token, unsigned int _line, const char *_file, const char *_fmt, ...):
		token(_token),
		line(_line),
		file(_file),
		fmt(_fmt)
	{
		va_list args;
		va_start(args, _fmt);
        (void)_vsnprintf_l(message, sizeof(message), fmt, NULL, args);
		va_end(args);
	}
};

#define JAVA_RUNTIME_ERROR(token, fmt, ...) JavaRuntimeError(token, __LINE__, __FILE__, fmt, __VA_ARGS__)

namespace JavaError {
	extern bool had_error;
	extern bool had_runtime_error;

	void error(uint32_t line, uint32_t column, const char* fmt, ...);
	void error(const Token &token, const char* fmt, ...);
	void error(const Token& token, const char* fmt, va_list args);
	void runtime_error(const JavaRuntimeError &error);
};
