#pragma once

#include "JavaCallable.h"
#include <functional>

typedef std::function<int()> Native_Arity;
typedef std::function<JavaObject(void*, uint32_t, uint32_t, std::vector<ArgumentInfo>)> Native_Call;
typedef std::function<std::string()> Native_ToString;

struct JavaNativeFunction : public JavaCallable {
	Native_Arity arity_fn;
	Native_Call call_fn;
	Native_ToString to_string_fn;

	JavaNativeFunction(Native_Arity p_arity_fn,
					   Native_Call p_call_fn,
					   Native_ToString p_to_string_fn):
		arity_fn(p_arity_fn),
		call_fn(p_call_fn),
		to_string_fn(p_to_string_fn)
	{}

	CallableType get_type() {
		return CallableType::Builtin;
	}

	int arity() override {
		return arity_fn();
	}

	JavaObject call(Interpreter* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) override {
		return call_fn(interpreter, line, column, arguments);
	}

	std::string to_string() override {
		return to_string_fn();
	}
};

