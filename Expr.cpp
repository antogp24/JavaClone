#include "Expr.h"

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

void expr_free(Expr* _expr) {
	if (_expr == nullptr) return;

	switch (_expr->get_type()) {
		case ExprType::assign: {
			Expr_Assign* expr = dynamic_cast<Expr_Assign*>(_expr);
			expr_free((Expr*)expr->lhs);
			expr_free((Expr*)expr->rhs);
			delete expr;
		} break;

		case ExprType::binary: {
			Expr_Binary* expr = dynamic_cast<Expr_Binary*>(_expr);
			expr_free((Expr*)expr->left);
			expr_free((Expr*)expr->right);
			delete expr;
		} break;

		case ExprType::call: {
			Expr_Call* expr = dynamic_cast<Expr_Call*>(_expr);
			expr_free((Expr*)expr->callee);
			for (int i = 0; i < expr->arguments->size(); i++) {
				expr_free((Expr*)expr->arguments->at(i).expr);
			}
			delete expr->arguments;
			delete expr;
		} break;

		case ExprType::cast: {
			Expr_Cast* expr = dynamic_cast<Expr_Cast*>(_expr);
			expr_free((Expr*)expr->right);
			delete expr;
		} break;

		case ExprType::get: {
			Expr_Get* expr = dynamic_cast<Expr_Get*>(_expr);
			expr_free((Expr*)expr->object);
			delete expr;
		} break;

		case ExprType::grouping: {
			Expr_Grouping* expr = dynamic_cast<Expr_Grouping*>(_expr);
			expr_free((Expr*)expr->expression);
			delete expr;
		} break;

		case ExprType::increment: {
			Expr_Increment* expr = dynamic_cast<Expr_Increment*>(_expr);
			delete expr;
		} break;

		case ExprType::literal: {
			Expr_Literal* expr = dynamic_cast<Expr_Literal*>(_expr);
			delete expr;
		} break;

		case ExprType::logical: {
			Expr_Logical* expr = dynamic_cast<Expr_Logical*>(_expr);
			expr_free((Expr*)expr->left);
			expr_free((Expr*)expr->right);
			delete expr;
		} break;

		case ExprType::set: {
			Expr_Set* expr = dynamic_cast<Expr_Set*>(_expr);
			expr_free((Expr*)expr->lhs);
			expr_free((Expr*)expr->rhs);
			delete expr;
		} break;

		case ExprType::ternary: {
			Expr_Ternary* expr = dynamic_cast<Expr_Ternary*>(_expr);
			expr_free((Expr*)expr->condition);
			expr_free((Expr*)expr->then);
			expr_free((Expr*)expr->otherwise);
			delete expr;
		} break;

		case ExprType::unary: {
			Expr_Unary* expr = dynamic_cast<Expr_Unary*>(_expr);
			expr_free((Expr*)expr->right);
			delete expr;
		} break;

		case ExprType::variable: {
			Expr_Variable* expr = dynamic_cast<Expr_Variable*>(_expr);
			delete expr;
		} break;
	}
}