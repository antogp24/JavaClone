#include "Interpreter.h"
#include "JavaCallable.h"
#include "JavaNativeFunction.h"
#include "JavaFunction.h"
#include "JavaClass.h"
#include "JavaInstance.h"
#include "AstPrinter.h"
#include "Error.h"

#include <chrono>
#include <math.h>
#include <assert.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

extern bool REPL;

Interpreter::Interpreter() {
	globals = DBG_new Environment();

	strings_arena = arena_make();
	instances = {};

	auto cast_to_double = [](JavaObject object, const std::string &name, uint32_t line, uint32_t column) {
		switch (object.type) {
			case JavaType::_byte: case JavaType::_char: case JavaType::_int: case JavaType::_long:
			case JavaType::_float:  return java_cast_to_double(object);
			case JavaType::_double: return object.value._double; break;
			default: throw JAVA_RUNTIME_ERR(name, line, column, "Expected a number as an argument.");
		}
	};

	globals->define_native_function("clock",
		[]() { return 0; },
		[](void* interpreter, uint32_t, uint32_t, std::vector<ArgumentInfo>) {
			using namespace std::chrono;
			Java_long result = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
			return JavaObject{ JavaType::_long, JavaValue{ ._long = result } };
		},
		[]() { return "<native_fn clock>"; });

	globals->define_native_function("sqrt",
		[]() { return 1; },
		[cast_to_double](void* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> args) {
			Java_double input = cast_to_double(args[0].object, "sqrt", line, column);
			return JavaObject{ JavaType::_double, JavaValue{ ._double = sqrt(input) }};
		},
		[]() { return "<native_fn sqrt>"; });

	globals->define_native_function("pow",
		[]() { return 2; },
		[cast_to_double](void* interpreter, uint32_t line, uint32_t column, std::vector<ArgumentInfo> args) {
			Java_double number = cast_to_double(args[0].object, "pow", line, column);
			Java_double power = cast_to_double(args[1].object, "pow", line, column);
			return JavaObject{ JavaType::_double, JavaValue{ ._double = pow(number, power) }};
		},
		[]() { return "<native_fn pow>"; });

	environment = globals;
}

Interpreter::~Interpreter() {
	arena_free(&strings_arena);

	for (auto const& [_, variable] : globals->values) {
		if (variable.object.type == JavaType::Function) {
			JavaCallable* function = (JavaCallable*)variable.object.value.function;

			switch (function->get_type()) {
				case CallableType::UserDefined: {
					JavaFunction* userfn = dynamic_cast<JavaFunction*>(function);
					const auto& body = userfn->declaration_body;

					for (int i = 0; i < body->size(); i++) {
						stmt_free(body->at(i));
					}

					delete userfn->declaration_body;
					delete userfn->declaration_params;
					delete userfn;
				} break;

				case CallableType::Builtin: {
					JavaNativeFunction* nativefn = (JavaNativeFunction*)function;
					delete nativefn;
				} break;
			}
		}
		else if (variable.object.type == JavaType::Class) {
			JavaClass* classinfo = (JavaClass*)variable.object.value.class_info;
			for (auto const& [_, variable] : classinfo->static_fields) {
				if (variable.object.type == JavaType::Function) {
					JavaCallable* callable = (JavaCallable*)variable.object.value.function;
					assert(callable->get_type() == CallableType::UserDefined);
					JavaFunction* userfn = dynamic_cast<JavaFunction*>(callable);
					delete userfn;
				}
			}
			for (Stmt_Function* method : classinfo->methods) {
				delete method->params;
				for (int i = 0; i < method->body->size(); i++) {
					stmt_free(method->body->at(i));
				}
				delete method->body;
				delete method;
			}
			delete classinfo;
		}
	}

	for (void* it : instances) {
		JavaInstance* instance = (JavaInstance*)it;
		for (auto const& [_, variable] : instance->fields) {
			if (variable.object.type == JavaType::Function) {
				JavaCallable* callable = (JavaCallable*)variable.object.value.function;
				assert(callable->get_type() == CallableType::UserDefined);
				JavaFunction* userfn = dynamic_cast<JavaFunction*>(callable);
				delete userfn;
			}
		}
		delete instance;
	}
	
	delete globals;
}

void Interpreter::interpret(std::vector<Stmt*>* statements) {
	try {
		for (int i = 0; i < statements->size(); i++) {
			execute_statement(statements->at(i));
		}
	}
	catch (JavaRuntimeError error) {
		JavaError::runtime_error(error);
	}
}

void Interpreter::execute_block(const std::vector<Stmt*>& statements, Environment* env) {
	Environment* previous = this->environment;
	this->environment = env;

	for (Stmt* statement : statements) {
		if (this->broke || this->continued) break;
		execute_statement(statement);
	}

	delete env;
	this->environment = previous;
}

JavaObject Interpreter::validate_variable(const Stmt_Var* stmt, const JavaType type, const Token& name, const Expr* initializer) {
	JavaObject value = { JavaType::none, JavaValue{} };

	if (type == JavaType::none) {
		throw JAVA_RUNTIME_ERROR_VA(stmt->type, "Token '%s' is an invalid type.", stmt->type.lexeme);
	}

	if (initializer != nullptr) {
		value = evaluate((Expr*)initializer);

		if (value.type == JavaType::_void) {
			throw JAVA_RUNTIME_ERROR(name, "Void isn't a valid value, as it is a zero-byte type.");
		}

		if (is_java_type_primitive(type) && value.type == JavaType::_null) {
			throw JAVA_RUNTIME_ERROR(stmt->type, "Primitives can't be null.");
		}

		if (is_java_type_number(type) && !is_java_type_number(value.type) ||
		   !is_java_type_number(type) &&  is_java_type_number(value.type))
		{
			throw JAVA_RUNTIME_ERROR_VA(stmt->type, "Can't do an implicit cast between '%s' and '%s'.", java_type_cstring(value.type), stmt->type.lexeme);
		}

		value.is_null = (value.type == JavaType::_null);
	}
	return value;
}

void Interpreter::execute_statement(Stmt* statement) {
	switch (statement->get_type()) {
		case StmtType::Break: {
			this->broke = true;
		} break;

		case StmtType::Block: {
			Stmt_Block* stmt = dynamic_cast<Stmt_Block*>(statement);
			auto block_environment = DBG_new Environment(environment);
			execute_block(stmt->statements, block_environment);
		} break;

		case StmtType::Class: {
			Stmt_Class* stmt = dynamic_cast<Stmt_Class*>(statement);
			JavaClass *class_info = DBG_new JavaClass{ 
				this,
				std::string(stmt->name.lexeme),
				stmt->name.line,
				stmt->name.column,
				stmt->attributes,
				stmt->methods,
			};
			environment->define(stmt->name, JavaType::Class, JavaVariable{
				.object = {JavaType::Class, {.class_info = class_info} },
				.visibility = Visibility::Public,
				.is_static = false,
				.is_final = true,
				.is_uninitialized = false,
			});
		} break;

		case StmtType::Continue: {
			this->continued = true;
		} break;

		case StmtType::Expression: { 
			Stmt_Expression* stmt = dynamic_cast<Stmt_Expression*>(statement);
			JavaObject value = evaluate((Expr*)stmt->expression);
			if (REPL) {
				AstPrinter::println("Expression Ast: ", (Expr*)stmt->expression);
				printf("Expression statement result: ");
				java_object_print(value);
				printf("\n\n");
			}
		} break;

		case StmtType::Function: {
			Stmt_Function* stmt = dynamic_cast<Stmt_Function*>(statement);
			void* fn = DBG_new JavaFunction(stmt);
			JavaVariable function = {
				.object = {
					JavaType::Function,
					JavaValue{ .function = fn }
				},
				.visibility = stmt->visibility,
				.is_static = stmt->is_static,
				.is_final = true,
				.is_uninitialized = false,
			};
			globals->define(stmt->name, JavaType::Function, function);
		} break;

		case StmtType::If: { 
			Stmt_If* stmt = dynamic_cast<Stmt_If*>(statement);
			JavaObject condition = evaluate((Expr*)stmt->condition);
			if (condition.type != JavaType::_boolean) {
				throw JAVA_RUNTIME_ERROR(stmt->token, "Condition must be boolean");
			}

			if (condition.value._boolean) {
				execute_statement((Stmt*)stmt->then_branch);
			}
			else {
				bool matched = false;
				for (int i = 0; i < stmt->else_ifs.size(); i++) {
					const Else_If& else_if = stmt->else_ifs.at(i);
					JavaObject else_if_condition = evaluate((Expr*)else_if.condition);
					if (else_if_condition.type != JavaType::_boolean) {
						throw JAVA_RUNTIME_ERROR(else_if.token, "Condition must be boolean");
					}
					if (else_if_condition.value._boolean) {
						matched = true;
						execute_statement((Stmt*)else_if.then_branch);
						break;
					}
				}
				if (stmt->else_branch != nullptr && !matched) {
					execute_statement((Stmt*)stmt->else_branch);
				}
			}
		} break;

		case StmtType::Print: { 
			Stmt_Print* stmt = dynamic_cast<Stmt_Print*>(statement);
			JavaObject value = evaluate((Expr*)stmt->expression);
			if (value.type == JavaType::_void) {
				throw JAVA_RUNTIME_ERROR(stmt->token, "Can't print void.");
			}
			if (REPL) AstPrinter::println("Print Ast: ", (Expr*)stmt->expression);
			java_object_print(value);
			if (stmt->has_newline) printf("\n");
		} break;

		case StmtType::Return: {
			Stmt_Return* stmt = dynamic_cast<Stmt_Return*>(statement);
			JavaObject value = { JavaType::_void, JavaValue{} };
			if (stmt->value != nullptr) {
				value = evaluate((Expr*)stmt->value);
			}
			throw Return{ value };
		} break;

		case StmtType::Var: {
			Stmt_Var* stmt = dynamic_cast<Stmt_Var*>(statement);

			assert(stmt->names.size() == stmt->initializers.size());

			for (int i = 0; i < stmt->names.size(); i++) {
				Expr* initializer = stmt->initializers.at(i);
				const Token& name = stmt->names.at(i);
				JavaType type = token_type_to_java_type(stmt->type.type);
				JavaObject value = validate_variable(stmt, type, name, initializer);
				environment->define(stmt, name, initializer, type, value);

				if (REPL) {
					printf("Defined %s ", stmt->is_static ? "static" : "non static");
					printf("(%s %s) ", stmt->is_final ? "final" : "var", name.lexeme);
					printf("of type (%s) with visibility ", stmt->type.lexeme);
					printf("%s", visibility_to_cstring(stmt->visibility));
					if (initializer != nullptr) {
						printf(" initialized with ");
						java_object_print(value);
					}
					printf("\n");
				}
			}
		} break;

		case StmtType::While: {
			Stmt_While* stmt = dynamic_cast<Stmt_While*>(statement);
			JavaObject condition = evaluate((Expr*)stmt->condition);
			if (condition.type != JavaType::_boolean) {
				throw JAVA_RUNTIME_ERROR(stmt->token, "Expected boolean condition.");
			}
			while (condition.value._boolean) {
				execute_statement((Stmt*)stmt->body);
				if (this->broke) {
					this->broke = false;
					break;
				}
				if (this->continued) {
					this->continued = false;
					if (((Stmt*)stmt->body)->get_type() == StmtType::Block && stmt->has_increment) {
						Stmt_Block* block = dynamic_cast<Stmt_Block*>((Stmt*)stmt->body);
						execute_statement(block->statements.back());
					}
				}
				condition = evaluate((Expr*)stmt->condition);
			}
		} break;
	}
}

JavaObject Interpreter::evaluate_binary(Expr* expression) {
	Expr_Binary* expr = dynamic_cast<Expr_Binary*>(expression);
	JavaObject lhs = evaluate((Expr*)expr->left);
	JavaObject rhs = evaluate((Expr*)expr->right);

	JavaType smaller = java_get_smaller_type(lhs, rhs);
	JavaType bigger = java_get_bigger_type(lhs, rhs);
	JavaObject result = { bigger, JavaValue{} };

	// Runtime errors reported on the operators.
	#define op_error(message) throw JAVA_RUNTIME_ERROR(expr->_operator, message)

	// These are intended for numbers.
	#define case_binary(T, op)                                             \
		case JavaType::T: {                                                \
			result.value.T = java_cast_to##T(lhs) op java_cast_to##T(rhs); \
		} break;

	// Used for division and remainder.
	#define case_binary_right_not_zero(T, op)                  \
		case JavaType::T: {                                    \
			Java##T right = java_cast_to##T(rhs);              \
			if (right == 0) {                                  \
				op_error("Right hand side can't be zero");     \
			}                                                  \
			result.value.T = java_cast_to##T(lhs) op right;    \
		} break;

	// In case macro replace 'case_binary' or 'case_binary_right_non_zero'
	#define case_op(op, T, case_macro)               \
		case TokenType::T: {                         \
			if (smaller < JavaType::_byte)           \
			    op_error("Only numbers");            \
			switch (bigger) {                        \
				case_macro(_byte, op)                \
				case_macro(_int, op)                 \
				case_macro(_long, op)                \
				case_macro(_float, op)               \
				case_macro(_double, op)              \
				default: op_error("Only numbers.");  \
			}                                        \
			return result;                           \
		} break;

	// Intended for operators with boolean results.
	#define case_binary_bool(T, op)                                               \
		case JavaType::T: {                                                       \
			result.type = JavaType::_boolean;                                     \
			result.value._boolean = java_cast_to##T(lhs) op java_cast_to##T(rhs); \
		} break;

	// Intended for operators with boolean results.
	#define case_op_bool(op, T)                      \
		case TokenType::T: {                         \
			if (smaller < JavaType::_byte)           \
			    op_error("Only numbers");            \
			switch (bigger) {                        \
				case_binary_bool(_byte, op)          \
				case_binary_bool(_char, op)          \
				case_binary_bool(_int, op)           \
				case_binary_bool(_long, op)          \
				case_binary_bool(_float, op)         \
				case_binary_bool(_double, op)        \
				default: op_error("Only numbers.");  \
			}                                        \
			return result;                           \
		} break;

	// Same thing but for whole numbers.
	#define case_op_whole(op, T, case_macro)               \
		case TokenType::T: {                               \
			if (smaller < JavaType::_byte)                 \
			    op_error("Only numbers");                  \
			switch (bigger) {                              \
				case_macro(_byte, op)                      \
				case_macro(_int, op)                       \
				case_macro(_long, op)                      \
				default: op_error("Only whole numbers.");  \
			}                                              \
			return result;                                 \
		} break;

	// Actually calling those crazy macros to generate the code.
	switch (expr->_operator.type) {
		case_op(-, minus, case_binary)
		case_op(*, star, case_binary)
		case_op(/, slash, case_binary_right_not_zero)
		case_op(+, plus, case_binary)
		case_op_bool(>, greater)
		case_op_bool(<, less)
		case_op_bool(>=, greater_equal)
		case_op_bool(<=, less_equal)
		case_op_bool(==, equal_equal)
		case_op_bool(!=, not_equal)
		case_op_whole(%, percent_sign, case_binary_right_not_zero)
		case_op_whole(<<, left_shift, case_binary)
		case_op_whole(>>, right_shift, case_binary)
		case_op_whole(|, bitwise_or, case_binary)
		case_op_whole(^, bitwise_xor, case_binary)
		case_op_whole(&, bitwise_and, case_binary)
	}

	// These crazy macros are only for this code.
	#undef op_error
	#undef case_binary
	#undef case_binary_right_non_zero
	#undef case_binary_bool
	#undef case_op
	#undef case_op_bool
	#undef case_op_whole

	return JavaObject{ JavaType::none, JavaValue{} };
}

JavaObject Interpreter::evaluate_unary(Expr* expression) {
	Expr_Unary* expr = dynamic_cast<Expr_Unary*>(expression);
	JavaObject right = evaluate((Expr*)expr->right);

	// Runtime errors reported on the operators.
	#define op_error(message) throw JAVA_RUNTIME_ERROR(expr->_operator, message)

	// This is intended for numbers and booleans.
	#define case_unary(op, T)                   \
		case JavaType::T: {                     \
			result.value.T = op right.value.T;  \
		} break;

	// This is intended for numbers.
	#define case_op_unary(op, T)                              \
		case TokenType::T: {                                  \
			JavaObject result = { right.type, JavaValue{} };  \
			switch (right.type) {                             \
				case_unary(op, _byte)                         \
				case_unary(op, _int)                          \
				case_unary(op, _long)                         \
				case_unary(op, _float)                        \
				case_unary(op, _double)                       \
				default: op_error("Only numbers");            \
			}                                                 \
			return result;                                    \
		} break;

	// This is intended for whole numbers.
	#define case_op_unary_whole(op, T)                        \
		case TokenType::T: {                                  \
			JavaObject result = { right.type, JavaValue{} };  \
			switch (right.type) {                             \
				case_unary(op, _byte)                         \
				case_unary(op, _int)                          \
				case_unary(op, _long)                         \
				default: op_error("Only whole numbers");      \
			}                                                 \
			return result;                                    \
		} break;

	// Actual code calling some of the macros.
	switch (expr->_operator.type) {
		case_op_unary(-, minus)
		case_op_unary_whole(~, bitwise_not)

		case TokenType::_not: {
			JavaObject result = { JavaType::_boolean, JavaValue{} };
			if (right.type != JavaType::_boolean) op_error("Only booleans.");
			result.value._boolean = !right.value._boolean;
			return result;
		} break;

		// Those crazy macros exist only for this code.
		#undef op_error
		#undef case_unary
		#undef case_op_unary
		#undef case_op_unary_whole
	}
	return JavaObject{ JavaType::none, JavaValue{} };
}

JavaObject Interpreter::evaluate_increment_or_decrement(Expr* expression) {
	Expr_Increment* expr = dynamic_cast<Expr_Increment*>(expression);
	JavaObject result = environment->get(expr->name);

	#define case_op(op, T) case JavaType::T: op result.value.T; break;
	#define type_error() throw JAVA_RUNTIME_ERROR(expr->name, "Expected a number operand.")

	if (expr->is_positive) {
		switch (result.type) {
			case_op(++, _byte)
			case_op(++, _char)
			case_op(++, _int)
			case_op(++, _long)
			case_op(++, _float)
			case_op(++, _double)
			default: type_error();
		}
	}
	else {
		switch (result.type) {
			case_op(--, _byte)
			case_op(--, _char)
			case_op(--, _int)
			case_op(--, _long)
			case_op(--, _float)
			case_op(--, _double)
			default: type_error();
		}
	}
	#undef case_op
	#undef type_error

	environment->assign(expr->name, result);
	return result;
}

JavaObject Interpreter::evaluate_logical(Expr* expression) {
	Expr_Logical* expr = dynamic_cast<Expr_Logical*>(expression);
	JavaObject result = { JavaType::_boolean, JavaValue{} };

	JavaObject lhs = evaluate((Expr*)expr->left);
	if (lhs.type != JavaType::_boolean) {
		throw JAVA_RUNTIME_ERROR(expr->_operator, "Expected boolean operand on the left hand side.");
	}

	switch (expr->_operator.type) {
		case TokenType::_or: {
			if (lhs.value._boolean == true) {
				result.value._boolean = true;
			}
			else {
				JavaObject rhs = evaluate((Expr*)expr->right);
				if (rhs.type != JavaType::_boolean) {
					throw JAVA_RUNTIME_ERROR(expr->_operator, "Expected boolean operand on the right hand side.");
				}
				result.value._boolean = rhs.value._boolean;
			}
		} break;

		case TokenType::_and: {
			if (lhs.value._boolean == false) {
				result.value._boolean = false;
			}
			else {
				JavaObject rhs = evaluate((Expr*)expr->right);
				if (rhs.type != JavaType::_boolean) {
					throw JAVA_RUNTIME_ERROR(expr->_operator, "Expected boolean operand on the right hand side.");
				}
				result.value._boolean = rhs.value._boolean;
			}
		} break;
	}
	return result;
}

JavaObject Interpreter::evaluate(Expr* expression) {
	if (expression == nullptr) return JavaObject{ JavaType::none, JavaValue{} };

	switch (expression->get_type()) {
		case ExprType::assign: {
			Expr_Assign* expr = dynamic_cast<Expr_Assign*>(expression);
			JavaObject value = evaluate((Expr*)expr->rhs);
			environment->assign(expr->lhs_name, expr->line, expr->column, value);
			return value;
		} break;

		case ExprType::binary: {
			return evaluate_binary(expression);
		} break;

		case ExprType::call: {
			Expr_Call* expr = dynamic_cast<Expr_Call*>(expression);
			JavaObject callee = evaluate((Expr*)expr->callee);

			std::vector<ArgumentInfo> arguments = {};
			for (int i = 0; i < expr->arguments->size(); i++) {
				const auto& argument = expr->arguments->at(i);
				JavaObject object = evaluate((Expr*)argument.expr);
				arguments.emplace_back(object, argument.column, argument.line);
			}

			JavaCallable *function = (JavaCallable*)callee.value.function;
			return function->call(this, expr->paren.line, expr->paren.column, arguments);
		} break;

		case ExprType::cast: {
			Expr_Cast* expr = dynamic_cast<Expr_Cast*>(expression);
			JavaObject result = { expr->type, {} };
			JavaObject right = evaluate((Expr*)expr->right);

			switch (expr->type) {
				case JavaType::_byte: result.value._byte = java_cast_to_byte(right); break;
				case JavaType::_char: result.value._char = java_cast_to_char(right); break;
				case JavaType::_int: result.value._int = java_cast_to_int(right); break;
				case JavaType::_long: result.value._long = java_cast_to_long(right); break;
				case JavaType::_float: result.value._float = java_cast_to_float(right); break;
				case JavaType::_double: result.value._double = java_cast_to_double(right); break;
				default: throw JAVA_RUNTIME_ERR(java_type_cstring(expr->type), expr->line, expr->column, "Invalid type to cast.");
			}
			return result;
		} break;

		case ExprType::literal: {
			Expr_Literal* expr = dynamic_cast<Expr_Literal*>(expression);
			return expr->literal;
		} break;

		case ExprType::ternary: {
			Expr_Ternary* expr = dynamic_cast<Expr_Ternary*>(expression);
			JavaObject condition = evaluate((Expr*)expr->condition);
			if (condition.type != JavaType::_boolean) {
				throw JAVA_RUNTIME_ERROR(expr->question_mark, "Only booleans.");
			}
			if (condition.value._boolean) return evaluate((Expr*)expr->then);
			return evaluate((Expr*)expr->otherwise);
		} break;

		case ExprType::logical: {
			return evaluate_logical(expression);
		} break;

		case ExprType::get: {
			Expr_Get* expr = dynamic_cast<Expr_Get*>(expression);
			JavaObject object = evaluate((Expr*)expr->object);
			switch (object.type) {
				case JavaType::Instance: {
					JavaInstance* instance = (JavaInstance*)object.value.instance;
					return instance->get(expr);
				} break;

				case JavaType::Class: {
					JavaClass* classinfo = (JavaClass*)object.value.class_info;
					return classinfo->get(expr);
				} break;

				default: throw JAVA_RUNTIME_ERR(expr->name, expr->line, expr->column, "Only instances and classes have properties.");
			}
		} break;

		case ExprType::grouping: {
			Expr_Grouping* expr = dynamic_cast<Expr_Grouping*>(expression);
			return evaluate((Expr*)expr->expression);
		} break;

		case ExprType::increment: {
			return evaluate_increment_or_decrement(expression);
		} break;

		case ExprType::set: {
			Expr_Set* expr = dynamic_cast<Expr_Set*>(expression);
			JavaObject lhs = evaluate((Expr*)expr->lhs);
			if (lhs.type != JavaType::Instance) {
				throw JAVA_RUNTIME_ERR(expr->rhs_name, expr->line, expr->column, "Only instances have fields.");
			}
			JavaObject value = evaluate((Expr*)expr->value);
			JavaInstance* instance = (JavaInstance*)lhs.value.instance;
			instance->set(expr, value);
			return value;
		} break;

		case ExprType::unary: {
			return evaluate_unary(expression);
		} break;

		case ExprType::variable: {
			Expr_Variable* expr = dynamic_cast<Expr_Variable*>(expression);
			return environment->get(expr->name, expr->line, expr->column);
		} break;
	}

	return JavaObject{ JavaType::none, JavaValue{} };
}

