#pragma once

#include "Expr.h"
#include "Visibility.h"

enum class StmtType {
	expression,
	print,
	var,
	block,
};


struct Stmt { inline virtual StmtType get_type() = 0; };

void statement_free(Stmt* statement);

struct Stmt_Expression : public Stmt {
	const Expr* expression;

	Stmt_Expression(const Expr* p_expression):
		expression(p_expression) {}

	inline StmtType get_type() override { return StmtType::expression; }
};

struct Stmt_Print : public Stmt {
	const Expr* expression;
	const bool has_newline;

	Stmt_Print(const Expr* p_expression, const bool p_has_newline):
		expression(p_expression), has_newline(p_has_newline) {}

	inline StmtType get_type() override { return StmtType::print; }
};

struct Stmt_Var : public Stmt {
	const Token type;
	const Token name;
	const Expr* initializer;
	const Visibility visibility;
	const bool is_static;
	const bool is_final;

	Stmt_Var(const Token p_type, const Token p_name, const Expr* p_initializer, const Visibility p_visibility, const bool p_is_static, const bool p_is_final):
		type(p_type),
		name(p_name),
		initializer(p_initializer),
		visibility(p_visibility),
		is_static(p_is_static),
		is_final(p_is_final)
	{}

	inline StmtType get_type() override { return StmtType::var; }
};

struct Stmt_Block : public Stmt {
	const std::vector<Stmt*> statements;

	Stmt_Block(const std::vector<Stmt*> p_statements):
		statements(p_statements) {}

	inline StmtType get_type() override { return StmtType::block; }
};
