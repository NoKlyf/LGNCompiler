#include "Parser.h"
#include "Lexer.h"
using namespace lgn;

std::optional<node::Program> Parser::parse()
{
	node::Program prog;

	while (peek().has_value()) {
		if (auto stmt = parse_stmt()) {
			prog.statements.push_back(stmt.value());
		}
	}

	return prog;
}

std::optional<node::Term*> lgn::Parser::parse_term()
{
	if (auto tok_int = try_consume(TokenType::tok_int)) {
		auto term_int = m_allocator.alloc<node::TermInt>();
		term_int->tok_int = tok_int.value();

		auto term = m_allocator.alloc<node::Term>();
		term->term = term_int;

		return term;
	} else if (auto tok_id = try_consume(TokenType::tok_id)) {
		auto term_id = m_allocator.alloc<node::TermId>();
		term_id->tok_id = tok_id.value();

		auto term = m_allocator.alloc<node::Term>();
		term->term = term_id;

		return term;
	} else if (auto tok_paren = try_consume(TokenType::tok_lparen)) {
		auto expr = parse_expr();

		if (!expr.has_value()) {
			std::cerr << "Expected expression" << std::endl;
			exit(EXIT_SUCCESS);
		}

		try_consume(TokenType::tok_rparen, "Expected ')'");

		auto term_paren = m_allocator.alloc<node::TermParen>();
		term_paren->expr = expr.value();

		auto term = m_allocator.alloc<node::Term>();
		term->term = term_paren;

		return term;
	}

	return {};
}

std::optional<node::Expr*> Parser::parse_expr(int min_prec)
{
	std::optional<node::Term*> term_left = parse_term();

	if (!term_left.has_value()) {
		return {};
	}

	auto expr_left = m_allocator.alloc<node::Expr>();
	expr_left->expr = term_left.value();

	while (true) {
		std::optional<Token> curr_tok = peek();
		std::optional<int> prec;

		if (!curr_tok.has_value()) {
			break;
		}

		prec = bin_prec(curr_tok->type);

		if (!prec.has_value() || prec < min_prec) {
			break;
		}

		Token op = consume();
		int next_min_prec = prec.value() + 1;
		auto expr_right = parse_expr(next_min_prec);

		if (!expr_right.has_value()) {
			std::cerr << "Unable to parse expression" << std::endl;
			exit(EXIT_SUCCESS);
		}

		auto expr = m_allocator.alloc<node::BinExpr>();
		auto expr_lhs = m_allocator.alloc<node::Expr>();

		if (op.type == TokenType::tok_plus) {
			auto add = m_allocator.alloc<node::BinExprAdd>();

			expr_lhs->expr = expr_left->expr;

			add->left = expr_lhs;
			add->right = expr_right.value();
			
			expr->expr = add;
		} else if (op.type == TokenType::tok_min) {
			auto sub = m_allocator.alloc<node::BinExprSub>();

			expr_lhs->expr = expr_left->expr;

			sub->left = expr_lhs;
			sub->right = expr_right.value();

			expr->expr = sub;
		} else if (op.type == TokenType::tok_mul) {
			auto mul = m_allocator.alloc<node::BinExprMul>();

			expr_lhs->expr = expr_left->expr;

			mul->left = expr_lhs;
			mul->right = expr_right.value();

			expr->expr = mul;
		} else if (op.type == TokenType::tok_div) {
			auto div = m_allocator.alloc<node::BinExprDiv>();

			expr_lhs->expr = expr_left->expr;

			div->left = expr_lhs;
			div->right = expr_right.value();

			expr->expr = div;
		}

		expr_left->expr = expr;
	}

	return expr_left;
}

std::optional<node::BinExpr*> lgn::Parser::parse_bin_expr()
{
	if (auto left = parse_expr()) {
		auto bin_expr = m_allocator.alloc<node::BinExpr>();
		
		if (peek().has_value() && peek().value().type == TokenType::tok_plus) {
			auto bin_expr_add = m_allocator.alloc<node::BinExprAdd>();
			bin_expr_add->left = left.value();
			consume();

			if (auto right = parse_expr()) {
				bin_expr_add->right = right.value();
				bin_expr->expr = bin_expr_add;

				return bin_expr;
			}
			
			std::cerr << "Expected expression after '+'" << std::endl;
			exit(EXIT_SUCCESS);
		}

		std::cerr << "Unsupported binary operator (Implementing soon)" << std::endl;
		exit(EXIT_SUCCESS);
	}

	return {};
}

std::optional<node::Statement*> lgn::Parser::parse_stmt()
{
	if (peek().has_value() && peek().value().type == TokenType::tok_kw) {
		if (peek().value().value == "exit") {
			consume();

			auto stmt_exit = m_allocator.alloc<node::StatementExit>();

			try_consume(TokenType::tok_lparen, "Expected '('");

			if (auto node_expr = parse_expr()) {
				stmt_exit->expr = node_expr.value();
			} else {
				std::cerr << "Expected expression" << std::endl;
				exit(EXIT_SUCCESS);
			}

			try_consume(TokenType::tok_rparen, "Expected ')'");
			try_consume(TokenType::tok_semi, "Expected ';' at end-of-line");

			auto stmt = m_allocator.alloc<node::Statement>();
			stmt->statement = stmt_exit;

			return stmt;
		} else if (peek().value().value == "let") {
			consume();

			auto stmt_let = m_allocator.alloc<node::StatementLet>();
			stmt_let->tok_id = try_consume(TokenType::tok_id, "Expected identifier");

			try_consume(TokenType::tok_eq, "Expected identifier after '='");

			if (auto node_expr = parse_expr()) {
				stmt_let->expr = node_expr.value();
			} else {
				std::cerr << "Expected expression" << std::endl;
				exit(EXIT_SUCCESS);
			}

			try_consume(TokenType::tok_semi, "Expected ';' at end-of-line");

			auto stmt = m_allocator.alloc<node::Statement>();
			stmt->statement = stmt_let;

			return stmt;
		} else if (peek().value().value == "if") {
			consume();
			bool open_paren = false;
			auto stmt_if = m_allocator.alloc<node::StatementIf>();

			if (peek().value().type == TokenType::tok_lparen) {
				consume();
				open_paren = true;
			}

			if (auto expr = parse_expr()) {
				stmt_if->expr = expr.value();
			} else {
				std::cerr << "Expected expression" << std::endl;
				exit(EXIT_SUCCESS);
			}

			if (open_paren)
				try_consume(TokenType::tok_rparen, "Expected ')'");

			if (auto scope = parse_scope()) {
				stmt_if->scope = scope.value();
			} else {
				std::cerr << "Expected '{'" << std::endl;
				exit(EXIT_SUCCESS);
			}

			auto stmt = m_allocator.alloc<node::Statement>();
			stmt->statement = stmt_if;

			return stmt;
		} else {
			std::cerr << "Unknown keyword '" << peek().value().value.value() << "'" << std::endl;
			exit(EXIT_SUCCESS);
		}
	} else if (peek().has_value() && peek().value().type == TokenType::tok_lbrace) {
		if (auto scope = parse_scope()) {
			auto stmt = m_allocator.alloc<node::Statement>();
			stmt->statement = scope.value();

			return stmt;
		}

		std::cerr << "Expected '{'" << std::endl;
	}

	return {};
}

std::optional<node::Scope*> lgn::Parser::parse_scope()
{
	if (!try_consume(TokenType::tok_lbrace))
		return {};

	auto scope = m_allocator.alloc<node::Scope>();

	while (auto stmt = parse_stmt()) {
		scope->statements.push_back(stmt.value());
	}

	try_consume(TokenType::tok_rbrace, "Expected '}'");

	return scope;
}

std::optional<Token> Parser::peek(int count)
{
	if (m_idx + count >= m_tks.size())
		return {};

	return m_tks.at(m_idx + count);
}

Token Parser::consume()
{
	return m_tks.at(m_idx++);
}

std::optional<Token> lgn::Parser::try_consume(TokenType type)
{
	if (peek().has_value() && peek().value().type == type)
		return consume();

	return {};
}

Token lgn::Parser::try_consume(TokenType type, const std::string& err)
{
	if (peek().has_value() && peek().value().type == type)
		return consume();

	std::cerr << err << std::endl;
	exit(EXIT_SUCCESS);
}
