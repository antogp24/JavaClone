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

JavaFunction *JavaFunction::bind(JavaInstance *instance) {
	Environment* env = DBG_new Environment(closure);
	env->define("this", 0, 0, JavaType::Instance, JavaVariable{
		.object = {
			JavaType::Instance,
			JavaValue{ .instance = instance },
		},
		.visibility = Visibility::Public,
		.is_static = false,
		.is_final = false,
		.is_uninitialized = false,
	});
	return DBG_new JavaFunction(this, env);
}

int JavaFunction::arity() {
	return (int)declaration_params->size();
}

JavaObject JavaFunction::call(Interpreter* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) {
	Environment* previous = interpreter->environment;
	Environment* environment = DBG_new Environment(this->closure);

	for (int i = 0; i < arity(); i++) {
		const JavaTypeInfo &decl = declaration_params->at(i).first;
		const std::string &parameter_name = declaration_params->at(i).second;

		const ArgumentInfo &arg = arguments.at(i);
		JavaVariable argument = { arg.object, Visibility::Local, false, false, false };

		environment->values[parameter_name] = argument;
	}

	try {
		interpreter->execute_block(*declaration_body, environment);
	}
	catch (Interpreter::Return retrn) {
		delete environment;
		interpreter->environment = previous;

		JavaObject r = retrn.value;
		if (r.type == JavaType::_void) return r;
		auto casted = try_cast(to_string(), line, column, return_type, r);
		r = casted.first;
		r.is_null = casted.second;

		return r;
	}
	if (return_type == JavaType::_void) return { JavaType::_void, JavaValue{} };

	return { JavaType::none, JavaValue{} };
}

std::string JavaFunction::to_string() {
	std::string result("<fn ");
	result.append(declaration_name).append(">");
	return result;
}
