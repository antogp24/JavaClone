#pragma once

#include "JavaObject.h"
#include "Expr.h"

class Interpreter {
public:
	void interpret(Expr *expression);
private:
	JavaObject evaluate(Expr *expression);
	JavaObject evaluate_binary(Expr *expression);
	JavaObject evaluate_logical(Expr* expression);
	JavaObject evaluate_unary(Expr *expression);
	bool is_truthy(const JavaObject &object);
	bool is_equal(const JavaObject &a, const JavaObject &b);
	void check_number_operand(const Token &_operator, const JavaObject &rhs);
	void check_number_operands(const JavaObject &lhs, const Token &_operator, const JavaObject &rhs);
};
