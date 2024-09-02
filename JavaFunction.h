#pragma once

#include "JavaCallable.h"
#include "JavaInstance.h"

struct JavaFunction : public JavaCallable {
	const JavaType return_type;
	const std::string declaration_name;
	const std::vector<std::pair<JavaTypeInfo, std::string>>* declaration_params;
	const std::vector<Stmt*>* declaration_body;
	Environment* closure;

	JavaFunction(const Stmt_Function* declaration, Environment* p_closure):
		return_type(declaration->return_type),
		declaration_name(declaration->name.lexeme),
		declaration_params(declaration->params),
		declaration_body(declaration->body),
		closure(p_closure)
	{}

	JavaFunction(JavaFunction* other, Environment* p_closure):
		return_type(other->return_type),
		declaration_name(other->declaration_name),
		declaration_params(other->declaration_params),
		declaration_body(other->declaration_body),
		closure(p_closure)
	{}

	JavaFunction(const Stmt_Function* declaration):
		return_type(declaration->return_type),
		declaration_name(declaration->name.lexeme),
		declaration_params(declaration->params),
		declaration_body(declaration->body),
		closure(nullptr)
	{}


	JavaFunction *bind(JavaInstance *instance);
	CallableType get_type() override;
	int arity() override;
	JavaObject call(Interpreter* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) override;
	std::string to_string() override;
};
