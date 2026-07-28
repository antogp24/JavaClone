#pragma once

#include <stdint.h>

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

	plus_plus,   // ++
	minus_minus, // --

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

	super,     // super
	_this,     // this
	extends,   // extends
	_abstract, // abstract
	_class,    // class

	sout, // sout
	soutln, // soutln

	_final,     // final
	_static,    // static
	_private,   // private
	_protected, // protected
	_public,    // public

	type_void,    // void
	type_boolean, // boolean
	type_byte,    // byte
	type_char,    // char
	type_int,     // int
	type_long,    // long
	type_float,   // float
	type_double,  // double

	type_String,       // String
	type_ArrayList,    // ArrayList
	type_user_defined, // The name of a class defined by the user.

	constructor, // __init__

	import,  // import
	package, // package

	comma,     // ,
	dot,       // .
	semicolon, // ;
	eof,       // EOF

	count,
};
