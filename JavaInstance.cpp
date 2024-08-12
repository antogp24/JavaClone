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

JavaInstance::JavaInstance(Interpreter* p_interpreter, JavaClass* p_class_info):
	interpreter(p_interpreter),
	class_info(p_class_info)
{
	for (const Stmt_Var* vardecl : class_info->attributes) {
		if (vardecl->is_static) continue;
		assert(vardecl->names.size() == vardecl->initializers.size());
		for (int i = 0; i < vardecl->names.size(); i++) {
			Expr* initializer = vardecl->initializers.at(i);
			const Token& name = vardecl->names.at(i);
			JavaType type = token_type_to_java_type(vardecl->type.type);
			JavaObject value = interpreter->validate_variable(vardecl, type, name, initializer);
			JavaVariable variable = {
				.object = value,
				.visibility = vardecl->visibility,
				.is_static = false,
				.is_final = vardecl->is_final,
				.is_uninitialized = false,
			};
			auto casted = try_cast(std::string(name.lexeme), name.line, name.column, type, value);
			variable.object = casted.first;
			variable.object.is_null = casted.second;
			fields.insert({ name.lexeme, variable });
		}
	}
	for (const Stmt_Function* methoddecl : class_info->methods) {
		if (methoddecl->is_static) continue;
		void* fn = DBG_new JavaFunction(methoddecl);
		JavaVariable variable = {
			.object = {
				JavaType::Function,
				JavaValue{ .function = fn },
			},
			.visibility = methoddecl->visibility,
			.is_static = false,
			.is_final = true,
			.is_uninitialized = false,
		};
		fields.insert({std::string(methoddecl->name.lexeme), variable});
	}
}

JavaObject JavaInstance::get(Expr_Get* expr) {
	return get(expr->name, expr->line, expr->column);
}

JavaObject JavaInstance::get(std::string name, uint32_t line, uint32_t column) {
	if (fields.contains(name)) {
		return fields.at(name).object;
	}
	if (class_info->static_fields.contains(name)) {
		return class_info->get(name, line, column);
	}
	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Undefined field %s.", name);
}

void JavaInstance::set(Expr_Set* expr, JavaObject value) {
	set(expr->rhs_name, expr->line, expr->column, value);
}

void JavaInstance::set(std::string name, uint32_t line, uint32_t column, JavaObject value) {
	if (fields.contains(name)) {
		fields[name].object = value;
	}
	if (class_info->static_fields.contains(name)) {
		class_info->set(name, line, column, value);
	}
	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Class '%s' doesn't have field '%s'.", class_info->name.c_str(), name);
}
