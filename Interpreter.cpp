#include "Interpreter.h"
#include "Error.h"

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

void Interpreter::interpret(Expr *expression) {
	try {
		JavaObject value = evaluate(expression);
		printf("Result: ");
		java_object_print(value);
		printf("\n\n");
	}
	catch (JavaRuntimeError error) {
		JavaError::runtime_error(error);
	}
}

JavaObject Interpreter::evaluate_binary(Expr* expression) {
	Expr_Binary* expr = dynamic_cast<Expr_Binary*>(expression);
	JavaObject lhs = evaluate((Expr*)expr->left);
	JavaObject rhs = evaluate((Expr*)expr->right);

	JavaType type = java_get_bigger_type(lhs, rhs);
	JavaObject result = { type, JavaValue{} };

	// These are intended for numbers.
	#define case_binary(T, op)                                               \
		case JavaType::##T: {                                                \
			result.value.##T = java_cast_to##T(lhs) op java_cast_to##T(rhs); \
		} break;

	// Used for division and remainder.
	#define case_binary_right_not_zero(T, op)                                              \
		case JavaType::##T: {                                                              \
			Java##T right = java_cast_to##T(rhs);                                          \
			if (right == 0) {                                                              \
				throw JavaRuntimeError(expr->_operator, "Right hand side can't be zero");  \
			}                                                                              \
			result.value.##T = java_cast_to##T(lhs) op right;                              \
		} break;

	// In case macro replace 'case_binary' or 'case_binary_right_non_zero'
	#define case_op(op, T, case_macro)                                         \
		case TokenType::##T: {                                                 \
			switch (type) {                                                    \
				case_macro(_byte, op)                                          \
				case_macro(_int, op)                                           \
				case_macro(_long, op)                                          \
				case_macro(_float, op)                                         \
				case_macro(_double, op)                                        \
				default: {                                                     \
					throw JavaRuntimeError(expr->_operator, "Only numbers.");  \
				}                                                              \
			}                                                                  \
			return result;                                                     \
		}

	// Intended for operators with boolean results.
	#define case_binary_bool(T, op)                                               \
		case JavaType::##T: {                                                     \
			result.type = JavaType::_boolean;                                     \
			result.value._boolean = java_cast_to##T(lhs) op java_cast_to##T(rhs); \
		} break;

	// Intended for operators with boolean results.
	#define case_op_bool(op, T)                                                \
		case TokenType::##T: {                                                 \
			switch (type) {                                                    \
				case_binary_bool(_byte, op)                                    \
				case_binary_bool(_int, op)                                     \
				case_binary_bool(_long, op)                                    \
				case_binary_bool(_float, op)                                   \
				case_binary_bool(_double, op)                                  \
				default: {                                                     \
					throw JavaRuntimeError(expr->_operator, "Only numbers.");  \
				}                                                              \
			}                                                                  \
			return result;                                                     \
		}

	// Same thing but for whole numbers.
	#define case_op_whole(op, T, case_macro)                                                     \
		case TokenType::##T: {                                                                   \
			switch (type) {                                                                      \
				case_macro(_byte, op)                                                            \
				case_macro(_int, op)                                                             \
				case_macro(_long, op)                                                            \
				default: {                                                                       \
					throw JavaRuntimeError(expr->_operator, "Only non floating point numbers."); \
				}                                                                                \
			}                                                                                    \
			return result;                                                                       \
		}

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

	// This is intended for numbers and booleans.
	#define case_unary(op, T)                                          \
		case JavaType::##T: {                                          \
			result.value.##T = result.value.##T = op right.value.##T;  \
		} break;

	switch (expr->_operator.type) {
		case TokenType::minus: {
			JavaObject result = { right.type, JavaValue{} };
			switch (right.type) {
				case_unary(-, _byte)
				case_unary(-, _int)
				case_unary(-, _long)
				case_unary(-, _float)
				case_unary(-, _double)
				default: {
					throw JavaRuntimeError(expr->_operator, "Only numbers");
				}
			}
			return result;
		}

		case TokenType::_not: {
			JavaObject result = { JavaType::_boolean, JavaValue{} };
			switch (right.type) {
				case_unary(!, _boolean)
				default: {
					throw JavaRuntimeError(expr->_operator, "Only booleans.");
				}
			}
			return result;
		}

		case TokenType::bitwise_not: {
			JavaObject result = { right.type, JavaValue{} };
			switch (right.type) {
				case_unary(~, _byte)
				case_unary(~, _int)
				case_unary(~, _long)
				default: {
					throw JavaRuntimeError(expr->_operator, "Only whole numbers.");
				}
			}
			return result;
		}

		#define case_cast(T) case_unary((Java##T), T)
		#define case_cast_number(T)                                                         \
			case TokenType::type##T: {                                                      \
				JavaObject result = { JavaType::##T, JavaValue{} };                         \
				switch (right.type) {                                                       \
					case_cast(_boolean)                                                     \
					case_cast(_byte)                                                        \
					case_cast(_char)                                                        \
					case_cast(_int)                                                         \
					case_cast(_long)                                                        \
					case_cast(_float)                                                       \
					case_cast(_double)                                                      \
					default: {                                                              \
						throw JavaRuntimeError(expr->_operator, "Type must be a number.");  \
					}                                                                       \
				}                                                                           \
				return result;                                                              \
			}

		case_cast_number(_boolean)
		case_cast_number(_byte)
		case_cast_number(_char)
		case_cast_number(_int)
		case_cast_number(_long)
		case_cast_number(_float)
		case_cast_number(_double)

		#undef case_unary
		#undef case_cast
		#undef case_cast_number
	}
	return JavaObject{ JavaType::none, JavaValue{} };
}

JavaObject Interpreter::evaluate_logical(Expr* expression) {
	Expr_Logical* expr = dynamic_cast<Expr_Logical*>(expression);
	JavaObject result = { JavaType::_boolean, JavaValue{} };

	JavaObject lhs = evaluate((Expr*)expr->left);
	if (lhs.type != JavaType::_boolean) {
		throw JavaRuntimeError(expr->_operator, "Expected boolean operand on the left hand side.");
	}

	switch (expr->_operator.type) {
		case TokenType::_or: {
			if (lhs.value._boolean == true) {
				result.value._boolean = true;
			}
			else {
				JavaObject rhs = evaluate((Expr*)expr->right);
				if (rhs.type != JavaType::_boolean) {
					throw JavaRuntimeError(expr->_operator, "Expected boolean operand on the right hand side.");
				}
				result.value._boolean = lhs.value._boolean || rhs.value._boolean;
			}
		} break;

		case TokenType::_and: {
			if (lhs.value._boolean == false) {
				result.value._boolean = false;
			}
			else {
				JavaObject rhs = evaluate((Expr*)expr->right);
				if (rhs.type != JavaType::_boolean) {
					throw JavaRuntimeError(expr->_operator, "Expected boolean operand on the right hand side.");
				}
				result.value._boolean = lhs.value._boolean && rhs.value._boolean;
			}
		} break;
	}
	return result;
}

JavaObject Interpreter::evaluate(Expr* expression) {
	if (expression == nullptr) return JavaObject{ JavaType::none, JavaValue{} };

	switch (expression->get_type()) {
		case ExprType::binary: {
			return evaluate_binary(expression);
		} break;

		case ExprType::literal: {
			Expr_Literal* expr = dynamic_cast<Expr_Literal*>(expression);
			return expr->literal;
		} break;

		case ExprType::logical: {
			return evaluate_logical((Expr*)expression);
		} break;

		case ExprType::grouping: {
			Expr_Grouping* expr = dynamic_cast<Expr_Grouping*>(expression);
			return evaluate((Expr*)expr->expression);
		} break;

		case ExprType::unary: {
			return evaluate_unary(expression);
		} break;
	}

	return JavaObject{ JavaType::none, JavaValue{} };
}

void Interpreter::check_number_operands(const JavaObject& lhs, const Token& _operator, const JavaObject& rhs) {
	if (is_java_type_number(lhs.type) && is_java_type_number(rhs.type)) {
		return;
	}
	throw JavaRuntimeError(_operator, "Expected number operands.");
}
