#include "JavaObject.h"
#include "stdio.h"

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
#endif


bool is_java_type_number(JavaType type) {
	switch (type) {
		case JavaType::_byte:
		case JavaType::_char:
		case JavaType::_int:
		case JavaType::_long:
		case JavaType::_float:
		case JavaType::_double: {
			return true;
		}
		default: {
			return false;
		}
	}
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
		case JavaType::_byte: return (Java##T)object.value._byte;            \
		case JavaType::_int: return (Java##T)object.value._int;              \
		case JavaType::_long: return (Java##T)object.value._long;            \
		case JavaType::_float: return (Java##T)object.value._float;          \
		case JavaType::_double: return (Java##T)object.value._double;        \
	}                                                                        \
	return 0;                                                                \
}
fn_java_cast(_byte)
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
			if (object.value.String == NULL) return;
			printf("%s", object.value.String);
		} break;
	}
}
