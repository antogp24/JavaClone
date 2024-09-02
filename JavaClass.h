#pragma once

#include <string>
#include "Token.h"
#include "JavaCallable.h"

struct JavaClass : public JavaCallable {
	const std::string name;
	uint32_t line, column;
	const bool is_abstract;
	Interpreter *interpreter;
	std::vector<Stmt_Var*> attributes;
	std::vector<Stmt_Function*> methods;
	std::unordered_map<std::string, JavaVariable> static_fields;
	Stmt_Function* constructor = nullptr;

	JavaClass(Interpreter *p_interpreter, std::string p_name, uint32_t p_line, uint32_t p_column, bool p_is_abstract, std::vector<Stmt_Var*> p_attributes, std::vector<Stmt_Function*> p_methods);

	int arity() override;
	JavaObject call(Interpreter* intepreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) override;
	std::string to_string() override;
	CallableType get_type() override;

	JavaObject get(Expr_Get* expr);
	JavaObject get(std::string name, uint32_t line, uint32_t column);
	void set(Expr_Set* expr, JavaObject value);
	void set(std::string name, uint32_t line, uint32_t column, JavaObject value);
};