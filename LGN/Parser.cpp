#include "Parser.h"
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
	}

	return {};
}

std::optional<node::Expr*> Parser::parse_expr()
{
	if (auto term = parse_term()) {
		if (try_consume(TokenType::tok_plus).has_value()) {
			auto bin_expr = m_allocator.alloc<node::BinExpr>();
			auto bin_expr_add = m_allocator.alloc<node::BinExprAdd>();
			auto left = m_allocator.alloc<node::Expr>();

			left->expr = term.value();
			bin_expr_add->left = left;

			if (auto right = parse_expr()) {
				bin_expr_add->right = right.value();
				bin_expr->expr = bin_expr_add;

				auto expr = m_allocator.alloc<node::Expr>();
				expr->expr = bin_expr;

				return expr;
			}
			
			std::cerr << "Expected expression after '+'" << std::endl;
			exit(EXIT_SUCCESS);
		} else {
			auto expr = m_allocator.alloc<node::Expr>();
			expr->expr = term.value();

			return expr;
		}
	}

	return {};
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
	while (peek().has_value()) {
		if (peek().value().type == TokenType::tok_kw) {
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
				}

				try_consume(TokenType::tok_semi, "Expected ';' at end-of-line");

				auto stmt = m_allocator.alloc<node::Statement>();
				stmt->statement = stmt_let;

				return stmt;
			}
		} else {
			std::cerr << "Unkown keyword '" << peek().value().value.value() << "'" << std::endl;
			exit(EXIT_SUCCESS);
		}
	}

	m_idx = 0;
	return {};
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