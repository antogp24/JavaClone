#pragma once

#include <string>
#include "Token.h"
#include "JavaCallable.h"

struct JavaClass : public JavaCallable {
	const std::string name;
	uint32_t line, column;

	JavaClass(std::string p_name, uint32_t p_line, uint32_t p_column):
		name(p_name), line(p_line), column(p_column) {}

	int arity() override;
	JavaObject call(Interpreter* intepreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) override;
	std::string to_string() override;
	CallableType get_type() override;
};