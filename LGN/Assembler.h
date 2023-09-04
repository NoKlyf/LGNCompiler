#pragma once
#include "Node.h"
#include <sstream>
#include <variant>
#include <unordered_map>
#include <iostream>

namespace lgn
{
	class Assembler
	{
	public:
		Assembler(const node::Program& prog) : m_prog(prog) {}

		std::string assemble();
		void assemble_statement(const node::Statement *stmt);
		void assemble_term(const node::Term* term);
		void assemble_expr(const node::Expr *expr);

	private:
		struct Var {
			size_t sp;
		};

		const node::Program m_prog;
		std::stringstream m_output;
		size_t m_ssize = 0;
		std::unordered_map<std::string, Var> m_vars {};
		bool need_exit = true;

		void push(const std::string& reg);
		void pop(const std::string& reg);
	};
}
