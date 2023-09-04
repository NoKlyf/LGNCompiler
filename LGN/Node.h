#pragma once
#include "Token.h";
#include <variant>
#include <vector>

namespace lgn::node
{
	struct Expr;

	struct BinExprAdd {
		Expr *left;
		Expr *right;
	};

	/*struct BinExprMul {
		Expr *left;
		Expr *right;
	};*/

	struct BinExpr {
		BinExprAdd* expr; // Make std::variant<BinExprAdd*, BinExprMul*> expr;
	};

	struct TermInt {
		Token tok_int;
	};

	struct TermId {
		Token tok_id;
	};

	struct Term {
		std::variant<TermInt*, TermId*> term;
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

	struct Statement {
		std::variant<StatementExit*, StatementLet*> statement;
	};

	struct Program {
		std::vector<Statement*> statements;
	};
}