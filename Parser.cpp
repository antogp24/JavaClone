#include "Parser.h"
#include "Error.h"

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
		Expr *then = expression();
		consume(TokenType::colon, "Expected ':' after then branch in ternary operator.");
		Expr *otherwise = ternary_conditional();
		expr = DBG_new Expr_Ternary{ expr, then, otherwise };
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

		error(equals, "Invalid assignment target.");
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
						error(peek(), "Can't have more than 255 arguments.");
					}
					// Always call 1 level of precedence above the comma operator.
					Expr* argument_expr = ternary_conditional();
					arguments->emplace_back(argument_expr);
				} while (match(TokenType::comma));
			}
			Token paren = consume(TokenType::paren_right, "Expected ')' after function call.");
			expr = DBG_new Expr_Call{expr, paren, arguments};
		}
		else if (match(TokenType::dot)) {
			Token name = consume(TokenType::identifier, "Expected property name after '.'.");
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

Parser::Error Parser::error(Token name, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	JavaError::error(name, fmt, ap);
	va_end(ap);
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
	throw error(peek(), fmt, ap);
	va_end(ap);
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

bool Parser::check(TokenType type) {
	if (is_at_end()) return false;
	return peek().type == type;
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

inline Token Parser::previous() {
	return tokens[current - 1];
}
