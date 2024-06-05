#pragma once

#include <stdint.h>
#include "JavaObject.h"

enum class TokenType : uint8_t {
	paren_left = 0, // (
	paren_right,    // )
	curly_left,     // {
	curly_right,    // }
	square_left,    // [
	square_right,   // ]

	plus,  // +
	minus, // -
	slash, // /
	star,  // *
	equal, // =

	percent_sign, // %

	single_quote,  // '
	double_quotes, // "

	greater,       // >
	less,          // <
	greater_equal, // >=
	less_equal,    // <=
	equal_equal,   // ==
	not_equal,     // !=

	colon,    // :
	question, // ?

	bitwise_not, // ~
	bitwise_and, // &
	bitwise_xor, // ^
	bitwise_or,  // |
	left_shift,  // <<
	right_shift, // >>

	identifier, // name
	string,     // "whatever"
	number,     // 69.0f
	character,  // 'a'

	_or,  // or
	_and, // and
	_not, // not

	_for,      // for
	_while,    // while
	_break,    // break
	_continue, // continue
	_return,   // return

	_if,   // if
	_else, // else

	_true,  // true
	_false, // false
	_null,  // null

	super,   // super
	_this,   // this
	extends, // extends
	_class,  // class

	_static,  // static
	_private, // private
	_public,  // public

	type_boolean, // boolean
	type_byte,    // byte
	type_char,    // char
	type_int,     // int
	type_long,    // long
	type_float,   // float
	type_double,  // double

	type_String,    // String
	type_ArrayList, // ArrayList

	import,  // import
	package, // package

	comma,     // ,
	dot,       // .
	semicolon, // ;
	eof,       // EOF

	count,
};

const char* get_token_type_name(TokenType type);

struct Token {
	TokenType type;
	char *lexeme;
	uint32_t line, column;
	JavaObject literal;

	Token();
	Token(TokenType _type, char* _lexeme, size_t lexeme_len, uint32_t _line, uint32_t _column, JavaObject _literal);
	static void assign(Token* a, const Token& other);
};

