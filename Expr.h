#pragma once

#include "Token.h"
#include <vector>

enum class ExprType {
	assign,
	binary,
	call,
	get,
	grouping,
	literal,
	logical,
	set,
	ternary,
	unary,
	variable,
};

struct Expr {
	virtual inline ExprType get_type() = 0;
};

void expression_free(Expr *expr);

struct Expr_Assign : public Expr {
	const Expr* lhs;
	const Token lhs_name;
	const Expr* rhs;

	Expr_Assign(const Expr* _lhs, const Token _lhs_name, const Expr* _rhs):
		lhs(_lhs),
		lhs_name(_lhs_name),
		rhs(_rhs)
	{
	}

	inline ExprType get_type() override {
		return ExprType::assign;
	}
};

struct Expr_Binary : public Expr {
	const Expr* left;
	const Token _operator;
	const Expr* right;

	Expr_Binary(const Expr* _left, const Token __operator, const Expr* _right) :
		left(_left),
		_operator(__operator),
		right(_right)
	{
	}

	inline ExprType get_type() override {
		return ExprType::binary;
	}
};

struct Expr_Call : public Expr {
	const Expr* callee;
	const Token paren;
	const std::vector<Expr*> *arguments;

	Expr_Call(const Expr* _callee, const Token _paren, const std::vector<Expr*> *_arguments) :
		callee(_callee),
		paren(_paren),
		arguments(_arguments)
	{
	}

	inline ExprType get_type() override {
		return ExprType::call;
	}
};

struct Expr_Get : public Expr {
	const Expr* object;
	const Token name;

	Expr_Get(const Expr* _object, const Token _name):
		object(_object),
		name(_name)
	{
	}

	inline ExprType get_type() override {
		return ExprType::get;
	}
};

struct Expr_Grouping : public Expr {
	const Expr* expression;

	Expr_Grouping(const Expr* _expression):
		expression(_expression)
	{
	}

	inline ExprType get_type() override {
		return ExprType::grouping;
	}
};

struct Expr_Literal : public Expr{
	JavaObject literal;

	Expr_Literal(JavaObject _literal):
		literal(_literal)
	{
	}

	inline ExprType get_type() override {
		return ExprType::literal;
	}
};

struct Expr_Logical : public Expr {
	const Expr* left;
	const Token _operator;
	const Expr* right;

	Expr_Logical(const Expr* _left, const Token __operator, const Expr* _right) :
		left(_left),
		_operator(__operator),
		right(_right)
	{
	}

	inline ExprType get_type() override {
		return ExprType::logical;
	}
};

struct Expr_Set : public Expr {
	const Expr_Get* lhs;
	const Token lhs_after_dot_name;
	const Expr* rhs;

	Expr_Set(const Expr_Get* _lhs, const Token lhs_2, const Expr* _rhs) :
		lhs(_lhs),
		lhs_after_dot_name(lhs_2),
		rhs(_rhs)
	{
	}

	inline ExprType get_type() override {
		return ExprType::set;
	}
};

struct Expr_Ternary : public Expr {
	const Expr *condition;
	const Expr *then;
	const Expr *otherwise;

	Expr_Ternary(const Expr* _condition, const Expr* _then, const Expr* _otherwise):
		condition(_condition),
		then(_then),
		otherwise(_otherwise)
	{
	}

	inline ExprType get_type() override {
		return ExprType::ternary;
	}
};

struct Expr_Unary : public Expr {
	const Token _operator;
	const Expr *right;

	Expr_Unary(const Token __operator, const Expr* _right) :
		_operator(__operator),
		right(_right)
	{
	}

	inline ExprType get_type() override {
		return ExprType::unary;
	}
};

struct Expr_Variable : public Expr {
	const Token name;
	const bool is_function;

	Expr_Variable(const Token _name, const bool _is_function) :
		name(_name),
		is_function(_is_function)
	{
	}

	inline ExprType get_type() override {
		return ExprType::variable;
	}
};

