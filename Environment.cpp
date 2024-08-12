#include "Environment.h"
#include "Interpreter.h"
#include "JavaNativeFunction.h"
#include "Error.h"
#include <assert.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

Environment::Environment() {
	values = JavaScope();
}

Environment::Environment(Environment* p_enclosing): enclosing(p_enclosing) {
	values = JavaScope();
}

void Environment::define(Stmt_Var* stmt, const Token& name, Expr* initializer, JavaType expected_type, JavaObject value) {
	assert(stmt != nullptr);
	JavaVariable variable = { value, stmt->visibility, stmt->is_static, stmt->is_final, initializer == nullptr };
	define(name, expected_type, variable);
}

void Environment::define(Token name, JavaType expected_type, JavaVariable variable) {
	if (variable.object.type == JavaType::_void) {
		throw JAVA_RUNTIME_ERROR_VA(name, "Can't define '%s' as void.", name.lexeme);
	}
	if (scope_has(name)) {
		throw JAVA_RUNTIME_ERROR_VA(name, "Variable '%s' is already defined in this scope.", name.lexeme);
	}
	JavaVariable defaultvar = variable;
	defaultvar.object.type = expected_type;
	scope_set(name, defaultvar);
	assign(name, variable.object, true);
}

void Environment::define_native_function(
		const std::string& name,
		std::function<int()> arity_fn,
		std::function<JavaObject(void*, uint32_t, uint32_t, std::vector<ArgumentInfo>)> call_fn,
		std::function<std::string()> to_string_fn)
{
	values[name] = JavaVariable{
		JavaObject{
			JavaType::Function,
			JavaValue{
				.function = DBG_new JavaNativeFunction { arity_fn, call_fn, to_string_fn }
			},
		},
		Visibility::Public,
		true,
		true,
		false,
	};
}

void Environment::assign(Token name, JavaObject value) {
	assign(name.lexeme, name.line, name.column, value, false);
}

void Environment::assign(Token name, JavaObject value, bool force) {
	assign(name.lexeme, name.line, name.column, value, force);
}

void Environment::assign(const std::string& name, uint32_t line, uint32_t column, JavaObject value) {
	assign(name, line, column, value, false);
}

void Environment::assign(const std::string& name, uint32_t line, uint32_t column, JavaObject value, bool force) {
	if (value.type == JavaType::_void) {
		throw JAVA_RUNTIME_ERR_VA(name, line, column, "Can't assign void to '%s'.", name.c_str());
	}
	if (values.contains(name)) {
		JavaVariable variable = values.at(name);
		variable.is_uninitialized = false;

		if (variable.is_final && !force) {
			throw JAVA_RUNTIME_ERR_VA(name, line, column, "Variable '%s' is final.", name.c_str());
		}

		auto casted = try_cast(name, line, column, variable.object.type, value);
		variable.object = casted.first;
		variable.object.is_null = casted.second;
		values[name] = variable;
		return;
	}

	if (enclosing != nullptr) {
		enclosing->assign(name, line, column, value);
		return;
	}

	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Undefined variable '%s'.", name.c_str());
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

void* Environment::get_function_ptr(const std::string& name) {
	JavaObject object = values.at(name).object;
	assert(object.type == JavaType::Function);
	return object.value.function;
}

JavaObject Environment::get(const Token &name) {
	return get(name.lexeme, name.line, name.column);
}

JavaObject Environment::get(const std::string& name, uint32_t line, uint32_t column) {
	if (values.contains(name)) {
		JavaVariable var = values.at(name);
		if (var.is_uninitialized) {
			throw JAVA_RUNTIME_ERR(name, line, column, "Variable is uninitialized.");
		}
		return var.object;
	}

	if (enclosing != nullptr) return enclosing->get(name, line, column);

	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Undefined variable %s.", name.c_str());
}

