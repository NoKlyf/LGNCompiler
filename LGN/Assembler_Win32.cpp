#include "Assembler_Win32.h"

#include "Assembler.h"
using namespace lgn;

std::string Assembler_Win32::assemble()
{
	m_output << "global _start\n\nsection .text\n_start:\n";

	for (const node::Statement* stmt : m_prog.statements) {
		assemble_statement(stmt);
	}

	if (need_exit) {
		m_output << "    mov eax, 1\n";
		m_output << "    mov ebx, 0\n";
		m_output << "    int 80h";
	}

	return m_output.str();
}

void Assembler_Win32::assemble_statement(const node::Statement* stmt)
{
	struct StmtVisitor {
		Assembler_Win32* assembler;

		void operator()(const node::StatementExit* stmt_exit) const
		{
			assembler->assemble_expr(stmt_exit->expr);

			assembler->m_output << "    mov eax, 1\n";
			assembler->pop("ebx");
			assembler->m_output << "    int 80h\n";

			assembler->need_exit = false;
		}

		void operator()(const node::StatementLet* stmt_let)
		{
			std::string var_name = stmt_let->tok_id.value.value();

			if (assembler->m_vars.contains(var_name)) {
				std::cerr << "Identifier '" << var_name << "' is already declared" << std::endl;
				exit(EXIT_SUCCESS);
			}

			assembler->m_vars.insert({ var_name, Var{.sp = assembler->m_ssize } });
			assembler->assemble_expr(stmt_let->expr);
		}
	};

	StmtVisitor visitor{ .assembler = this };
	std::visit(visitor, stmt->statement);
}

void Assembler_Win32::assemble_term(const node::Term* term)
{
	struct TermVisitor {
		Assembler_Win32* assembler;

		void operator()(const node::TermInt* term_int) const
		{
			assembler->m_output << "    mov eax, " << term_int->tok_int.value.value() << "\n";
			assembler->push("eax");
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

				offset << "dword [esp + " << (assembler->m_ssize - var.sp - 1) * 4 << "]";
				assembler->push(offset.str());
			}
		}
	};

	TermVisitor visitor{ .assembler = this };
	std::visit(visitor, term->term);
}

void Assembler_Win32::assemble_expr(const node::Expr* expr)
{
	struct ExprVisitor {
		Assembler_Win32* assembler;

		void operator()(const node::Term* term)
		{
			assembler->assemble_term(term);
		}

		void operator()(const node::BinExpr* bin_expr) const
		{
			assembler->assemble_expr(bin_expr->expr->left);
			assembler->assemble_expr(bin_expr->expr->right);
			assembler->pop("eax");
			assembler->pop("ebx");
			assembler->m_output << "    add eax, ebx\n";
			assembler->push("eax");
		}
	};

	ExprVisitor visitor{ .assembler = this };
	std::visit(visitor, expr->expr);
}

void Assembler_Win32::push(const std::string& reg)
{
	m_output << "    push " << reg << "\n";
	m_ssize++;
}

void Assembler_Win32::pop(const std::string& reg)
{
	m_output << "    pop " << reg << "\n";
	m_ssize--;
}