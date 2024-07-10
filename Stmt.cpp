#include "Stmt.h"

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

void stmt_free(Stmt* statement) {
	if (statement == nullptr) return;

	switch (statement->get_type()) {
		case StmtType::Break: {
			Stmt_Break* stmt = dynamic_cast<Stmt_Break*>(statement);
			delete stmt;
		} break;

		case StmtType::Block: {
			Stmt_Block* stmt = dynamic_cast<Stmt_Block*>(statement);
			for (int i = 0; i < stmt->statements.size(); i++) {
				stmt_free(stmt->statements.at(i));
			}
			delete stmt;
		} break;

		case StmtType::Class: {
			Stmt_Class* stmt = dynamic_cast<Stmt_Class*>(statement);
			for (int i = 0; i < stmt->attributes.size(); i++) {
				stmt_free(stmt->attributes.at(i));
			}
			for (int i = 0; i < stmt->methods.size(); i++) {
				stmt_free(stmt->methods.at(i));
			}
			delete stmt;
		} break;

		case StmtType::Continue: {
			Stmt_Continue* stmt = dynamic_cast<Stmt_Continue*>(statement);
			delete stmt;
		} break;

		case StmtType::If: {
			Stmt_If* stmt = dynamic_cast<Stmt_If*>(statement);
			expr_free((Expr*)stmt->condition);
			stmt_free((Stmt*)stmt->then_branch);
			for (int i = 0; i < stmt->else_ifs.size(); i++) {
				const Else_If &else_if = stmt->else_ifs.at(i);
				expr_free((Expr*)else_if.condition);
				stmt_free((Stmt*)else_if.then_branch);
			}
			stmt_free((Stmt*)stmt->else_branch);
			delete stmt;
		} break;

		case StmtType::Expression: {
			Stmt_Expression* stmt = dynamic_cast<Stmt_Expression*>(statement);
			expr_free((Expr*)stmt->expression);
			delete stmt;
		} break;

		case StmtType::Function: {
			Stmt_Function* stmt = dynamic_cast<Stmt_Function*>(statement);
			// Body and params are deleted at the Interpreter's destructor.
			// This must also be taken into consideration for class methods.
			delete stmt;
		} break;

		case StmtType::Print: {
			Stmt_Print* stmt = dynamic_cast<Stmt_Print*>(statement);
			expr_free((Expr*)stmt->expression);
			delete stmt;
		} break;

		case StmtType::Return: {
			Stmt_Return* stmt = dynamic_cast<Stmt_Return*>(statement);
			expr_free((Expr*)stmt->value);
			delete stmt;
		} break;

		case StmtType::Var: {
			Stmt_Var* stmt = dynamic_cast<Stmt_Var*>(statement);
			for (int i = 0; i < stmt->initializers.size(); i++) {
				expr_free(stmt->initializers.at(i));
			}
			delete stmt;
		} break;

		case StmtType::While: {
			Stmt_While* stmt = dynamic_cast<Stmt_While*>(statement);
			expr_free((Expr*)stmt->condition);
			stmt_free((Stmt*)stmt->body);
			delete stmt;
		} break;
	}
}