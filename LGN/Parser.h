#pragma once
#include "Node.h"
#include "ArenaAllocator.h"
#include <vector>
#include <iostream>

namespace lgn
{
	class Parser
	{
	public:
		Parser(const std::vector<Token>& tokens) : m_tks(tokens), m_allocator(1024 * 1024 * 4) {}

		std::optional<node::Program> parse();
		std::optional<node::Term*> parse_term();
		std::optional<node::Expr*> parse_expr();
		std::optional<node::BinExpr*> parse_bin_expr();
		std::optional<node::Statement*> parse_stmt();
	private:
		const std::vector<Token> m_tks;
		size_t m_idx = 0;
		memory::ArenaAllocator m_allocator;

		std::optional<Token> peek(int count = 0);
		Token consume();
		std::optional<Token> try_consume(TokenType type);
		Token try_consume(TokenType type, const std::string& err);
	};
}