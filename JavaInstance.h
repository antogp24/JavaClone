#pragma once

#include "JavaClass.h"

struct JavaInstance {
	Interpreter* interpreter;
	JavaClass* class_info;
	std::unordered_map<std::string, JavaVariable> fields;

	JavaInstance(Interpreter* p_interpreter, JavaClass* p_class_info);
	JavaObject get(Expr_Get* expr);
	JavaObject get(std::string name, uint32_t line, uint32_t column);
	void set(Expr_Set* expr, JavaObject value);
	void set(std::string name, uint32_t line, uint32_t column, JavaObject value);
};
