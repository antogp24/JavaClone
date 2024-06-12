#include "Interpreter.h"
#include "Error.h"
#include "AstPrinter.h"

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
	environment = globals;
}

Interpreter::~Interpreter() {
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

void Interpreter::execute_block(const std::vector<Stmt*>& statements, Environment* environment) {
	Environment* previous = this->environment;
	this->environment = environment;

	for (int i = 0; i < statements.size(); i++) {
		execute_statement(statements.at(i));
	}

	delete environment;
	this->environment = previous;
}

void Interpreter::execute_statement(Stmt* statement) {
	// if (statement == nullptr) return;

	switch (statement->get_type()) {
		case StmtType::Block: {
			Stmt_Block* stmt = dynamic_cast<Stmt_Block*>(statement);
			execute_block(stmt->statements, DBG_new Environment(environment));
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
			if (REPL) AstPrinter::println("Print Ast: ", (Expr*)stmt->expression);
			java_object_print(value);
			if (stmt->has_newline) printf("\n");
		} break;

		case StmtType::Var: {
			Stmt_Var* stmt = dynamic_cast<Stmt_Var*>(statement);
			JavaObject value = { JavaType::_null, JavaValue{} };

			if (stmt->initializer != nullptr) {
				value = evaluate((Expr*)stmt->initializer);

				if (is_token_type_number(stmt->type.type) && !is_java_type_number(value.type) ||
				   !is_token_type_number(stmt->type.type) && is_java_type_number(value.type))
				{
					throw JAVA_RUNTIME_ERROR(stmt->type, "Can't do an implicit cast between '%s' and '%s'.", java_type_cstring(value.type), stmt->type.lexeme);
				}
			}
			value.type = token_type_to_java_type(stmt->type.type);
			environment->define(stmt, value);

			if (REPL) {
				printf("Defined %s ", stmt->is_static ? "static" : "non static");
				printf("(%s %s) ", stmt->is_final ? "final" : "var", stmt->name.lexeme);
				printf("of type (%s) with visibility ", stmt->type.lexeme);
				printf("%s", visibility_to_cstring(stmt->visibility));
			}
			if (stmt->initializer != nullptr) {
				JavaObject value = evaluate((Expr*)stmt->initializer);
				if (REPL) {
					printf(" initialized with ");
					java_object_print(value);
				}
			}
			if (REPL) printf("\n");
		} break;

		case StmtType::While: {
			Stmt_While* stmt = dynamic_cast<Stmt_While*>(statement);
			JavaObject condition = evaluate((Expr*)stmt->condition);
			if (condition.type != JavaType::_boolean) {
				throw JAVA_RUNTIME_ERROR(stmt->token, "Expected boolean condition.");
			}
			while (condition.value._boolean) {
				execute_statement((Stmt*)stmt->body);
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
	#define case_binary(T, op)                                               \
		case JavaType::##T: {                                                \
			result.value.##T = java_cast_to##T(lhs) op java_cast_to##T(rhs); \
		} break;

	// Used for division and remainder.
	#define case_binary_right_not_zero(T, op)                  \
		case JavaType::##T: {                                  \
			Java##T right = java_cast_to##T(rhs);              \
			if (right == 0) {                                  \
				op_error("Right hand side can't be zero");     \
			}                                                  \
			result.value.##T = java_cast_to##T(lhs) op right;  \
		} break;

	// In case macro replace 'case_binary' or 'case_binary_right_non_zero'
	#define case_op(op, T, case_macro)               \
		case TokenType::##T: {                       \
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
		case JavaType::##T: {                                                     \
			result.type = JavaType::_boolean;                                     \
			result.value._boolean = java_cast_to##T(lhs) op java_cast_to##T(rhs); \
		} break;

	// Intended for operators with boolean results.
	#define case_op_bool(op, T)                      \
		case TokenType::##T: {                       \
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
		case TokenType::##T: {                             \
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
	#define case_unary(op, T)                                          \
		case JavaType::##T: {                                          \
			result.value.##T = result.value.##T = op right.value.##T;  \
		} break;

	// This is intended for numbers.
	#define case_op_unary(op, T)                              \
		case TokenType::##T: {                                \
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
		case TokenType::##T: {                                \
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

		// Macros for casting operations.
		#define case_cast(T) case_unary((Java##T), T)
		#define case_cast_number(T)                                                           \
			case TokenType::type##T: {                                                        \
				JavaObject result = { JavaType::##T, JavaValue{} };                           \
				switch (right.type) {                                                         \
					case_cast(_boolean)                                                       \
					case_cast(_byte)                                                          \
					case_cast(_char)                                                          \
					case_cast(_int)                                                           \
					case_cast(_long)                                                          \
					case_cast(_float)                                                         \
					case_cast(_double)                                                        \
					default: {                                                                \
						throw JAVA_RUNTIME_ERROR(expr->_operator, "Type must be a number.");  \
					}                                                                         \
				}                                                                             \
				return result;                                                                \
			}

		// Calling the cast operation macros.
		case_cast_number(_boolean)
		case_cast_number(_byte)
		case_cast_number(_char)
		case_cast_number(_int)
		case_cast_number(_long)
		case_cast_number(_float)
		case_cast_number(_double)

		// Those crazy macros exist only for this code.
		#undef op_error
		#undef case_unary
		#undef case_op_unary
		#undef case_op_unary_whole
		#undef case_cast
		#undef case_cast_number
	}
	return JavaObject{ JavaType::none, JavaValue{} };
}

JavaObject Interpreter::evaluate_increment_or_decrement(Expr* expression) {
	Expr_Increment* expr = dynamic_cast<Expr_Increment*>(expression);
	JavaObject result = environment->get(expr->name);

	#define case_op(op, T) case JavaType::##T: op result.value.##T; break;
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
			environment->assign(expr->lhs_name, value);
			return value;
		} break;

		case ExprType::binary: {
			return evaluate_binary(expression);
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

		case ExprType::grouping: {
			Expr_Grouping* expr = dynamic_cast<Expr_Grouping*>(expression);
			return evaluate((Expr*)expr->expression);
		} break;

		case ExprType::increment: {
			return evaluate_increment_or_decrement(expression);
		} break;

		case ExprType::unary: {
			return evaluate_unary(expression);
		} break;

		case ExprType::variable: {
			Expr_Variable* expr = dynamic_cast<Expr_Variable*>(expression);
			return environment->get(expr->name);
		} break;
	}

	return JavaObject{ JavaType::none, JavaValue{} };
}

void Interpreter::check_number_operands(const JavaObject& lhs, const Token& _operator, const JavaObject& rhs) {
	if (is_java_type_number(lhs.type) && is_java_type_number(rhs.type)) {
		return;
	}
	throw JAVA_RUNTIME_ERROR(_operator, "Expected number operands.");
}
