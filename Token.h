#pragma once

#include <stdint.h>
#include "JavaObject.h"
#include "TokenType.h"

struct Token {
	TokenType type;
	char *lexeme;
	uint32_t line, column;
	JavaObject literal;

	bool is__init__ = false;

	Token();
	Token(TokenType _type, char* _lexeme, size_t lexeme_len, uint32_t _line, uint32_t _column, JavaObject _literal);
	static void assign(Token* a, const Token& other);
};

const char* get_token_type_name(TokenType type);
Token make__init__token(Token peeked);
