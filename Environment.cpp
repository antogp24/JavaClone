#include "Environment.h"
#include "Error.h"
#include <assert.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

void Environment::define(Stmt_Var* stmt, JavaObject value) {
	assert(stmt != nullptr);
	JavaVariable variable = { value, stmt->visibility, stmt->is_static, stmt->is_final };
	define(stmt->name, variable);
}

void Environment::define(Token name, JavaVariable variable) {
	if (scope_has(name)) {
		throw JAVA_RUNTIME_ERROR(name, "Variable '%s' is already defined in this scope.", name.lexeme);
	}
	scope_set(name, variable);
}

void Environment::assign(Token name, JavaObject value) {
	if (scope_has(name)) {
		JavaVariable variable = scope_get(name);
		if (variable.is_final) {
			throw JAVA_RUNTIME_ERROR(name, "Variable '%s' is final.", name.lexeme);
		}
		variable.value = value;
		scope_set(name, variable);
		return;
	}

	if (enclosing != nullptr) {
		enclosing->assign(name, value);
		return;
	}

	throw JAVA_RUNTIME_ERROR(name, "Undefined variable '%s'.", name.lexeme);
}

bool Environment::scope_has(const Token &name) {
	std::string copied(name.lexeme);
	return values.contains(copied);
}

JavaVariable Environment::scope_get(const Token &name) {
	std::string copied(name.lexeme);
	return values.at(copied);
}

void Environment::scope_set(const Token& name, JavaVariable variable) {
	std::string copied(name.lexeme);
	values[copied] = variable;
}

JavaObject Environment::get(const Token &name) {
	if (scope_has(name)) {
		return scope_get(name).value;
	}

	if (enclosing != nullptr) return enclosing->get(name);

	throw JAVA_RUNTIME_ERROR(name, "Undefined variable %s.", name.lexeme);
}
