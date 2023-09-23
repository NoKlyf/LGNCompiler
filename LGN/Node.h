#pragma once
#include "Token.h"
#include <variant>
#include <vector>

namespace lgn::node
{
	struct Expr;
	struct Statement;

	struct BinExprAdd {
		Expr *left;
		Expr *right;
	};

	struct BinExprSub {
		Expr* left;
		Expr* right;
	};

	struct BinExprMul {
		Expr *left;
		Expr *right;
	};

	struct BinExprDiv {
		Expr* left;
		Expr* right;
	};

	struct BinExpr {
		std::variant<BinExprAdd*, BinExprSub*, BinExprMul*, BinExprDiv*> expr;
	};

	struct Scope {
		std::vector<Statement*> statements;
	};

	struct TermInt {
		Token tok_int;
	};

	struct TermId {
		Token tok_id;
	};

	struct TermParen {
		Expr* expr;
	};

	struct Term {
		std::variant<TermInt*, TermId*, TermParen*> term;
	};

	struct Expr {
		std::variant<Term*, BinExpr*> expr;
	};

	struct StatementExit {
		Expr *expr;
	};

	struct StatementLet {
		Token tok_id;
		Expr *expr;
	};

	struct StatementIf {
		Expr* expr;
		Scope* scope;
	};

	struct Statement {
		std::variant<StatementExit*, StatementLet*, StatementIf*, Scope*> statement;
	};

	struct Program {
		std::vector<Statement*> statements;
	};
}
