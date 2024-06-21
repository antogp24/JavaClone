#include "Stmt.h"

void statement_free(Stmt* statement) {
	if (statement == nullptr) return;

	switch (statement->get_type()) {
		case StmtType::Break: {
			Stmt_Break* stmt = dynamic_cast<Stmt_Break*>(statement);
			delete stmt;
		} break;

		case StmtType::Block: {
			Stmt_Block* stmt = dynamic_cast<Stmt_Block*>(statement);
			for (int i = 0; i < stmt->statements.size(); i++) {
				statement_free(stmt->statements.at(i));
			}
			delete stmt;
		} break;

		case StmtType::Continue: {
			Stmt_Continue* stmt = dynamic_cast<Stmt_Continue*>(statement);
			delete stmt;
		} break;

		case StmtType::If: {
			Stmt_If* stmt = dynamic_cast<Stmt_If*>(statement);
			expression_free((Expr*)stmt->condition);
			statement_free((Stmt*)stmt->then_branch);
			for (int i = 0; i < stmt->else_ifs.size(); i++) {
				const Else_If &else_if = stmt->else_ifs.at(i);
				expression_free((Expr*)else_if.condition);
				statement_free((Stmt*)else_if.then_branch);
			}
			statement_free((Stmt*)stmt->else_branch);
			delete stmt;
		} break;

		case StmtType::Expression: {
			Stmt_Expression* stmt = dynamic_cast<Stmt_Expression*>(statement);
			expression_free((Expr*)stmt->expression);
			delete stmt;
		} break;

		case StmtType::Print: {
			Stmt_Print* stmt = dynamic_cast<Stmt_Print*>(statement);
			expression_free((Expr*)stmt->expression);
			delete stmt;
		} break;

		case StmtType::Var: {
			Stmt_Var* stmt = dynamic_cast<Stmt_Var*>(statement);
			for (int i = 0; i < stmt->initializers.size(); i++) {
				expression_free(stmt->initializers.at(i));
			}
			delete stmt;
		} break;

		case StmtType::While: {
			Stmt_While* stmt = dynamic_cast<Stmt_While*>(statement);
			expression_free((Expr*)stmt->condition);
			statement_free((Stmt*)stmt->body);
			delete stmt;
		} break;
	}
}