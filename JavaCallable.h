#pragma once

#include "Interpreter.h"

enum class CallableType {
	Builtin,
	Constructor,
	UserDefined,
};

struct JavaCallable {
	virtual CallableType get_type() = 0;
	virtual int arity() = 0;
	virtual JavaObject call(Interpreter* intepreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) = 0;
	virtual std::string to_string() = 0;
};
