#pragma once
#include "Token.h"
#include <iostream>
#include <vector>

namespace lgn
{
	class Lexer {
	public:
		Lexer(const std::string& src) : m_src(src) {}

		std::vector<Token> tokenize();
	private:
		const std::string m_src;
		size_t m_idx = 0;

		std::optional<char> peek(int count = 0) const;
		char consume();
	};
}