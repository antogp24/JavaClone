#pragma once

#include <vector>

#include "Arena.h"
#include "Token.h"
#include "Expr.h"

class Parser {
public:
	Parser(const std::vector<Token>& _tokens);
	~Parser();
	Expr *parse_expression();

private:
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
	bool match(TokenType type);
	bool match(size_t n, ...);
	bool check(TokenType type);
	Token advance();
	inline bool is_at_end();
	inline Token peek();
	inline Token previous();

private:
	const std::vector<Token> &tokens;
	uint32_t current = 0;
};
