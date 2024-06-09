#pragma once

#include "Token.h"

enum class Visibility {
	None,
	Private,
	Protected,
	Package,
	Public,
};

Visibility visibility_from_token_type(TokenType type);
bool is_token_type_visibility(TokenType type);
const char* visibility_to_cstring(Visibility v);
