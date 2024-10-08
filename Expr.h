#pragma once

#include "Token.h"
#include "JavaObject.h"
#include <vector>
#include <string>

enum class ExprType {
	assign,
	binary,
	call,
	cast,
	get,
	grouping,
	increment,
	literal,
	logical,
	set,
	ternary,
	self,
	unary,
	variable,
};

struct Expr { virtual inline ExprType get_type() = 0; };

void expr_free(Expr *expr);

struct Expr_Assign : public Expr {
	const Expr* lhs;
	const std::string lhs_name;
	const uint32_t line;
	const uint32_t column;
	const Expr* rhs;

	Expr_Assign(const Expr* _lhs, const std::string _lhs_name, const uint32_t _line, const uint32_t _column, const Expr* _rhs):
		lhs(_lhs),
		lhs_name(_lhs_name),
		line(_line),
		column(_column),
		rhs(_rhs)
	{}

	inline ExprType get_type() override { return ExprType::assign; }
};

struct Expr_Binary : public Expr {
	const Expr* left;
	const Token _operator;
	const Expr* right;

	Expr_Binary(const Expr* _left, const Token __operator, const Expr* _right) :
		left(_left),
		_operator(__operator),
		right(_right)
	{}

	inline ExprType get_type() override { return ExprType::binary; }
};

struct Expr_Cast : public Expr {
	const JavaType type;
	const uint32_t line, column;
	const Expr *right;

	Expr_Cast(const JavaType _type, const uint32_t _line, const uint32_t _column, const Expr* _right):
		type(_type), line(_line), column(_column), right(_right) {}

	inline ExprType get_type() override { return ExprType::cast; }
};

struct ParseCallInfo {
	Expr* expr;
	uint32_t line, column;
};

struct Expr_Call : public Expr {
	const Expr* callee;
	const Token paren;
	const std::vector<ParseCallInfo> *arguments;

	Expr_Call(const Expr* _callee, const Token _paren, const std::vector<ParseCallInfo> *_arguments) :
		callee(_callee),
		paren(_paren),
		arguments(_arguments)
	{}

	inline ExprType get_type() override { return ExprType::call; }
};

struct Expr_Get : public Expr {
	const Expr* object;
	const std::string name;
	const uint32_t line, column;

	Expr_Get(const Expr* _object, const Token _name):
		object(_object),
		name(_name.lexeme),
		line(_name.line),
		column(_name.column)
	{}

	inline ExprType get_type() override { return ExprType::get; }
};

struct Expr_Grouping : public Expr {
	const Expr* expression;

	Expr_Grouping(const Expr* _expression):
		expression(_expression)
	{}

	inline ExprType get_type() override { return ExprType::grouping; }
};

struct Expr_Increment : public Expr {
	const Token name;
	const bool is_positive;

	Expr_Increment(const Token p_name, const int8_t p_is_positive):
		name(p_name), is_positive(p_is_positive)
	{}

	inline ExprType get_type() override { return ExprType::increment; }
};

struct Expr_Literal : public Expr{
	JavaObject literal;

	Expr_Literal(JavaObject _literal):
		literal(_literal)
	{}

	inline ExprType get_type() override { return ExprType::literal; }
};

struct Expr_Logical : public Expr {
	const Expr* left;
	const Token _operator;
	const Expr* right;

	Expr_Logical(const Expr* _left, const Token __operator, const Expr* _right) :
		left(_left),
		_operator(__operator),
		right(_right)
	{}

	inline ExprType get_type() override { return ExprType::logical; }
};

struct Expr_Set : public Expr {
	const Expr_Get* lhs;
	const std::string rhs_name;
	const uint32_t line, column;
	const Expr* value;

	Expr_Set(const Expr_Get* _lhs, const std::string _name, const uint32_t _line, const uint32_t _column, const Expr* _value) :
		lhs(_lhs),
		rhs_name(_name),
		line(_line),
		column(_column),
		value(_value)
	{}

	inline ExprType get_type() override { return ExprType::set; }
};

struct Expr_Ternary : public Expr {
	const Expr *condition;
	const Expr *then;
	const Expr *otherwise;
	const Token question_mark;

	Expr_Ternary(const Expr* _condition, const Expr* _then, const Expr* _otherwise, const Token _question_mark):
		condition(_condition),
		then(_then),
		otherwise(_otherwise),
		question_mark(_question_mark)
	{}

	inline ExprType get_type() override { return ExprType::ternary; }
};

struct Expr_This : public Expr {
	std::string name;
	uint32_t line, column;

	Expr_This(std::string p_name, uint32_t p_line, uint32_t p_column):
		name(p_name), line(p_line), column(p_column) {}

	inline ExprType get_type() override { return ExprType::self; }
};

struct Expr_Unary : public Expr {
	const Token _operator;
	const Expr *right;

	Expr_Unary(const Token __operator, const Expr* _right) :
		_operator(__operator),
		right(_right)
	{}

	inline ExprType get_type() override { return ExprType::unary; }
};

struct Expr_Variable : public Expr {
	const std::string name;
	const uint32_t line;
	const uint32_t column;
	const bool is_function;

	Expr_Variable(const std::string _name, const uint32_t _line, const uint32_t _column, const bool _is_function) :
		name(_name),
		line(_line),
		column(_column),
		is_function(_is_function)
	{}

	inline ExprType get_type() override { return ExprType::variable; }
};

