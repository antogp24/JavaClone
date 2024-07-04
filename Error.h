#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>

#include "Token.h"
#include "Color.h"

class JavaRuntimeError {
private:
	unsigned int call_line;
	const char* call_file;
	const char* fmt;
public:
	const std::string name;
	const uint32_t line;
	const uint32_t column;
	char message[1024] = {0};

	JavaRuntimeError(const std::string &_name,
					 const uint32_t _line,
					 const uint32_t _column,
					 unsigned int _call_line,
					 const char *_call_file,
					 const char *_fmt,
					 ...):
		name(_name),
		line(_line),
		column(_column),
		call_line(_call_line),
		call_file(_call_file),
		fmt(_fmt)
	{
		va_list args;
		va_start(args, _fmt);
        (void)_vsnprintf_l(message, sizeof(message), fmt, NULL, args);
		va_end(args);
	}

	JavaRuntimeError(const Token &token, unsigned int _call_line, const char *_call_file, const char *_fmt, ...):
		name(token.lexeme),
		line(token.line),
		column(token.column),
		call_line(_call_line),
		call_file(_call_file),
		fmt(_fmt)
	{
		va_list args;
		va_start(args, _fmt);
        (void)_vsnprintf_l(message, sizeof(message), fmt, NULL, args);
		va_end(args);
	}
};

#define JAVA_RUNTIME_ERROR(token, message) JavaRuntimeError(token, __LINE__, __FILE__, message)
#define JAVA_RUNTIME_ERROR_VA(token, fmt, ...) JavaRuntimeError(token, __LINE__, __FILE__, fmt, __VA_ARGS__)

#define JAVA_RUNTIME_ERR(name, line, column, message) JavaRuntimeError(name, line, column, __LINE__, __FILE__, message)
#define JAVA_RUNTIME_ERR_VA(name, line, column, fmt, ...) JavaRuntimeError(name, line, column, __LINE__, __FILE__, fmt, __VA_ARGS__)

namespace JavaError {
	extern bool had_error;
	extern bool had_runtime_error;

	void error(uint32_t line, uint32_t column, const char* fmt, ...);
	void error(const std::string& name, uint32_t line, uint32_t column, const char* fmt, ...);
	void error(const Token& token, const char* fmt, ...);
	void error(const Token& token, const char* fmt, va_list args);
	void runtime_error(const JavaRuntimeError &error);
};
