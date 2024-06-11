#include "Visibility.h"
#include "Token.h"

Visibility visibility_from_token_type(TokenType type) {
	switch (type) {
		case TokenType::_private: return Visibility::Private;
		case TokenType::_protected: return Visibility::Protected;
		case TokenType::_public: return Visibility::Public;
	}
	return Visibility::None;
}

bool is_token_type_visibility(TokenType type) {
	return type == TokenType::_public    ||
		   type == TokenType::_private   ||
		   type == TokenType::_protected;
}

const char* visibility_to_cstring(Visibility v) {
	switch (v) {
		case Visibility::Local: return "local";
		case Visibility::Private: return "private";
		case Visibility::Protected: return "protected";
		case Visibility::Public: return "public";
		case Visibility::Package: return "package";
	}
	return "invalid";
}
