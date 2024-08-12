#include "JavaClass.h"
#include "JavaInstance.h"
#include "JavaFunction.h"
#include "Error.h"
#include <assert.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

JavaClass::JavaClass(Interpreter *p_interpreter, std::string p_name, uint32_t p_line, uint32_t p_column, std::vector<Stmt_Var*> p_attributes, std::vector<Stmt_Function*> p_methods):
	interpreter(p_interpreter), name(p_name), line(p_line), column(p_column), attributes(p_attributes), methods(p_methods)
{
	for (const Stmt_Var* vardecl : attributes) {
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
			static_fields.insert({ name.lexeme, variable });
		}
	}
	for (const Stmt_Function* methoddecl : methods) {
		if (!methoddecl->is_static) continue;
		void* fn = DBG_new JavaFunction(methoddecl);
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
		static_fields.insert({std::string(methoddecl->name.lexeme), variable});
	}
}

int JavaClass::arity() {
	return 0;
}

JavaObject JavaClass::call(Interpreter* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) {
	JavaInstance* instance = DBG_new JavaInstance{ interpreter, this }; // freed at interpreter destructor.
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
	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Undefined static field %s.", name);
}

void JavaClass::set(Expr_Set* expr, JavaObject value) {
	set(expr->rhs_name, expr->line, expr->column, value);
}

void JavaClass::set(std::string name, uint32_t line, uint32_t column, JavaObject value) {
	if (static_fields.contains(name)) {
		static_fields[name].object = value;
	}
	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Class '%s' doesn't have static field '%s'.", name.c_str(), name);
}