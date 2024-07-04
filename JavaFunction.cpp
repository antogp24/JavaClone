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

CallableType JavaFunction::get_type() {
	return CallableType::UserDefined;
}

int JavaFunction::arity() {
	return (int)declaration_params->size();
}

JavaObject JavaFunction::call(Interpreter* interpreter, std::vector<ArgumentInfo> arguments) {
	Environment* previous = interpreter->environment;
	Environment* environment = DBG_new Environment(interpreter->globals);

	for (int i = 0; i < arity(); i++) {
		std::string parameter_name = declaration_params->at(i).second;
		JavaVariable argument = {arguments.at(i).object, Visibility::Local, false, false, false};

		const ArgumentInfo &arg = arguments.at(i);
		const JavaTypeInfo &decl = declaration_params->at(i).first;

		environment->values[parameter_name] = argument;
	}

	try {
		interpreter->execute_block(*declaration_body, environment);
	}
	catch (Interpreter::Return rtrn) {
		delete environment;
		interpreter->environment = previous;
		return rtrn.value;
	}

	return {JavaType::none};
}

std::string JavaFunction::to_string() {
	std::string result("<fn ");
	result.append(declaration_name);
	result.append(">");
	return result;
}
