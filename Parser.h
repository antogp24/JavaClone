#pragma once

#include <vector>

#include "Arena.h"
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
#include "Visibility.h"

class Parser {
public:
	Parser(const std::vector<Token>& _tokens);
	~Parser();
	Expr* parse_expression();
	std::vector<Stmt*>* parse_statements();
	Stmt* statement();
	void statements_free(std::vector<Stmt*>* statements);

private:
	Stmt* print_statement(const bool has_newline);
	Stmt* expression_statement();
	Stmt* block_statement();
	Stmt* declaration();
	Stmt* complex_var_declaration(TokenType first_modifier);
	Stmt* var_declaration(Token type, Visibility visibility, bool is_static, bool is_final);

	Expr* expression();
	Expr* comma_operator();
	Expr* ternary_conditional();
	Expr* assignment();
	Expr* logical_or();
	Expr* logical_and();
	Expr* equality();
	Expr* comparison();
	Expr* bitwise_or();
	Expr* bitwise_xor();
	Expr* bitwise_and();
	Expr* bitwise_shift();
	Expr* term();
	Expr* factor();
	Expr* unary();
	Expr* call();
	Expr* primary();

	class Error {};
	Error error(Token name, const char *fmt, ...);
	void synchronize();

	Token consume(TokenType type, const char *fmt, ...);
	Token consume(TokenType type, Expr* to_free, const char *fmt, ...);
	Token consume(TokenType type, Expr* to_free, std::vector<Expr*> *to_free_list, const char* fmt, ...);
	Token consume(TokenType type, Expr* to_free_0, Expr* to_free_1, const char *fmt, ...);
	Token consume(TokenType type, const std::vector<Stmt*> &to_free, const char *fmt, ...);
	bool match_any_modifier();
	bool match_java_type();
	bool match(TokenType type);
	bool match(size_t n, ...);
	bool check_java_type();
	bool check(TokenType type);
	Token advance();
	inline bool is_at_end();
	inline Token peek();
	inline Token peek_next();
	inline Token previous();

private:
	const std::vector<Token> &tokens;
	uint32_t current = 0;
};
