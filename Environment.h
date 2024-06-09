#pragma once

#include <string_view>
#include <unordered_map>

#include "JavaObject.h"
#include "Token.h"
#include "Stmt.h"
#include "Visibility.h"

struct JavaVariable {
	JavaObject value;
	Visibility visibility;
	bool is_static;
	bool is_final;
};

// TODO: Refactor this to have the value be JavaVariable.
typedef std::unordered_map<std::string, JavaVariable> JavaScope;

class Environment {
public:
	Environment() {}
	Environment(Environment* p_enclosing):
		enclosing(p_enclosing) {}

	bool scope_has(const Token &name);
	JavaVariable scope_get(const Token &name);
	void scope_set(const Token &name, JavaVariable value);

	void define(Stmt_Var *stmt, JavaObject value);
	void define(Token name, JavaVariable value);
	void assign(Token name, JavaObject value);
	JavaObject get(const Token &name);
private:
	JavaScope values = JavaScope();
public:
	Environment* enclosing = nullptr;
};
