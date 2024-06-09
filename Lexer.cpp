#include "Lexer.h"

#include <assert.h>
#include <unordered_map>
#include <string>
#include <string_view>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

void lexer_fill_keywords() {
	keywords->insert({ "true", TokenType::_true });
	keywords->insert({ "false", TokenType::_false });
	keywords->insert({ "null", TokenType::_null });
	keywords->insert({ "if", TokenType::_if });
	keywords->insert({ "else", TokenType::_else });
	keywords->insert({ "for", TokenType::_for });
	keywords->insert({ "while", TokenType::_while });
	keywords->insert({ "break", TokenType::_break });
	keywords->insert({ "continue", TokenType::_continue });
	keywords->insert({ "return", TokenType::_return });
	keywords->insert({ "public", TokenType::_public });
	keywords->insert({ "protected", TokenType::_protected });
	keywords->insert({ "private", TokenType::_private });
	keywords->insert({ "final", TokenType::_final });
	keywords->insert({ "static", TokenType::_static });
	keywords->insert({ "extends", TokenType::extends });
	keywords->insert({ "class", TokenType::_class });
	keywords->insert({ "sout", TokenType::sout });
	keywords->insert({ "soutln", TokenType::soutln });
	keywords->insert({ "super", TokenType::super });
	keywords->insert({ "this", TokenType::_this });
	keywords->insert({ "boolean", TokenType::type_boolean });
	keywords->insert({ "byte", TokenType::type_byte });
	keywords->insert({ "char", TokenType::type_char });
	keywords->insert({ "int", TokenType::type_int });
	keywords->insert({ "long", TokenType::type_long });
	keywords->insert({ "float", TokenType::type_float });
	keywords->insert({ "double", TokenType::type_double });
	keywords->insert({ "String", TokenType::type_String });
	keywords->insert({ "ArrayList", TokenType::type_ArrayList });
}

Lexer::Lexer(char* src, uint64_t len): source({ src, len })
{
}

Lexer::~Lexer() {
	for (auto& token : tokens) {
		if (token.lexeme != NULL) {
			free(token.lexeme);
			token.lexeme = NULL;
		}
	}
}

void Lexer::print_tokens() {
	printf("Tokens: { ");
	for (const Token& token : tokens) {
		printf("(%s", get_token_type_name(token.type));
		if (token.lexeme != NULL) printf(" \"%s\"", token.lexeme);
		switch (token.literal.type) {
			case JavaType::_long: printf(" %lld", token.literal.value._long); break;
			case JavaType::_double: printf(" %f", token.literal.value._double); break;
		}
		printf(") ");
	}
	printf("}\n");
}

const std::vector<Token>& Lexer::scan() { 
	while (current < source.len) {
		start = current;
		scan_token();
	}
	start = current;

	add_token(TokenType::eof, JavaType::none, {0});
	return tokens;
}

void Lexer::scan_token() {
	char c = advance();

	switch (c) {
		case '(': add_token(TokenType::paren_left); break;
		case ')': add_token(TokenType::paren_right); break;
		case '{': add_token(TokenType::curly_left); break;
		case '}': add_token(TokenType::curly_right); break;
		case '[': add_token(TokenType::square_left); break;
		case ']': add_token(TokenType::square_right); break;
		case '&': add_token(match('&') ? TokenType::_and : TokenType::bitwise_and); break;
		case '|': add_token(match('|') ? TokenType::_or : TokenType::bitwise_or); break;
		case '^': add_token(TokenType::bitwise_xor); break;
		case '~': add_token(TokenType::bitwise_not); break;
		case ',': add_token(TokenType::comma); break;
		case '.': {
			if (is_digit(peek()))
				JavaError::error(line, column, "There must be a number before the dot in the double or float literal.");
			else
				add_token(TokenType::dot);
		} break;
		case '-': add_token(match('-') ? TokenType::minus_minus : TokenType::minus); break;
		case '+': add_token(match('+') ? TokenType::plus_plus : TokenType::plus); break;
		case '*': add_token(TokenType::star); break;
		case '%': add_token(TokenType::percent_sign); break;
		case ';': add_token(TokenType::semicolon); break;
		case ':': add_token(TokenType::colon); break;
		case '?': add_token(TokenType::question); break;
		case '=': add_token(match('=') ? TokenType::equal_equal : TokenType::equal); break;
		case '!': add_token(match('=') ? TokenType::not_equal : TokenType::_not); break;
		case '<': {
			if (match('=')) {
				add_token(TokenType::less_equal);
			}
			else if (match('<')) {
				add_token(TokenType::left_shift);
			}
			else {
				add_token(TokenType::less);
			}
		} break;
		case '>': {
			if (match('=')) {
				add_token(TokenType::greater_equal);
			}
			else if (match('>')) {
				add_token(TokenType::right_shift);
			}
			else {
				add_token(TokenType::greater);
			}
		} break;
		case '/': {
			if (match('/')) {
				// ignore single line comment.
				while (peek() != '\n' && !is_at_end()) advance();
			}
			else if (match('*')) {
				scan_multiline_comment();
			}
			else {
				add_token(TokenType::slash);
			}
		} break;

		// ignore whitespace
		case ' ':
		case '\0':
		case '\t':
		case '\r': break;

		case '\n': { column = 1; line++; } break;
		case '\"': scan_string_literal(); break;
		case '\'': scan_char_literal(); break;

		default: {
			if (is_digit(c)) {
				scan_number_literal();
			}
			else if (is_alpha(c)) {
				scan_identifier();
			}
			else {
				JavaError::error(line, column, "Unexpected ascii character '%c' (%i).", c, c);
			}
		} break;
	}
}

void Lexer::scan_identifier() {
	while (is_alpha_numeric(peek())) advance();

	std::string name(&source.bytes[start], current - start);
	name.append("\0");

	TokenType type = keywords->contains(name) ? keywords->at(name) : TokenType::identifier;
	add_token(type);
}

void Lexer::scan_number_literal() {
	enum class Number_Type : uint8_t { _long, _double, _float };
	Number_Type number_type = Number_Type::_long;

	while (is_digit(peek())) advance();

	if (peek() == '.' && is_digit(peek_next())) {
		number_type = Number_Type::_double;
		advance();
		while (!is_at_end() && is_digit(peek())) {
			if (peek_next() == 'f') {
				number_type = Number_Type::_float;
				advance();
			}
			advance();
		}
	}
	else if (peek() == '.' && peek_next() == 'f') {
		JavaError::error(line, column, "There must be a number between the dot and the 'f' in the float literal.");
	}
	else if (peek() == '.' && !is_digit(peek_next())) {
		JavaError::error(line, column, "There must be a number after the dot in the double literal.");
	}

	std::string name(&source.bytes[start], current - start);
	name.append("\0");

	JavaValue value = {};

	#define case_number(T, strconv_fn)                           \
		case Number_Type::##T: {                                 \
			value.##T = (Java##T)strconv_fn(name.c_str());       \
			add_token(TokenType::number, JavaType::##T, value);  \
		} break;

	switch (number_type) {
		case_number(_long, atoll)
		case_number(_float, atof)
		case_number(_double, atof)
	}
	#undef case_number
}

void Lexer::scan_string_literal() {
	while (peek() != '\"' && !is_at_end()) {
		if (peek() == '\n') {
			column = 1;
			line++;
		}
		if (peek() == '\\') {
			advance();
			switch (peek()) {
				case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				case 't': case 'b': case 'n': case 'r': case 'f': case '\'': case '\"': case '\\':
					break;
				default: {
					JavaError::error(line, column, "Unrecognized escape sequence \\%c.", peek());
				} break;
			}
		}
		advance();
	}

	if (is_at_end()) {
		JavaError::error(line, column, "Unterminated string.");
		return;
	}

	advance(); // skip the closing "

	add_string_token();
}

void Lexer::scan_char_literal() {
	JavaValue value = {};

	// When ''
	if (peek() == '\'') {
		advance();
		add_token(TokenType::character, JavaType::_char, value);
		return;
	}

	if (peek() == '\\') {
		advance();
		char code = advance();

		#define case_code(a, b) case a: { value._char = b; } break;
		switch (code) {
			case_code('t', '\t')
			case_code('b', '\b')
			case_code('n', '\n')
			case_code('r', '\r')
			case_code('f', '\f')
			case_code('\'', '\'')
			case_code('\"', '\"')
			default: {
				JavaError::error(line, column, "Unrecognized escape sequence \\%c.", code);
			} break;
		}
		#undef case_code
	}
	else {
		value._char = advance();
	}

	if (advance() != '\'') {
		while (!is_at_end() && peek() != '\'') advance();
		if (!is_at_end() && peek() == '\'') advance();
		JavaError::error(line, column, "Character literal only supports a single character or escape sequence.");
		return;
	}
	add_token(TokenType::character, JavaType::_char, value);
}

inline void Lexer::scan_multiline_comment() {
	uint32_t nested_count = 1;

	for (;; advance()) {
		if (peek() == '\n') {
			column = 1;
			line++;
			continue;
		}
		if (peek() == '/' && peek_next() == '*') {
			nested_count++;
		}
		if (peek() == '*' && peek_next() == '/') {
			nested_count--;
		}
		if (nested_count == 0) {
			advance(); advance();
			break;
		}
	}
}

inline void Lexer::add_token(TokenType type) {
	char* lexeme = source.bytes + start;
	size_t lexeme_len = current - start;
	tokens.emplace_back(Token{ type, lexeme, lexeme_len, line, column - 1, JavaObject{JavaType::none, JavaValue{0}} });
}

inline void Lexer::add_token(TokenType type, JavaType jtype, JavaValue jvalue) {
	char* lexeme = source.bytes + start;
	size_t lexeme_len = current - start;
	tokens.emplace_back(Token{ type, lexeme, lexeme_len, line, column - 1, JavaObject{jtype, jvalue} });
}

inline void Lexer::add_string_token() {
	add_token(TokenType::string, JavaType::String, JavaValue{});
	Token &last = tokens.at(tokens.size() - 1);
	last.literal.value.String = last.lexeme;
}

inline char Lexer::advance() {
	char c = source.bytes[current];
	column++; current++;
	return c;
}

inline bool Lexer::match(char next) {
	if (is_at_end()) return false;
	if (source.bytes[current] != next) return false;
	column++; current++;
	return true;
}

inline char Lexer::peek() {
	assert(current >= 0 && current < source.len);
	return source.bytes[current];
}

inline char Lexer::peek_prev() {
	if (current - 1 < 0) return '\0';
	return source.bytes[current - 1];
}

inline char Lexer::peek_next() {
	if (current + 1 >= source.len) return '\0';
	return source.bytes[current + 1];
}

inline bool Lexer::is_at_end() {
	return current >= source.len;
}

inline bool Lexer::is_digit(char c) {
	return c >= '0' && c <= '9';
}

inline bool Lexer::is_alpha(char c) {
	return
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c == '_');

}

inline bool Lexer::is_alpha_numeric(char c) {
	return is_digit(c) || is_alpha(c);
}
