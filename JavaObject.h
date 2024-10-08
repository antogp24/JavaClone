#pragma once

#include "TokenType.h"
#include <utility>
#include <string>
#include <set>

enum class JavaType : uint8_t {
	none = 0,
	_void,
	_boolean,
	_byte,
	_char,
	_int,
	_long,
	_float,
	_double,
	_null,
	UserDefined,
	String,
	Function,
	Instance,
	Class,
	count,
};

typedef bool Java_boolean;
typedef int8_t Java_byte;
typedef int16_t Java_char;
typedef int32_t Java_int;
typedef int64_t Java_long;
typedef float Java_float;
typedef double Java_double;
typedef char* Java_String;

union JavaValue {
	Java_boolean _boolean;
	Java_byte _byte;
	Java_char _char;
	Java_int _int;
	Java_long _long;
	Java_float _float;
	Java_double _double;
	Java_String String;
	void* function;
	void* instance;
	void* class_info;
};

struct JavaObject {
	JavaType type;
	JavaValue value;
	int32_t is_null = false;
};

std::pair<JavaObject, bool> try_cast(const std::string& name, uint32_t line, uint32_t column, JavaType type, JavaObject value);
JavaType token_type_to_java_type(TokenType type);
const char* java_type_cstring(JavaType type);
bool is_java_type_number(JavaType type);
bool is_java_type_primitive(JavaType type);
bool is_token_type_number(TokenType type);
bool is_token_type_modifier(TokenType type);
bool is_token_type_java_type(TokenType type);
void java_object_print(const JavaObject& object);
JavaType java_get_smaller_type(const JavaObject& lhs, const JavaObject& rhs);
JavaType java_get_bigger_type(const JavaObject& lhs, const JavaObject &rhs);
#define fn_java_cast(T) Java##T java_cast_to##T(const JavaObject& object);
fn_java_cast(_boolean)
fn_java_cast(_byte)
fn_java_cast(_char)
fn_java_cast(_int)
fn_java_cast(_long)
fn_java_cast(_float)
fn_java_cast(_double)
#undef fn_java_cast

