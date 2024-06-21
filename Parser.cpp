#include "Parser.h"
#include "Error.h"
#include "AstPrinter.h"

#include <stdarg.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif


Parser::Parser(const std::vector<Token>& _tokens):
	tokens(_tokens)
{
}

Parser::~Parser() {
}

Expr *Parser::parse_expression() {
	try {
		return expression();
	}
	catch (Error error) {
		(void)error;
		return nullptr;
	}
}

std::vector<Stmt*>* Parser::parse_statements() {
	std::vector<Stmt*>* statements = DBG_new std::vector<Stmt*>();

	while (!is_at_end()) {
		try {
			statements->emplace_back(declaration());
		}
		catch (Error error) {
			(void)error;
			delete statements;
			synchronize();
			return nullptr;
		}
	}

	return statements;
}

void Parser::statements_free(std::vector<Stmt*>* statements) {
	if (statements == nullptr) return;
	for (int i = 0; i < statements->size(); i++) {
		statement_free(statements->at(i));
	}
	delete statements;
}

Stmt* Parser::declaration() {
	if (match_java_type()) return var_declaration(previous(), Visibility::Package, false, false);
	if (match_any_modifier()) return complex_var_declaration(previous().type);

	return statement();
}

Stmt* Parser::complex_var_declaration(TokenType first_modifier) {
	enum { STATIC = 0, VISIBILITY, FINAL, COUNT };
	size_t counts[COUNT] = {0};

	// If there is a visibility modifier, which one it is?
	Visibility visibility = Visibility::Package;
	if (is_token_type_visibility(first_modifier)) {
		visibility = visibility_from_token_type(previous().type);
	}

	auto hash = [this](TokenType type) { 
		switch (type) {
			case TokenType::_static:    return STATIC;
			case TokenType::_public:  // falltrough
			case TokenType::_private: // falltrough
			case TokenType::_protected: return VISIBILITY;
			case TokenType::_final:     return FINAL;
		}
		throw error(previous(), "Expected a valid entry in the counts array.");
	};
	auto check_counts = [this](size_t counts[COUNT]) {
		if ((counts[STATIC] > 1) ||  (counts[VISIBILITY] > 1) || (counts[FINAL] > 1)) {
			throw error(previous(), "Modifiers must appear only once.");
		}
	};

	counts[hash(first_modifier)] = 1;

	if (!check(TokenType::identifier)) {
		while (!is_at_end() && !check_java_type()) {
			if (match_any_modifier()) {
				if (is_token_type_visibility(previous().type)) {
					visibility = visibility_from_token_type(previous().type);
				}
				counts[hash(previous().type)] += 1;
				check_counts(counts);
			}
			else if (!check_java_type()) {
				throw error(peek(), "Unexpected token in the modifiers.");
			}
		}
	}

	if (!check_java_type()) {
		throw error(previous(), "Expected type after the modifiers.");
	}
	Token type = advance();

	return var_declaration(type, visibility, counts[STATIC] == 1, counts[FINAL] == 1);
}

Stmt* Parser::var_declaration(Token type, Visibility visibility, bool is_static, bool is_final) {
	std::vector<Token> names = {};
	std::vector<Expr*> initializers = {};

	Token first_name = consume(TokenType::identifier, "Expected variable name.");
	names.push_back(first_name);
	
	Expr* first_initializer = nullptr;
	if (match(TokenType::equal)) {
		// Always call one level of precedence above the comma operator.
		first_initializer = ternary_conditional();
	}
	if (first_initializer == nullptr && is_final) {
		throw error(previous(), "Constant must have an initializer.");
	}
	initializers.push_back(first_initializer);

	while (match(TokenType::comma)) {
		Token name = consume(TokenType::identifier, "Expected variable name.");
		names.push_back(name);

		Expr* initializer = nullptr;
		if (match(TokenType::equal)) {
			// Always call one level of precedence above the comma operator.
			initializer = ternary_conditional();
		}
		if (initializer == nullptr && is_final) {
			for (Expr* it : initializers) {
				if (it != nullptr) {
					expression_free(it);
				}
			}
			throw error(previous(), "Constant must have an initializer.");
		}
		initializers.push_back(initializer);
	}

	if (!check(TokenType::semicolon)) {
		for (Expr* initializer : initializers) {
			if (initializer != nullptr) {
				expression_free(initializer);
			}
		}
		throw error(previous(), "Expected ';' after variable declaration.");
	}
	advance();

	return DBG_new Stmt_Var{type, names, initializers, visibility, is_static, is_final};
}

Stmt* Parser::statement() {
	if (match(TokenType::sout)) return print_statement(false);
	if (match(TokenType::soutln)) return print_statement(true);
	if (match(TokenType::curly_left)) return block_statement();
	if (match(TokenType::_if)) return if_statement();
	if (match(TokenType::_while)) return while_statement();
	if (match(TokenType::_for)) return for_statement();
	if (match(TokenType::_break)) return break_statement();
	if (match(TokenType::_continue)) return continue_statement();

	return expression_statement();
}

Stmt* Parser::continue_statement() {
	if (loop_level == 0) throw error(previous(), "Can't use continue statement outside a loop");
	consume(TokenType::semicolon, "Expected ';' after continue statement.");
	return DBG_new Stmt_Continue{};
}

Stmt* Parser::break_statement() {
	if (loop_level == 0) throw error(previous(), "Can't use break statement outside a loop");
	consume(TokenType::semicolon, "Expected ';' after break statement.");
	return DBG_new Stmt_Break{};
}

Stmt* Parser::for_statement() {
	this->loop_level++;
	Token token = previous();
	consume(TokenType::paren_left, "Expect '(' before 'for' initializer.");

	Stmt* initializer;
	if (match(TokenType::semicolon)) {
		initializer = nullptr;
	}
	else if (match_java_type()) {
		initializer = var_declaration(previous(), Visibility::Local, false, false);
	}
	else {
		initializer = expression_statement();
	}

	Expr* condition = nullptr;
	if (!check(TokenType::semicolon)) {
		condition = parse_expression();
	}
	if (!match(TokenType::semicolon)) {
		statement_free(initializer);
		expression_free(condition);
		throw JAVA_RUNTIME_ERROR(previous(), "Expected ';' after 'for' condition.");
	}

	Expr* increment = nullptr;
	if (!check(TokenType::paren_right)) {
		increment = parse_expression();
	}
	if (!match(TokenType::paren_right)) {
		statement_free(initializer);
		expression_free(condition);
		expression_free(increment);
		throw JAVA_RUNTIME_ERROR(previous(), "Expected ')' after 'for' increment.");
	}

	Stmt* body = statement();
	this->loop_level--;

	if (increment != nullptr) {
		Stmt_Expression* increment_statement = DBG_new Stmt_Expression{increment};
		if (body->get_type() == StmtType::Block) {
			Stmt_Block* block = dynamic_cast<Stmt_Block*>(body);
			block->statements.push_back(increment_statement);
		}
		else {
			std::vector<Stmt*> statements;
			statements.emplace_back(body);
			statements.emplace_back((Stmt*)increment_statement);
			body = DBG_new Stmt_Block{ statements };
		}
	}

	if (condition == nullptr) {
		JavaObject literal = { JavaType::_boolean, JavaValue{} };
		literal.value._boolean = true;
		condition = DBG_new Expr_Literal{literal};
	}
	body = DBG_new Stmt_While{token, condition, body, increment != nullptr};

	if (initializer != nullptr) {
		std::vector<Stmt*> statements;
		statements.emplace_back(initializer);
		statements.emplace_back(body);
		body = DBG_new Stmt_Block{ statements };
	}

	return body;
}

Stmt* Parser::while_statement() {
	this->loop_level++;
	Token token = previous();
	consume(TokenType::paren_left, "Expect '(' before 'while' condition.");
	Expr* condition = parse_expression();
	consume(TokenType::paren_right, condition, "Expect ')' after 'while' condition.");
	Stmt* body = statement();
	this->loop_level--;

	return DBG_new Stmt_While{token, condition, body, false};
}

Stmt* Parser::if_statement() {
	Token token = previous();

	consume(TokenType::paren_left, "Expect '(' after 'if'.");
	Expr* condition = parse_expression();
	consume(TokenType::paren_right, condition, "Expect ')' after condition in 'if'.");
	if (is_at_end()) {
		expression_free(condition);
		throw error(peek(), "Expect statement after ')' in 'if'.");
	}
	Stmt* then_branch = statement();

	std::vector<Else_If> else_ifs = {};
	while (check(TokenType::_else)) {
		if (peek_next().type == TokenType::_if) {
			advance(); // consume else
			Token else_if_token = advance(); // consume if
			if (!match(TokenType::paren_left)) {
				expression_free(condition);
				statement_free(then_branch);
				for (Else_If else_if : else_ifs) {
					if (else_if.condition != nullptr) expression_free((Expr*)else_if.condition);
					if (else_if.then_branch != nullptr) statement_free((Stmt*)else_if.then_branch);
				}
				throw error(peek(), "Expected '(' after 'else if'.");
			}
			Expr* else_if_condition = parse_expression();
			if (!match(TokenType::paren_right)) {
				expression_free(condition);
				statement_free(then_branch);
				expression_free(else_if_condition);
				for (Else_If else_if : else_ifs) {
					if (else_if.condition != nullptr) expression_free((Expr*)else_if.condition);
					if (else_if.then_branch != nullptr) statement_free((Stmt*)else_if.then_branch);
				}
				throw error(peek(), "Expected ')' after condition in 'else if'.");
			}
			if (is_at_end()) {
				expression_free(condition);
				statement_free(then_branch);
				expression_free(else_if_condition);
				for (Else_If else_if : else_ifs) {
					if (else_if.condition != nullptr) expression_free((Expr*)else_if.condition);
					if (else_if.then_branch != nullptr) statement_free((Stmt*)else_if.then_branch);
				}
				throw error(peek(), "Expected statement after ')' in 'else if'.");
			}
			Stmt* else_if_then_branch = statement();
			else_ifs.emplace_back(else_if_token, else_if_condition, else_if_then_branch);
		}
		else {
			break;
		}
	}

	Stmt* else_branch = nullptr;
	if (match(TokenType::_else)) {
		else_branch = statement();
	}

	return DBG_new Stmt_If{token, condition, then_branch, else_ifs, else_branch};
}

Stmt* Parser::print_statement(const bool has_newline) {
	consume(TokenType::paren_left, "Expected '(' before expression in print statement.");
	Expr* value = parse_expression();
	consume(TokenType::paren_right, value, "Expected ')' after expression in print statement.");
	consume(TokenType::semicolon, value, "Expected ';' after ')' in print statement.");
	return DBG_new Stmt_Print(value, has_newline);
}

Stmt* Parser::expression_statement() {
	Expr* value = parse_expression();
	consume(TokenType::semicolon, value, "Expected ';' after value in expression statement.");
	return DBG_new Stmt_Expression(value);
}

Stmt* Parser::block_statement() {
	std::vector<Stmt*> statements = {};

	while (!check(TokenType::curly_right) && !is_at_end()) {
		statements.emplace_back(declaration());
	}

	consume(TokenType::curly_right, statements, "Expect '}' at the end of the block.");
	return DBG_new Stmt_Block{ statements };
}

Expr* Parser::expression() {
	return comma_operator();
}

Expr* Parser::comma_operator() {
	Expr* expr = ternary_conditional();

	while (match(TokenType::comma)) {
		expression_free(expr);
		expr = ternary_conditional();
	}

	return expr;
}

Expr* Parser::ternary_conditional() {
	Expr *expr = assignment();

	if (match(TokenType::question)) {
		Token question_mark = previous();
		if (is_at_end()) {
			expression_free(expr);
			throw error(question_mark, "Expected then branch after '?' in ternary.");
		}
		Expr *then = expression();
		consume(TokenType::colon, expr, then, "Expected ':' after then branch in ternary operator.");
		Expr *otherwise = ternary_conditional();
		expr = DBG_new Expr_Ternary{ expr, then, otherwise, question_mark };
	}

	return expr;
}

Expr* Parser::assignment() {
	Expr* expr = logical_or();

	if (match(TokenType::equal)) {
		Token equals = previous();
		Expr* rhs = assignment();

		switch (expr->get_type()) {
			case ExprType::variable: {
				Expr_Variable* variable = dynamic_cast<Expr_Variable*>(expr);
				return DBG_new Expr_Assign{variable, variable->name, rhs};
			}
			case ExprType::get: {
				Expr_Get* get = dynamic_cast<Expr_Get*>(expr);
				return DBG_new Expr_Set{get, get->name, rhs};
			}
		}

		expression_free(expr);
		expression_free(rhs);
		throw error(equals, "Invalid assignment target.");
	}

	return expr;
}

Expr* Parser::logical_or() {
	Expr* expr = logical_and();

	while (match(TokenType::_or)) {
		Token _operator = previous();
		Expr *right = logical_and();
		expr = DBG_new Expr_Logical{expr, _operator, right};
	}

	return expr;
}

Expr* Parser::logical_and() {
	Expr* expr = equality();

	while (match(TokenType::_and)) {
		Token _operator = previous();
		Expr *right = equality();
		expr = DBG_new Expr_Logical{expr, _operator, right};
	}

	return expr;
}

Expr *Parser::equality() {
	Expr *expr = comparison();

	while (match(2, TokenType::not_equal, TokenType::equal_equal)) {
		Token _operator = previous();
		Expr *right = comparison();
		expr = DBG_new Expr_Binary{expr, _operator, right};
	}
	return expr;
}

Expr *Parser::comparison() {
	Expr* expr = bitwise_or();
	
	while (match(4, TokenType::greater, TokenType::greater_equal, TokenType::less, TokenType::less_equal)) {
		Token _operator = previous();
		Expr* right = bitwise_or();
		expr = DBG_new Expr_Binary{expr, _operator, right};
	}

	return expr;
}

Expr* Parser::bitwise_or() {
	Expr* expr = bitwise_xor();

	while (match(TokenType::bitwise_or)) {
		Token _operator = previous();
		Expr* right = bitwise_xor();
		expr = DBG_new Expr_Binary{expr, _operator, right};
	}

	return expr;
}

Expr* Parser::bitwise_xor() {
	Expr* expr = bitwise_and();

	while (match(TokenType::bitwise_xor)) {
		Token _operator = previous();
		Expr* right = bitwise_and();
		expr = DBG_new Expr_Binary{expr, _operator, right};
	}

	return expr;
}

Expr* Parser::bitwise_and() {
	Expr* expr = bitwise_shift();

	while (match(TokenType::bitwise_and)) {
		Token _operator = previous();
		Expr* right = bitwise_shift();
		expr = DBG_new Expr_Binary{expr, _operator, right};
	}

	return expr;
}

Expr* Parser::bitwise_shift() {
	Expr* expr = term();

	while (match(2, TokenType::left_shift, TokenType::right_shift)) {
		Token _operator = previous();
		Expr* right = term();
		expr = DBG_new Expr_Binary{expr, _operator, right};
	}

	return expr;
}

Expr *Parser::term() {
	Expr* expr = factor();

	while (match(2, TokenType::minus, TokenType::plus)) {
		Token _operator = previous();
		Expr* right = factor();
		expr = DBG_new Expr_Binary{expr, _operator, right};
	}

	return expr;
}

Expr *Parser::factor() {
	Expr* expr = unary();

	while (match(3, TokenType::slash, TokenType::star, TokenType::percent_sign)) {
		Token _operator = previous();
		Expr* right = unary();
		expr = DBG_new Expr_Binary{expr, _operator, right};
	}

	return expr;
}

Expr *Parser::unary() {
	if (match(3, TokenType::_not, TokenType::minus, TokenType::bitwise_not)) {
		Token _operator = previous();
		Expr* right = unary();
		return DBG_new Expr_Unary{_operator, right};
	}

	// Prefix -- ++
	if (match(2, TokenType::plus_plus, TokenType::minus_minus)) {
		bool is_positive = previous().type == TokenType::plus_plus;
		Token name = consume(TokenType::identifier, "Expected identifier after prefix '%s'.", previous().lexeme);
		return DBG_new Expr_Increment{ name, is_positive };
	}

	// Postfix -- ++
	if (check(TokenType::identifier) && (check_next(TokenType::plus_plus) || check_next(TokenType::minus_minus))) {
		Token name = advance();
		bool is_positive = advance().type == TokenType::plus_plus;
		return DBG_new Expr_Increment{ name, is_positive };
	}

	if (match(TokenType::paren_left)) {
		switch (peek().type) {
			case TokenType::type_boolean:
			case TokenType::type_byte:
			case TokenType::type_char:
			case TokenType::type_int:
			case TokenType::type_long:
			case TokenType::type_float:
			case TokenType::type_double: {
				Token type = advance();
				consume(TokenType::paren_right, "Expected ')' after type in cast");
				Expr* right = unary();
				return DBG_new Expr_Unary{type, right};
			} break;

			default: {
				if (current - 1 >= 0) current--;
			} break;
		}
	}

	return call();
}

Expr* Parser::call() {
	Expr* expr = primary();

	while (true) {
		if (match(TokenType::paren_left)) {
			std::vector<Expr*> *arguments = DBG_new std::vector<Expr*>;
			if (!check(TokenType::paren_right)) {
				do {
					if (arguments->size() >= 255) {
						expression_free(expr);
						for (int i = 0; i < arguments->size(); i++)
							expression_free(arguments->at(i));
						delete arguments;
						throw error(peek(), "Can't have more than 255 arguments.");
					}
					// Always call 1 level of precedence above the comma operator.
					Expr* argument_expr = ternary_conditional();
					arguments->emplace_back(argument_expr);
				} while (match(TokenType::comma));
			}
			Token paren = consume(TokenType::paren_right, expr, arguments, "Expected ')' after function call.");
			expr = DBG_new Expr_Call{expr, paren, arguments};
		}
		else if (match(TokenType::dot)) {
			Token name = consume(TokenType::identifier, expr, "Expected property name after '.'.");
			expr = DBG_new Expr_Get{expr, name};
		}
		else {
			break;
		}
	}

	return expr;
}

Expr *Parser::primary() {
	if (match(TokenType::_false)) {
		JavaValue value = {};
		value._boolean = false;
		return DBG_new Expr_Literal{ JavaObject{JavaType::_boolean, value} };
	}
	if (match(TokenType::_true)) {
		JavaValue value = {};
		value._boolean = true;
		return DBG_new Expr_Literal{ JavaObject{JavaType::_boolean, value} };
	}
	if (match(TokenType::_null)) {
		return DBG_new Expr_Literal{ JavaObject{JavaType::_null, JavaValue{}} };
	}

	if (match(3, TokenType::number, TokenType::string, TokenType::character)) {
		return DBG_new Expr_Literal{ previous().literal };
	}

	if (match(TokenType::identifier)) {
		bool is_function = (peek().type == TokenType::paren_left);
		return DBG_new Expr_Variable{previous(), is_function};
	}

	if (match(TokenType::paren_left)) {
		Expr* expr = expression();
		consume(TokenType::paren_right, "Expected closing ')'.");
		return DBG_new Expr_Grouping{ expr };
	}

	throw error(peek(), "Expected expression.");
}

Parser::Error Parser::error(Token token, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	JavaError::error(token, fmt, args);
	va_end(args);
	return {};
}

Parser::Error Parser::error(Token token, const char *fmt, va_list args) {
	JavaError::error(token, fmt, args);
	return {};
}

void Parser::synchronize() {
	advance();

	while (!is_at_end()) {
		if (previous().type == TokenType::semicolon) return;

		switch (peek().type) {
		case TokenType::_class:
		case TokenType::_static:
		case TokenType::_public:
		case TokenType::_private:
		case TokenType::type_byte:
		case TokenType::type_char:
		case TokenType::type_int:
		case TokenType::type_long:
		case TokenType::type_float:
		case TokenType::type_double:
		case TokenType::type_ArrayList:
		case TokenType::type_String:
		case TokenType::_for:
		case TokenType::_if:
		case TokenType::_while:
		case TokenType::_return:
		case TokenType::_break:
		case TokenType::_continue:
			return;
		}

		advance();
	}
}

Token Parser::consume(TokenType type, const char* fmt, ...) {
	if (check(type)) return advance();

	va_list ap;
	va_start(ap, fmt);
	Error e = error(peek(), fmt, ap);
	va_end(ap);
	throw e;
}

Token Parser::consume(TokenType type, Expr* to_free, const char* fmt, ...) {
	if (check(type)) return advance();

	expression_free(to_free);
	va_list ap;
	va_start(ap, fmt);
	Error e = error(peek(), fmt, ap);
	va_end(ap);
	throw e;
}

Token Parser::consume(TokenType type, Expr* to_free, std::vector<Expr*> *to_free_list, const char* fmt, ...) {
	if (check(type)) return advance();

	expression_free(to_free);
	if (to_free_list != nullptr) {
		for (int i = 0; i < to_free_list->size(); i++) {
			expression_free(to_free_list->at(i));
		}
		delete to_free_list;
	}
	va_list ap;
	va_start(ap, fmt);
	Error e = error(peek(), fmt, ap);
	va_end(ap);
	throw e;
}

Token Parser::consume(TokenType type, Expr* to_free_0, Expr* to_free_1, const char* fmt, ...) {
	if (check(type)) return advance();

	expression_free(to_free_0);
	expression_free(to_free_1);
	va_list ap;
	va_start(ap, fmt);
	Error e = error(peek(), fmt, ap);
	va_end(ap);
	throw e;
}

Token Parser::consume(TokenType type, const std::vector<Stmt*> &to_free, const char* fmt, ...) {
	if (check(type)) return advance();

	for (int i = 0; i < to_free.size(); i++) {
		statement_free(to_free.at(i));
	}

	va_list ap;
	va_start(ap, fmt);
	Error e = error(peek(), fmt, ap);
	va_end(ap);
	throw e;
}

bool Parser::match_any_modifier() {
	bool any = check(TokenType::_static)    ||
			   check(TokenType::_final)     ||
			   check(TokenType::_public)    ||
			   check(TokenType::_private)   ||
			   check(TokenType::_protected);
	if (any) advance();
	return any;
}

bool Parser::match_java_type() {
	bool is_type = check_java_type();
	if (is_type) advance();
	return is_type;
}

bool Parser::match(TokenType type) {
	if (check(type)) {
		advance();
		return true;
	}
	return false;
}

bool Parser::match(size_t n, ...) {
	va_list ap;
	va_start(ap, n);

	for (int i = 0; i < n; i++) {
		TokenType type = va_arg(ap, TokenType);
		if (check(type)) {
			advance();
			return true;
		}
	}

	va_end(ap);
	return false;
}


bool Parser::check_java_type() {
	if (is_at_end()) return false;
	return peek().type == TokenType::type_boolean ||
		   peek().type == TokenType::type_byte    ||
	       peek().type == TokenType::type_char    || 
	       peek().type == TokenType::type_int     || 
	       peek().type == TokenType::type_long    || 
	       peek().type == TokenType::type_float   || 
	       peek().type == TokenType::type_double  || 
	       peek().type == TokenType::type_String  || 
	       peek().type == TokenType::type_ArrayList;
}

bool Parser::check(TokenType type) {
	if (is_at_end()) return false;
	return peek().type == type;
}

bool Parser::check_next(TokenType type) {
	if (peek_next().type == TokenType::eof) return false;
	return peek_next().type == type;
}

Token Parser::advance() {
	if (!is_at_end()) current++;
	return previous();
}

inline bool Parser::is_at_end() {
	return peek().type == TokenType::eof;
}

inline Token Parser::peek() {
	return tokens[current];
}

inline Token Parser::peek_next() {
	return tokens[current + 1];
}

inline Token Parser::previous() {
	return tokens[current - 1];
}
