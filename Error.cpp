#include "Error.h"
#include "Color.h"

#include <stdio.h>
#include <stdarg.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

void JavaError::error(uint32_t line, uint32_t column, const char* fmt, ...) {
	printf(COLOR_CYN"Error at [%u:%u]: ", line, column);

	va_list args;
	__crt_va_start(args, fmt);
	(void)_vfprintf_l(stdout, fmt, NULL, args);
	__crt_va_end(args);

	printf(ERROR_MSG_END);
	had_error = true;
}

void JavaError::error(const Token &token, const char* fmt, ...) {
	char* error_point = token.lexeme != NULL ? token.lexeme : (char*)get_token_type_name(token.type);
	printf(COLOR_CYN"Error at '%s' on [%u:%u]: ", error_point, token.line, token.column);

	va_list args;
	__crt_va_start(args, fmt);
	(void)_vfprintf_l(stdout, fmt, NULL, args);
	__crt_va_end(args);

	printf(ERROR_MSG_END);
	had_error = true;
}

void JavaError::error(const Token &token, const char* fmt, va_list args) {
	char* error_point = token.lexeme != NULL ? token.lexeme : (char*)get_token_type_name(token.type);
	printf(COLOR_CYN"Error at '%s' on [%u:%u]: ", error_point, token.line, token.column);
	(void)_vfprintf_l(stdout, fmt, NULL, args);
	printf(ERROR_MSG_END);
	had_error = true;
}

void JavaError::runtime_error(const JavaRuntimeError &error) {
	printf(COLOR_CYN"Error at '%s' on [%u:%u]: %s", error.token.lexeme, error.token.line, error.token.column, error.message);
	printf(ERROR_MSG_END);
	had_runtime_error = true;
}

