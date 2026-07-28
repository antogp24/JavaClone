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

			if (fields.contains(name.lexeme)) {
				throw JAVA_RUNTIME_ERROR_VA(name, "In class '%s' the field '%s' is already defined.", this->class_info->name.c_str(), name.lexeme);
			}
			fields.insert({ name.lexeme, variable });
		}
	}
	for (const Stmt_Function* methoddecl : class_info->methods) {
		if (methoddecl->is_static) continue;

		Environment* env = DBG_new Environment(interpreter->globals);
		env->define("this", 0, 0, JavaType::Instance, JavaVariable{
			.object = {
				JavaType::Instance,
				JavaValue{ .instance = this },
			},
			.visibility = Visibility::Public,
			.is_static = false,
			.is_final = false,
			.is_uninitialized = false,
		});
		void* fn = DBG_new JavaFunction(methoddecl, env);
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
		if (fields.contains(methoddecl->name.lexeme)) {
			throw JAVA_RUNTIME_ERROR_VA(methoddecl->name, "In class '%s' the method '%s' is already defined.", this->class_info->name.c_str(), methoddecl->name.lexeme);
		}
		fields.insert({std::string(methoddecl->name.lexeme), variable});
	}
}

JavaObject JavaInstance::get(Expr_Get* expr) {
	return get(expr->name, expr->line, expr->column);
}

static bool is_private_out_of_class(JavaInstance* instance, Visibility visibility) {
	bool has_this = false;
	Environment* e;
	for (e = instance->interpreter->environment; true; e = e->enclosing) {
		if (e->values.contains("this")) {
			has_this = true;
			break;
		}
		if (e->enclosing == NULL) break;
	}

	if (!has_this) return visibility == Visibility::Private;
	if (e->values["this"].object.type != JavaType::Instance) return visibility == Visibility::Private;

	JavaInstance* this_instance = (JavaInstance*)e->values["this"].object.value.instance;
	return visibility == Visibility::Private && this_instance->class_info->name != instance->class_info->name;
}

JavaObject JavaInstance::get(std::string name, uint32_t line, uint32_t column) {
	if (fields.contains(name)) {
		if (is_private_out_of_class(this, fields[name].visibility)) {
			throw JAVA_RUNTIME_ERR_VA(name, line, column, "In class '%s' the field '%s' is private.", class_info->name.c_str(), name.c_str());
		}
		return fields[name].object;
	}
	if (class_info->static_fields.contains(name)) {
		return class_info->get(name, line, column);
	}
	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Class '%s' doesn't have field '%s'.", class_info->name.c_str(), name.c_str());
}

void JavaInstance::set(Expr_Set* expr, JavaObject value) {
	set(expr->rhs_name, expr->line, expr->column, value);
}

void JavaInstance::set(std::string name, uint32_t line, uint32_t column, JavaObject value) {
	if (fields.contains(name)) {
		fields[name].object = value;
		return;
	}
	if (class_info->static_fields.contains(name)) {
		class_info->set(name, line, column, value);
		return;
	}
	throw JAVA_RUNTIME_ERR_VA(name, line, column, "Class '%s' doesn't have field '%s'.", class_info->name.c_str(), name.c_str());
}
