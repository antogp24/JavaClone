#include "Token.h"

#include <unordered_map>
#include <assert.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

const char* get_token_type_name(TokenType type) {
	switch (type) {
		case TokenType::paren_left: return "PAREN_LEFT";
		case TokenType::paren_right: return "PAREN_RIGHT";
		case TokenType::curly_left: return "CURLY_LEFT";
		case TokenType::curly_right: return "CURLY_RIGHT";
		case TokenType::square_left: return "SQUARE_LEFT";
		case TokenType::square_right: return "SQUARE_RIGHT";
		case TokenType::plus: return "PLUS";
		case TokenType::minus: return "MINUS";
		case TokenType::slash: return "SLASH";
		case TokenType::star: return "STAR";
		case TokenType::percent_sign: return "PERCENT_SIGN";
		case TokenType::plus_plus: return "PLUS_PLUS";
		case TokenType::minus_minus: return "MINUS_MINUS";
		case TokenType::equal: return "EQUALS";
		case TokenType::single_quote: return "SINGLE_QUOTE";
		case TokenType::double_quotes: return "DOUBLE_QUOTE";
		case TokenType::greater: return "GREATER";
		case TokenType::less: return "LESS";
		case TokenType::greater_equal: return "GREATER_EQUAL";
		case TokenType::less_equal: return "LESS_EQUAL";
		case TokenType::equal_equal: return "EQUAL_EQUAL";
		case TokenType::not_equal: return "NOT_EQUAL";
		case TokenType::colon: return "COLON";
		case TokenType::question: return "QUESTION";
		case TokenType::bitwise_not: return "BITWISE_NOT";
		case TokenType::bitwise_and: return "BITWISE_AND";
		case TokenType::bitwise_xor: return "BITWISE_XOR";
		case TokenType::bitwise_or: return "BITWISE_OR";
		case TokenType::left_shift: return "LEFT_SHIFT";
		case TokenType::right_shift: return "RIGHT_SHIFT";
		case TokenType::identifier: return "IDENTIFIER";
		case TokenType::string: return "STRING";
		case TokenType::number: return "NUMBER";
		case TokenType::character: return "CHARACTER";
		case TokenType::_or: return "OR";
		case TokenType::_and: return "AND";
		case TokenType::_not: return "NOT";
		case TokenType::_for: return "FOR";
		case TokenType::_while: return "WHILE";
		case TokenType::_break: return "BREAK";
		case TokenType::_continue: return "CONTINUE";
		case TokenType::_return: return "RETURN";
		case TokenType::_if: return "IF";
		case TokenType::_else: return "ELSE";
		case TokenType::_true: return "TRUE";
		case TokenType::_false: return "FALSE";
		case TokenType::_null: return "NULL";
		case TokenType::super: return "SUPER";
		case TokenType::_this: return "THIS";
		case TokenType::extends: return "EXTENDS";
		case TokenType::_class: return "CLASS";
		case TokenType::sout: return "SOUT";
		case TokenType::soutln: return "SOUTLN";
		case TokenType::_final: return "FINAL";
		case TokenType::_static: return "STATIC";
		case TokenType::_private: return "PRIVATE";
		case TokenType::_protected: return "PROTECTED";
		case TokenType::_public: return "PUBLIC";
		case TokenType::type_void: return "TYPE_VOID";
		case TokenType::type_boolean: return "TYPE_BOOLEAN";
		case TokenType::type_byte: return "TYPE_BYTE";
		case TokenType::type_char: return "TYPE_CHAR";
		case TokenType::type_int: return "TYPE_INT";
		case TokenType::type_long: return "TYPE_LONG";
		case TokenType::type_float: return "TYPE_FLOAT";
		case TokenType::type_double: return "TYPE_DOUBLE";
		case TokenType::type_String: return "TYPE_STRING";
		case TokenType::type_ArrayList: return "TYPE_ARRAYLIST";
		case TokenType::import: return "IMPORT";
		case TokenType::package: return "PACKAGE";
		case TokenType::comma: return "COMMA";
		case TokenType::dot: return "DOT";
		case TokenType::semicolon: return "SEMICOLON";
		case TokenType::eof: return "EOF";
		default: assert(false && "Not implemented");
	}
	return nullptr;
}

Token::Token() :
	type((TokenType)0),
	lexeme(NULL),
	line(1),
	column(1)
{
	literal = {};
}

Token::Token(TokenType _type, char *_lexeme, size_t lexeme_len, uint32_t _line, uint32_t _column, JavaObject _literal):
	type(_type),
	lexeme(NULL),
	line(_line),
	column(_column),
	literal(_literal)
{
	if (lexeme_len == 0) { return; }
	lexeme = (char*)malloc((lexeme_len + 1) * sizeof(char));
	assert(lexeme != NULL);
	memset(lexeme, 0, lexeme_len + 1);
	strncpy(lexeme, _lexeme, lexeme_len);

	if (type == TokenType::string) {
		literal.value.String = lexeme;
	}
}

void Token::assign(Token *a, const Token& other) {
	a->type = other.type;
	a->line = other.line;
	a->column = other.column;
	a->literal = other.literal;
}

