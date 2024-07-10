#include "JavaClass.h"
#include "JavaInstance.h"

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

int JavaClass::arity() {
	return 0;
}

JavaObject JavaClass::call(Interpreter* intepreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> arguments) {
	JavaInstance* instance = DBG_new JavaInstance{ this }; // freed at interpreter destructor.
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