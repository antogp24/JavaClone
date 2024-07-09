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

void Environment::define(Stmt_Var* stmt, const Token& name, Expr* initializer, JavaObject value) {
	assert(stmt != nullptr);
	JavaVariable variable = { value, stmt->visibility, stmt->is_static, stmt->is_final, initializer == nullptr };
	define(name, variable);
}

void Environment::define(Token name, JavaVariable variable) {
	if (variable.value.type == JavaType::_void) {
		throw JAVA_RUNTIME_ERROR_VA(name, "Can't define '%s' as void.", name.lexeme);
	}
	if (scope_has(name)) {
		throw JAVA_RUNTIME_ERROR_VA(name, "Variable '%s' is already defined in this scope.", name.lexeme);
	}
	scope_set(name, variable);
}

void Environment::define_native_function(
		const std::string& name,
		Native_Arity arity_fn,
		Native_Call call_fn,
		Native_ToString to_string_fn)
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
	assign(name.lexeme, name.line, name.column, value);
}

void Environment::assign(const std::string& name, uint32_t line, uint32_t column, JavaObject value) {
	if (value.type == JavaType::_void) {
		throw JAVA_RUNTIME_ERR_VA(name, line, column, "Can't assign void to '%s'.", name.c_str());
	}
	if (values.contains(name)) {
		JavaVariable variable = values.at(name);
		variable.is_uninitialized = false;

		if (variable.is_final) {
			throw JAVA_RUNTIME_ERR_VA(name, line, column, "Variable '%s' is final.", name.c_str());
		}

		if (variable.value.type == value.type) {
			variable.value = value;
		}
		else if (value.type != JavaType::_null) {
			switch (variable.value.type) {
				case JavaType::_byte: variable.value.value._byte = java_cast_to_byte(value); break;
				case JavaType::_char: variable.value.value._char = java_cast_to_char(value); break;
				case JavaType::_int: variable.value.value._int = java_cast_to_int(value); break;
				case JavaType::_long: variable.value.value._long = java_cast_to_long(value); break;
				case JavaType::_float: variable.value.value._float = java_cast_to_float(value); break;
				case JavaType::_double: variable.value.value._double = java_cast_to_double(value); break;
				default: throw JAVA_RUNTIME_ERR_VA(name, line, column, "Can't implicitly cast '%s' to '%s'.", java_type_cstring(value.type), java_type_cstring(variable.value.type));
			}
		}
		else {
			if (variable.value.type != JavaType::String) {
				throw JAVA_RUNTIME_ERR(name, line, column, "Only objects can be null.");
			}
			variable.value.is_null = true;
		}
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
	JavaObject object = values.at(name).value;
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
		return var.value;
	}

	if (enclosing != nullptr) return enclosing->get(name, line, column);

	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Undefined variable %s.", name.c_str());
}

