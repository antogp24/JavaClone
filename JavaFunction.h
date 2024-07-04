#pragma once

#include "JavaCallable.h"

struct JavaFunction : public JavaCallable {
	const std::string declaration_name;
	const std::vector<std::pair<JavaTypeInfo, std::string>>* declaration_params;
	const std::vector<Stmt*>* declaration_body;

	JavaFunction(const Stmt_Function* declaration):
		declaration_name(declaration->name.lexeme),
		declaration_params(declaration->params),
		declaration_body(declaration->body)
	{}

	CallableType get_type() override;
	int arity() override;
	JavaObject call(Interpreter* interpreter, std::vector<ArgumentInfo> arguments) override;
	std::string to_string() override;
};
