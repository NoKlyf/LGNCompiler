#include "Assembler.h"
using namespace lgn;

std::string Assembler::assemble()
{
	m_output << "global _start\n_start:\n";

	for (const node::Statement *stmt : m_prog.statements) {
		assemble_statement(stmt);
	}

	if (need_exit) {
		m_output << "    mov rax, 60\n";
		m_output << "    mov rdi, 0\n";
		m_output << "    syscall";
	}

	return m_output.str();
}

void Assembler::assemble_statement(const node::Statement *stmt)
{
	struct StmtVisitor {
		Assembler *assembler;

		void operator()(const node::StatementExit *stmt_exit) const
		{
			assembler->assemble_expr(stmt_exit->expr);

			assembler->m_output << "    mov rax, 60\n";
			assembler->pop("rdi");
			assembler->m_output << "    syscall\n";

			assembler->need_exit = false;
		}

		void operator()(const node::StatementLet *stmt_let)
		{
			std::string var_name = stmt_let->tok_id.value.value();

			if (assembler->m_vars.contains(var_name)) {
				std::cerr << "Identifier '" << var_name << "' is already declared" << std::endl;
				exit(EXIT_SUCCESS);
			}
			
			assembler->m_vars.insert({ var_name, Var{ .sp = assembler->m_ssize } });
			assembler->assemble_expr(stmt_let->expr);
		}
	};

	StmtVisitor visitor{ .assembler = this };
	std::visit(visitor, stmt->statement);
}

void Assembler::assemble_term(const node::Term* term)
{
	struct TermVisitor {
		Assembler* assembler;

		void operator()(const node::TermInt* term_int) const
		{
			assembler->m_output << "    mov rax, " << term_int->tok_int.value.value() << "\n";
			assembler->push("rax");
		}

		void operator()(const node::TermId* term_id) const
		{
			std::string var_name = term_id->tok_id.value.value();

			if (!assembler->m_vars.contains(var_name)) {
				std::cerr << "Undeclared identifier '" << var_name << "'" << std::endl;
				exit(EXIT_SUCCESS);
			}

			const Var& var = assembler->m_vars.at(var_name);

			if ((assembler->m_ssize - var.sp - 1) * 8 > 0) {
				std::stringstream offset;

				offset << "qword [rsp + " << (assembler->m_ssize - var.sp - 1) * 8 << "]";
				assembler->push(offset.str());
			}
		}
	};

	TermVisitor visitor{ .assembler = this };
	std::visit(visitor, term->term);
}

void Assembler::assemble_expr(const node::Expr *expr)
{
	struct ExprVisitor {
		Assembler *assembler;

		void operator()(const node::Term *term)
		{
			assembler->assemble_term(term);
		}

		void operator()(const node::BinExpr* bin_expr) const
		{
			assembler->assemble_expr(bin_expr->expr->left);
			assembler->assemble_expr(bin_expr->expr->right);
			assembler->pop("rax");
			assembler->pop("rbx");
			assembler->m_output << "    add rax, rbx\n";
			assembler->push("rax");
		}
	};

	ExprVisitor visitor{ .assembler = this };
	std::visit(visitor, expr->expr);
}

void Assembler::push(const std::string& reg)
{
	m_output << "    push " << reg << "\n";
	m_ssize++;
}

void Assembler::pop(const std::string& reg)
{
	m_output << "    pop " << reg << "\n";
	m_ssize--;
}