#pragma once

#include "JavaCallable.h"

struct JavaFunction : public JavaCallable {
	const JavaType return_type;
	const std::string declaration_name;
	const std::vector<std::pair<JavaTypeInfo, std::string>>* declaration_params;
	const std::vector<Stmt*>* declaration_body;

	JavaFunction(const Stmt_Function* declaration):
		return_type(declaration->return_type),
		declaration_name(declaration->name.lexeme),
		declaration_params(declaration->params),
		declaration_body(declaration->body)
	{}

	CallableType get_type() override;
	int arity() override;
	JavaObject call(Interpreter* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) override;
	std::string to_string() override;
};
