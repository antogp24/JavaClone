#pragma once

#include "Expr.h"
#include "Stmt.h"
#include "Arena.h"
#include "JavaObject.h"
#include "Environment.h"

class Interpreter {
public:
	Interpreter();
	~Interpreter();
	void interpret(std::vector<Stmt*>* statements);
	void execute_block(const std::vector<Stmt*> &statements, Environment *environment);
	JavaObject validate_variable(const Stmt_Var* stmt, const JavaType type, const Token& name, const Expr* initializer);
	struct Return { JavaObject value; };
private:
	void execute_statement(Stmt* statement);
	JavaObject evaluate(Expr *expression);
	JavaObject evaluate_binary(Expr *expression);
	JavaObject evaluate_logical(Expr* expression);
	JavaObject evaluate_increment_or_decrement(Expr* expression);
	JavaObject evaluate_unary(Expr *expression);
private:
	bool broke = false;
	bool continued = false;
public:
	Environment* globals;
	Environment* environment;
	std::vector<void*> instances;
	Arena strings_arena;
};
