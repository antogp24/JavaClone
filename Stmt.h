#pragma once

#include <string>
#include "Expr.h"
#include "Visibility.h"

enum class StmtType {
	Break,
	Block,
	Continue,
	Expression,
	Function,
	If,
	Print,
	Return,
	Var,
	While,
};


struct Stmt { inline virtual StmtType get_type() = 0; };

void stmt_free(Stmt* statement);

struct Stmt_Break : public Stmt {
	inline StmtType get_type() override { return StmtType::Break; }
};

struct Stmt_Block : public Stmt {
	std::vector<Stmt*> statements;

	Stmt_Block(std::vector<Stmt*> p_statements):
		statements(p_statements) {}

	inline StmtType get_type() override { return StmtType::Block; }
};

struct Stmt_Continue : public Stmt {
	inline StmtType get_type() override { return StmtType::Continue; }
};

struct Stmt_Expression : public Stmt {
	const Expr* expression;

	Stmt_Expression(const Expr* p_expression):
		expression(p_expression) {}

	inline StmtType get_type() override { return StmtType::Expression; }
};

struct JavaTypeInfo {
	JavaType type;
	std::string name;
};

struct Stmt_Function : public Stmt {
	const JavaType return_type;
	const Token name;
	const Visibility visibility;
	const bool is_static;
	const std::vector<std::pair<JavaTypeInfo, std::string>>* params;
	const std::vector<Stmt*>* body;

	Stmt_Function(const JavaType p_return_type,
				  const Token p_name,
				  const Visibility p_visibility,
				  const bool p_is_static,
				  const std::vector<std::pair<JavaTypeInfo, std::string>>* p_params,
				  const std::vector<Stmt*>* p_body):
		return_type(p_return_type),
		name(p_name),
		visibility(p_visibility),
		is_static(p_is_static),
		params(p_params),
		body(p_body)
	{}

	inline StmtType get_type() override { return StmtType::Function; }
};

struct Stmt_Print : public Stmt {
	const Token token;
	const Expr* expression;
	const bool has_newline;

	Stmt_Print(const Token p_token, const Expr* p_expression, const bool p_has_newline):
		token(p_token), expression(p_expression), has_newline(p_has_newline) {}

	inline StmtType get_type() override { return StmtType::Print; }
};

struct Stmt_Return : public Stmt {
	const std::string keyword;
	uint32_t line, column;
	const Expr* value;

	Stmt_Return(const std::string p_keyword, uint32_t p_line, uint32_t p_column, const Expr* p_value):
		keyword(p_keyword), line(p_line), column(p_column), value(p_value) {}

	inline StmtType get_type() override { return StmtType::Return; }
};

struct Stmt_Var : public Stmt {
	const Token type;
	const std::vector<Token> names;
	const std::vector<Expr*> initializers;
	const Visibility visibility;
	const bool is_static;
	const bool is_final;

	Stmt_Var(const Token p_type, const std::vector<Token> p_names, const std::vector<Expr*> p_initializers, const Visibility p_visibility, const bool p_is_static, const bool p_is_final):
		type(p_type),
		names(p_names),
		initializers(p_initializers),
		visibility(p_visibility),
		is_static(p_is_static),
		is_final(p_is_final)
	{}

	inline StmtType get_type() override { return StmtType::Var; }
};

struct Else_If {
	const Token token;
	const Expr* condition;
	const Stmt* then_branch;
};

struct Stmt_If : public Stmt {
	const Token token;
	const Expr* condition;
	const Stmt* then_branch;
	const std::vector<Else_If> else_ifs;
	const Stmt* else_branch;

	Stmt_If(const Token p_token, const Expr* p_condition, const Stmt* p_then_branch, const std::vector<Else_If> p_else_ifs, const Stmt* p_else_branch) :
		token(p_token), condition(p_condition), then_branch(p_then_branch), else_ifs(p_else_ifs), else_branch(p_else_branch)
	{}

	inline StmtType get_type() override { return StmtType::If; }
};

struct Stmt_While : public Stmt {
	const Token token;
	const Expr* condition;
	const Stmt* body;
	const bool has_increment;

	Stmt_While(const Token p_token, const Expr* p_condition, const Stmt* p_body, const bool p_has_increment):
		token(p_token), condition(p_condition), body(p_body), has_increment(p_has_increment)
	{}

	inline StmtType get_type() override { return StmtType::While; }
};
