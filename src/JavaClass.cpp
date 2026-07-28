#include "JavaClass.h"
#include "JavaInstance.h"
#include "JavaFunction.h"
#include "Error.h"
#include <assert.h>
#include <string.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

JavaClass::JavaClass(Interpreter *p_interpreter, std::string p_name, uint32_t p_line, uint32_t p_column, bool p_is_abstract, std::vector<Stmt_Var*> p_attributes, std::vector<Stmt_Function*> p_methods):
	interpreter(p_interpreter), name(p_name), line(p_line), column(p_column), is_abstract(p_is_abstract), attributes(p_attributes), methods(p_methods)
{
	for (Stmt_Function* methoddecl : methods) {
		if (strcmp(methoddecl->name.lexeme, "__init__") == 0) {
			this->constructor = methoddecl;
			continue;
		}
		if (!methoddecl->is_static) continue;

		void* fn = DBG_new JavaFunction(methoddecl, interpreter->globals);
		JavaVariable variable = {
			.object = {
				JavaType::Function,
				JavaValue{ .function = fn },
			},
			.visibility = methoddecl->visibility,
			.is_static = true,
			.is_final = true,
			.is_uninitialized = false,
		};
		if (static_fields.contains(methoddecl->name.lexeme)) {
			throw JAVA_RUNTIME_ERROR_VA(methoddecl->name, "In class '%s' the method '%s' is already defined.", this->name.c_str(), methoddecl->name.lexeme);
		}
		static_fields.insert({std::string(methoddecl->name.lexeme), variable});
	}
	for (Stmt_Var* vardecl : attributes) {
		if (!vardecl->is_static) continue;
		assert(vardecl->names.size() == vardecl->initializers.size());

		for (int i = 0; i < vardecl->names.size(); i++) {
			Expr* initializer = vardecl->initializers.at(i);
			const Token& name = vardecl->names.at(i);
			JavaType type = token_type_to_java_type(vardecl->type.type);
			JavaObject value = interpreter->validate_variable(vardecl, type, name, initializer);
			JavaVariable variable = {
				.object = value,
				.visibility = vardecl->visibility,
				.is_static = true,
				.is_final = vardecl->is_final,
				.is_uninitialized = false,
			};
			auto casted = try_cast(std::string(name.lexeme), name.line, name.column, type, value);
			variable.object = casted.first;
			variable.object.is_null = casted.second;

			if (static_fields.contains(name.lexeme)) {
				throw JAVA_RUNTIME_ERROR_VA(name, "In class '%s' the field '%s' is already defined.", this->name.c_str(), name.lexeme);
			}
			static_fields.insert({ name.lexeme, variable });
		}
	}
}

int JavaClass::arity() {
	if (constructor == nullptr) return 0;
	return (int)constructor->params->size();
}

JavaObject JavaClass::call(Interpreter* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) {
	if (this->is_abstract) {
		throw JAVA_RUNTIME_ERR(this->name, line, column, "Abstract class can't be instantiated.");
	}
	if (arguments.size() != arity()) {
		throw JAVA_RUNTIME_ERR_VA(this->name, line, column, "Expected %i arguments in the constructor, but recieved %i.", arity(), arguments.size());
	}
	JavaInstance* instance = DBG_new JavaInstance{ interpreter, this }; // freed at interpreter destructor.
	if (constructor != nullptr) {
		JavaObject member = instance->get("__init__", line, column);
		assert(member.type == JavaType::Function);
		JavaFunction* fn = (JavaFunction*)member.value.function;
		fn->call(interpreter, line, column, arguments);
	}
	interpreter->instances.push_back(instance);
	return { JavaType::Instance, {.instance = instance} };
}

std::string JavaClass::to_string() {
	std::string result("<class ");
	result.append(name).append(">");
	return result;
}

CallableType JavaClass::get_type() {
	return CallableType::Constructor;
}

JavaObject JavaClass::get(Expr_Get* expr) {
	return get(expr->name, expr->line, expr->column);
}

JavaObject JavaClass::get(std::string name, uint32_t line, uint32_t column) {
	if (static_fields.contains(name)) {
		return static_fields.at(name).object;
	}
	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Class '%s' doesn't have static field '%s'.", this->name.c_str(), name.c_str());
}

void JavaClass::set(Expr_Set* expr, JavaObject value) {
	set(expr->rhs_name, expr->line, expr->column, value);
}

void JavaClass::set(std::string name, uint32_t line, uint32_t column, JavaObject value) {
	if (static_fields.contains(name)) {
		static_fields[name].object = value;
	}
	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Class '%s' doesn't have static field '%s'.", this->name.c_str(), name.c_str());
}