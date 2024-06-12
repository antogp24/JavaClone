#include "Expr.h"

void expression_free(Expr* _expr) {
	if (_expr == nullptr) return;

	switch (_expr->get_type()) {
		case ExprType::assign: {
			Expr_Assign* expr = dynamic_cast<Expr_Assign*>(_expr);
			expression_free((Expr*)expr->lhs);
			expression_free((Expr*)expr->rhs);
			free(expr);
		} break;

		case ExprType::binary: {
			Expr_Binary* expr = dynamic_cast<Expr_Binary*>(_expr);
			expression_free((Expr*)expr->left);
			expression_free((Expr*)expr->right);
			free(expr);
		} break;

		case ExprType::call: {
			Expr_Call* expr = dynamic_cast<Expr_Call*>(_expr);
			expression_free((Expr*)expr->callee);
			for (int i = 0; i < expr->arguments->size(); i++) {
				expression_free((Expr*)expr->arguments->at(i));
			}
			delete expr->arguments;
			free(expr);
		} break;

		case ExprType::get: {
			Expr_Get* expr = dynamic_cast<Expr_Get*>(_expr);
			expression_free((Expr*)expr->object);
			free(expr);
		} break;

		case ExprType::grouping: {
			Expr_Grouping* expr = dynamic_cast<Expr_Grouping*>(_expr);
			expression_free((Expr*)expr->expression);
			free(expr);
		} break;

		case ExprType::increment: {
			Expr_Increment* expr = dynamic_cast<Expr_Increment*>(_expr);
			free(expr);
		} break;

		case ExprType::literal: {
			Expr_Literal* expr = dynamic_cast<Expr_Literal*>(_expr);
			free(expr);
		} break;

		case ExprType::logical: {
			Expr_Logical* expr = dynamic_cast<Expr_Logical*>(_expr);
			expression_free((Expr*)expr->left);
			expression_free((Expr*)expr->right);
			free(expr);
		} break;

		case ExprType::set: {
			Expr_Set* expr = dynamic_cast<Expr_Set*>(_expr);
			expression_free((Expr*)expr->lhs);
			expression_free((Expr*)expr->rhs);
			free(expr);
		} break;

		case ExprType::ternary: {
			Expr_Ternary* expr = dynamic_cast<Expr_Ternary*>(_expr);
			expression_free((Expr*)expr->condition);
			expression_free((Expr*)expr->then);
			expression_free((Expr*)expr->otherwise);
			free(expr);
		} break;

		case ExprType::unary: {
			Expr_Unary* expr = dynamic_cast<Expr_Unary*>(_expr);
			expression_free((Expr*)expr->right);
			free(expr);
		} break;

		case ExprType::variable: {
			Expr_Variable* expr = dynamic_cast<Expr_Variable*>(_expr);
			free(expr);
		} break;
	}
}