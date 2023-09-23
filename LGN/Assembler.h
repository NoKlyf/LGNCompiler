#pragma once
#include "Node.h"
#include <sstream>
#include <variant>
#include <iostream>

namespace lgn
{
	class Assembler
	{
	public:
		Assembler(const node::Program& prog) : m_prog(prog) {}

		std::string assemble();
		void assemble_statement(const node::Statement *stmt);
		void assemble_term(const node::Term *term);
		void assemble_expr(const node::Expr *expr);
		void assemble_bin_expr(const node::BinExpr* expr);

	private:
		struct Var {
			std::string name;
			size_t sp;
		};

		const node::Program m_prog;
		std::stringstream m_output;

		size_t m_ssize = 0;
		int m_label_count = 0;

		std::vector<Var> m_vars {};
		std::vector<size_t> m_scopes {};

		bool need_exit = true;

		void push(const std::string& reg);
		void pop(const std::string& reg);

		void create_scope(const node::Scope* scope);
		void begin_scope();
		void end_scope();

		std::string create_label();
	};
}
