#pragma once

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string_view>

#include "Token.h"
#include "Error.h"

struct big_string_view {
	char* bytes;
	uint64_t len;
};

typedef std::unordered_map<std::string_view, TokenType> Keywords;
extern Keywords *keywords;
void lexer_fill_keywords();

class Lexer {
public:
	Lexer(char* src, uint64_t len);
	~Lexer();

	void print_tokens();
	const std::vector<Token>& scan();

private:
	void scan_token();
	void scan_identifier();
	void scan_number_literal();
	void scan_string_literal();
	void scan_char_literal();
	inline void scan_multiline_comment();
	inline void add_token(TokenType type);
	inline void add_token(TokenType type, JavaType jtype, JavaValue jvalue);
	inline void add_string_token();
	inline char advance();
	inline bool match(char next);
	inline char peek();
	inline char peek_prev();
	inline char peek_next();
	inline bool is_at_end();
	inline bool is_digit(char c);
	inline bool is_alpha(char c);
	inline bool is_alpha_numeric(char c);

private:
	big_string_view source;
	std::vector<Token> tokens;
	uint64_t start = 0, current = 0;
	uint32_t line = 1, column = 1;
};
