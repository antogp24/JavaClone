#pragma once

#include "Expr.h"
#include <stdio.h>
#include <stdarg.h>

struct AstPrinter {

	static void parenthesize(const char *name, ...) {
		va_list ap;
		va_start(ap, name);
		printf("(%s", name);
		for (Expr* expr = va_arg(ap, Expr*); expr != NULL; expr = va_arg(ap, Expr*)) {
			printf(" ");
			print(expr);
		}
		printf(")");
		va_end(ap);
	}

	static void println(const char *message, Expr *expr) {
		printf(message);
		print(expr);
		printf("\n");
	}

	static void print(Expr *_expr) {
		if (_expr == nullptr) {
			printf("(invalid|empty)");
			return;
		}
		switch (_expr->get_type()) {
			case ExprType::assign: {
				Expr_Assign* expr = dynamic_cast<Expr_Assign*>(_expr);
				printf("(assign ");
				print((Expr*)expr->lhs);
				printf(" = ");
				print((Expr*)expr->rhs);
				printf(")");
			} break;

			case ExprType::binary: {
				Expr_Binary* expr = dynamic_cast<Expr_Binary*>(_expr);
				parenthesize(expr->_operator.lexeme, expr->left, expr->right, NULL);
			} break;

			case ExprType::call: {
				Expr_Call* expr = dynamic_cast<Expr_Call*>(_expr);
				printf("(call ");
				print((Expr*)expr->callee);
				for (int i = 0; i < expr->arguments->size(); i++) {
					printf(" ");
					print((Expr*)expr->arguments->at(i).expr);
					if (i != expr->arguments->size() - 1) {
						printf(", ");
					}
				}
				printf(")");
			} break;

			case ExprType::get: {
				Expr_Get* expr = dynamic_cast<Expr_Get*>(_expr);
				printf("(get ");
				print((Expr*)expr->object);
				printf(".%s)", expr->name.c_str());
			} break;

			case ExprType::grouping: {
				Expr_Grouping* expr = dynamic_cast<Expr_Grouping*>(_expr);
				parenthesize("group", expr->expression, NULL);
			} break;

			case ExprType::literal: {
				Expr_Literal* expr = dynamic_cast<Expr_Literal*>(_expr);
				java_object_print(expr->literal);
			} break;

			case ExprType::logical: {
				Expr_Logical* expr = dynamic_cast<Expr_Logical*>(_expr);
				parenthesize(expr->_operator.lexeme, expr->left, expr->right, NULL);
			} break;

			case ExprType::set: {
				Expr_Set* expr = dynamic_cast<Expr_Set*>(_expr);
				printf("(set ");
				print((Expr*)expr->lhs);
				printf(" = ");
				print((Expr*)expr->value);
				printf(")");
			} break;

			case ExprType::ternary: {
				Expr_Ternary* expr = dynamic_cast<Expr_Ternary*>(_expr);
				printf("(ternary ");
				print((Expr *)expr->condition);
				printf(" ? ");
				print((Expr *)expr->then);
				printf(" : ");
				print((Expr *)expr->otherwise);
				printf(")");
			} break;

			case ExprType::unary: {
				Expr_Unary* expr = dynamic_cast<Expr_Unary*>(_expr);
				parenthesize(expr->_operator.lexeme, expr->right, NULL);
			} break;

			case ExprType::variable: {
				Expr_Variable* expr = dynamic_cast<Expr_Variable*>(_expr);
				printf(expr->is_function ? "(fn " : "(var ");
				printf("%s)", expr->name.c_str());
			} break;
		}
	}
};