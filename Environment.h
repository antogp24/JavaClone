#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include "JavaObject.h"
#include "Call_Info.h"
#include "Token.h"
#include "Stmt.h"
#include "Visibility.h"

struct JavaVariable {
	JavaObject value;
	Visibility visibility;
	bool is_static;
	bool is_final;
	bool is_uninitialized;
};

typedef std::unordered_map<std::string, JavaVariable> JavaScope;

struct Environment {
	Environment() {}
	Environment(Environment* p_enclosing):
		enclosing(p_enclosing) {}

	bool scope_has(const Token &name);
	JavaVariable scope_get(const Token &name);
	void scope_set(const Token &name, JavaVariable value);

	void define(Stmt_Var* stmt, const Token& name, Expr* initializer, JavaObject value);
	void define(Token name, JavaVariable variable);
	void assign(Token name, JavaObject value);
	void assign(const std::string &name, uint32_t line, uint32_t column, JavaObject value);
	JavaObject get(const std::string &name, uint32_t line, uint32_t column);
	JavaObject get(const Token &name);
	void define_native_function(
		const std::string& name,
		std::function<int()> arity_fn,
		std::function<JavaObject(void*, std::vector<ArgumentInfo>)> call_fn,
		std::function<std::string()> to_string_fn);
	void* get_function_ptr(const std::string& name);

	JavaScope values = JavaScope();
	Environment* enclosing = nullptr;
};
