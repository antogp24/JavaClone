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

JavaObject JavaFunction::call(Interpreter* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) {
	Environment* previous = interpreter->environment;
	Environment* environment = DBG_new Environment(interpreter->globals);

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

		#define case_cast(T)                     \
			case JavaType::T: {                  \
				r.value.T = java_cast_to##T(r);  \
				r.type = return_type;            \
			} break;

		if (return_type != r.type && is_java_type_number(return_type) && is_java_type_number(r.type)) {
			switch (return_type) {
				case_cast(_byte)
				case_cast(_char)
				case_cast(_int)
				case_cast(_long)
				case_cast(_float)
				case_cast(_double)
			}
		}
		else if (return_type != r.type) {
			throw JAVA_RUNTIME_ERR_VA(declaration_name, line, column, "Return type '%s' doesn't match with '%s'.", java_type_cstring(return_type), java_type_cstring(r.type));
		}
		#undef case_cast
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
