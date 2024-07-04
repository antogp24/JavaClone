#pragma once

#include "Expr.h"
#include "Stmt.h"
#include "JavaObject.h"
#include "Environment.h"

class Interpreter {
public:
	Interpreter();
	~Interpreter();
	void interpret(std::vector<Stmt*>* statements);
	void execute_block(const std::vector<Stmt*> &statements, Environment *environment);
	static struct Return { JavaObject value; };
private:
	void execute_statement(Stmt* statement);
	JavaObject evaluate(Expr *expression);
	JavaObject evaluate_binary(Expr *expression);
	JavaObject evaluate_logical(Expr* expression);
	JavaObject evaluate_increment_or_decrement(Expr* expression);
	JavaObject evaluate_unary(Expr *expression);
	// bool is_truthy(const JavaObject &object);
	// bool is_equal(const JavaObject &a, const JavaObject &b);
	// void check_number_operand(const Token &_operator, const JavaObject &rhs);
	void check_number_operands(const JavaObject &lhs, const Token &_operator, const JavaObject &rhs);
private:
	bool broke = false;
	bool continued = false;
public:
	Environment* globals;
	Environment* environment;
};
