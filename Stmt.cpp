#include "Stmt.h"

void statement_free(Stmt* statement) {
	if (statement == nullptr) return;

	switch (statement->get_type()) {
		case StmtType::expression: {
			Stmt_Expression* stmt = dynamic_cast<Stmt_Expression*>(statement);
			expression_free((Expr*)stmt->expression);
			delete stmt;
		} break;
		case StmtType::print: {
			Stmt_Print* stmt = dynamic_cast<Stmt_Print*>(statement);
			expression_free((Expr*)stmt->expression);
			delete stmt;
		} break;
		case StmtType::var: {
			Stmt_Var* stmt = dynamic_cast<Stmt_Var*>(statement);
			expression_free((Expr*)stmt->initializer);
			delete stmt;
		} break;
		case StmtType::block: {
			Stmt_Block* stmt = dynamic_cast<Stmt_Block*>(statement);
			for (int i = 0; i < stmt->statements.size(); i++) {
				statement_free(stmt->statements.at(i));
			}
			delete stmt;
		} break;
	}
}