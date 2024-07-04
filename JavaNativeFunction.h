#pragma once

#include "JavaCallable.h"
#include <functional>

struct JavaNativeFunction : public JavaCallable {
	std::function<int()> arity_fn;
	std::function<JavaObject(void*, std::vector<ArgumentInfo>)> call_fn;
	std::function<std::string()> to_string_fn;

	JavaNativeFunction(std::function<int()> p_arity_fn,
					   std::function<JavaObject(void*, std::vector<ArgumentInfo>)> p_call_fn,
					   std::function<std::string()> p_to_string_fn):
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

	JavaObject call(Interpreter* interpreter, std::vector<ArgumentInfo> arguments) override {
		return call_fn(interpreter, arguments);
	}

	std::string to_string() override {
		return to_string_fn();
	}
};

