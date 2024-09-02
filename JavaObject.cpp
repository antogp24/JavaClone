#include <stdio.h>
#include "JavaObject.h"
#include "JavaCallable.h"
#include "JavaClass.h"
#include "JavaInstance.h"
#include "Token.h"
#include "Error.h"
#include <cassert>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

std::pair<JavaObject, bool> try_cast(const std::string& name, uint32_t line, uint32_t column, JavaType type, JavaObject obj) {
	JavaObject result = { type, JavaValue{} };

	if (type == obj.type) {
		return { obj, false };
	}
	else if (obj.type != JavaType::_null) {
		switch (type) {
			case JavaType::_byte: result.value._byte = java_cast_to_byte(obj); break;
			case JavaType::_char: result.value._char = java_cast_to_char(obj); break;
			case JavaType::_int: result.value._int = java_cast_to_int(obj); break;
			case JavaType::_long: result.value._long = java_cast_to_long(obj); break;
			case JavaType::_float: result.value._float = java_cast_to_float(obj); break;
			case JavaType::_double: result.value._double = java_cast_to_double(obj); break;
			case JavaType::UserDefined: {
				if (obj.type != JavaType::Instance) {
					throw JAVA_RUNTIME_ERR(name, line, column, "User defined types are incompatible with anything other than themselves.");
				}
				result.type = JavaType::Instance;
				result.value.instance = obj.value.instance;
			} break;
			default: throw JAVA_RUNTIME_ERR_VA(name, line, column, "Can't implicitly cast '%s' to '%s'.", java_type_cstring(obj.type), java_type_cstring(type));
		}
		return { result, false };
	}
	else {
		if (type != JavaType::String) {
			throw JAVA_RUNTIME_ERR(name, line, column, "Only objects can be null.");
		}
		return { result, true };
	}
}

bool is_java_type_number(JavaType type) {
	switch (type) {
		case JavaType::_byte:
		case JavaType::_char:
		case JavaType::_int:
		case JavaType::_long:
		case JavaType::_float:
		case JavaType::_double: return true;
		default: return false;
	}
}

bool is_java_type_primitive(JavaType type) {
	return is_java_type_number(type) || type == JavaType::_boolean;
}

bool is_token_type_java_type(TokenType type) {
	return token_type_to_java_type(type) != JavaType::none;
}

bool is_token_type_number(TokenType type) {
	return is_java_type_number(token_type_to_java_type(type));
}

bool is_token_type_modifier(TokenType type) {
	switch (type) {
		case TokenType::_private: return true;
		case TokenType::_protected: return true;
		case TokenType::_public: return true;
		default: return false;
	}
}

JavaType token_type_to_java_type(TokenType type) {
	switch (type) {
		case TokenType::_null: return JavaType::_null;
		case TokenType::type_void: return JavaType::_void;
		case TokenType::type_boolean: return JavaType::_boolean;
		case TokenType::type_byte: return JavaType::_byte;
		case TokenType::type_char: return JavaType::_char;
		case TokenType::type_int: return JavaType::_int;
		case TokenType::type_long: return JavaType::_long;
		case TokenType::type_float: return JavaType::_float;
		case TokenType::type_double: return JavaType::_double;
		case TokenType::type_String: return JavaType::String;
		case TokenType::type_user_defined: return JavaType::UserDefined;
		default: return JavaType::none;
	}
}

const char* java_type_cstring(JavaType type) {
	switch (type) {
		case JavaType::_null: return "null";
		case JavaType::_void: return "void";
		case JavaType::_boolean: return "boolean";
		case JavaType::_byte: return "byte";
		case JavaType::_char: return "char";
		case JavaType::_int: return "int";
		case JavaType::_long: return "long";
		case JavaType::_float: return "float";
		case JavaType::_double: return "double";
		case JavaType::String: return "String";
		case JavaType::Function: return "Function";
		case JavaType::Instance: return "Instance";
		case JavaType::Class: return "Class";
		case JavaType::UserDefined: return "UserDefined";
		default: return "None";
	}
}

JavaType java_get_smaller_type(const JavaObject& lhs, const JavaObject& rhs) {
	if ((uint8_t)lhs.type < (uint8_t)rhs.type) {
		return lhs.type;
	}
	return rhs.type;
}

JavaType java_get_bigger_type(const JavaObject& lhs, const JavaObject& rhs) {
	if ((uint8_t)lhs.type > (uint8_t)rhs.type) {
		return lhs.type;
	}
	return rhs.type;
}

#define fn_java_cast(T)                                                      \
Java##T java_cast_to##T(const JavaObject& object) {                          \
	switch (object.type) {                                                   \
		case JavaType::_boolean: return (Java##T)object.value._boolean;      \
		case JavaType::_byte: return (Java##T)object.value._byte;            \
		case JavaType::_char: return (Java##T)object.value._char;            \
		case JavaType::_int: return (Java##T)object.value._int;              \
		case JavaType::_long: return (Java##T)object.value._long;            \
		case JavaType::_float: return (Java##T)object.value._float;          \
		case JavaType::_double: return (Java##T)object.value._double;        \
	}                                                                        \
	return 0;                                                                \
}
fn_java_cast(_boolean)
fn_java_cast(_byte)
fn_java_cast(_char)
fn_java_cast(_int)
fn_java_cast(_long)
fn_java_cast(_float)
fn_java_cast(_double)
#undef fn_java_cast

void java_object_print(const JavaObject& object) {
	switch (object.type) {
		case JavaType::_null: {
			printf("null");
		} break;

		case JavaType::_boolean: {
			printf(object.value._boolean ? "true" : "false");
		} break;

		case JavaType::_byte: {
			printf("0x%x", object.value._byte);
		} break;

		case JavaType::_char: {
			printf("\'%c\'", object.value._char);
		} break;

		case JavaType::_int: {
			printf("%ld", object.value._int);
		} break;

		case JavaType::_long: {
			printf("%lld", object.value._long);
		} break;

		case JavaType::_float: {
			printf("%ff", object.value._float);
		} break;

		case JavaType::_double: {
			printf("%lf", object.value._double);
		} break;

		case JavaType::String: {
			if (object.is_null) printf("null");
			else if (object.value.String != nullptr) printf("%s", object.value.String);
		} break;

		case JavaType::Function: {
			JavaCallable* callable = (JavaCallable*)object.value.function;
			printf("%s", callable->to_string().c_str());
		} break;

		case JavaType::Class: {
			JavaClass* class_info = (JavaClass*)object.value.class_info;
			printf("%s", class_info->to_string().c_str());
		} break;

		case JavaType::Instance: {
			JavaInstance* instance = (JavaInstance*)object.value.instance;
			printf("%s@%p", instance->class_info->name.c_str(), instance);
		} break;

		default: assert(false && "Unimplemented printing this java type.");
	}
}
