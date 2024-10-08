#pragma once

#include <vector>
#include <set>

#include "Arena.h"
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
#include "Visibility.h"

class Parser {
public:
	Parser(std::vector<Token>& _tokens);
	~Parser();
	Expr* parse_expression();
	std::vector<Stmt*>* parse_statements();
	void statements_free(std::vector<Stmt*>* statements);

private:
	Stmt* declaration();
	Stmt* statement();
	Stmt* if_statement();
	Stmt* while_statement();
	Stmt* for_statement();
	Stmt* break_statement();
	Stmt* continue_statement();
	Stmt* print_statement(const bool has_newline);
	Stmt* return_statement();
	Stmt* expression_statement();
	std::vector<Stmt*> block_statement();
	std::vector<Stmt*>* heap_block_statement();
	Stmt* complex_var_declaration(TokenType first_modifier);
	Stmt* class_declaration(bool is_abstract);
	Stmt* var_declaration(Token type, Visibility visibility, bool is_static, bool is_final);
	Stmt* fun_declaration(TokenType return_type, Token name, Visibility visibility, bool is_static);

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
	Error error(Token token, const char* fmt, va_list args);
	void synchronize();

	Token consume_java_type(const char *fmt, ...);
	Token consume_no_reset(TokenType type, const char* fmt, ...);
	Token consume(TokenType type, const char *fmt, ...);
	bool match_any_modifier();
	bool match_constructor();
	bool match_java_type();
	bool match(TokenType type);
	bool match(size_t n, ...);
	bool check_java_type();
	bool check(TokenType type);
	bool check_next(TokenType type);
	Token advance();
	inline bool is_at_end();
	inline Token peek();
	inline Token peek_next();
	inline Token previous();

private:
	std::vector<Token> &tokens;
	uint32_t current = 0;
	uint32_t loop_level = 0;
	uint32_t func_level = 0;
	uint32_t class_level = 0;
	std::vector<Expr*> expr_freelist;
	std::vector<Stmt*> stmt_freelist;
public:
	std::set<std::string> class_names;
};
