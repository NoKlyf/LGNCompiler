#include "Assembler.h"
#include <format>
using namespace lgn;

std::string Assembler::assemble()
{
	m_output << "global _start\n_start:\n";

	for (const node::Statement *stmt : m_prog.statements)
		assemble_statement(stmt);

	if (need_exit) {
		m_output << "    mov rax, 60\n";
		m_output << "    mov rdi, 0\n";
		m_output << "    syscall";
	}

	return m_output.str();
}

void Assembler::assemble_statement(const node::Statement* stmt)
{
	struct StmtVisitor {
		Assembler& assembler;

		void operator()(const node::StatementExit* stmt_exit) const
		{
			assembler.assemble_expr(stmt_exit->expr);
					 
			assembler.m_output << "    mov rax, 60\n";
			assembler.pop("rdi");
			assembler.m_output << "    syscall\n";
					 
			assembler.need_exit = false;
		}

		void operator()(const node::StatementLet *stmt_let) const
		{
			std::string var_name = stmt_let->tok_id.value.value();
			auto iterator = std::find_if(assembler.m_vars.cbegin(), assembler.m_vars.cend(), [&](const Var& var) {
				return var.name == var_name;
			});

			if (iterator != assembler.m_vars.cend()) {
				std::cerr << "Identifier '" << var_name << "' is already declared" << std::endl;
				exit(EXIT_SUCCESS);
			}
			
			assembler.m_vars.push_back({ .name = var_name, .sp = assembler.m_ssize });
			assembler.assemble_expr(stmt_let->expr);
		}

		void operator()(const node::StatementIf* stmt_if) const
		{
			std::string label = assembler.create_label();

			assembler.assemble_expr(stmt_if->expr);
			assembler.pop("rax");
			assembler.m_output << "    test rax, rax\n";
			assembler.m_output << "    jz " << label << "\n";
			assembler.create_scope(stmt_if->scope);
			assembler.m_output << "\n" << label << ":\n";
		}

		void operator()(const node::Scope* scope) const
		{
			assembler.create_scope(scope);
		}
	};

	StmtVisitor visitor{ .assembler = *this };
	std::visit(visitor, stmt->statement);
}

void Assembler::assemble_term(const node::Term* term)
{
	struct TermVisitor {
		Assembler& assembler;

		void operator()(const node::TermInt* term_int) const
		{
			//assembler.m_output << "    mov rax, " << term_int->tok_int.value.value() << "\n";
			assembler.push(term_int->tok_int.value.value());
		}

		void operator()(const node::TermId* term_id) const
		{
			std::string var_name = term_id->tok_id.value.value();
			auto iterator = std::find_if(assembler.m_vars.cbegin(), assembler.m_vars.cend(), [&](const Var& var) {
				return var.name == var_name;
			});

			if (iterator == assembler.m_vars.cend()) {
				std::cerr << "Undeclared identifier '" << var_name << "'" << std::endl;
				exit(EXIT_SUCCESS);
			}

			if ((assembler.m_ssize - (*iterator).sp - 1) * 8 > 0) {
				std::stringstream offset;

				offset << "qword [rsp + " << (assembler.m_ssize - (*iterator).sp - 1) * 8 << "]";
				assembler.push(offset.str());
			}
		}

		void operator()(const node::TermParen* term_paren) const
		{
			assembler.assemble_expr(term_paren->expr);
		}
	};

	TermVisitor visitor{ .assembler = *this };
	std::visit(visitor, term->term);
}

void Assembler::assemble_bin_expr(const node::BinExpr* bin_expr)
{
	struct BinExprVisitor
	{
		Assembler& assembler;

		void operator()(const node::BinExprAdd* add) const
		{
			assembler.assemble_expr(add->right);
			assembler.assemble_expr(add->left);
			assembler.pop("rax");
			assembler.pop("rbx");
			assembler.m_output << "    add rax, rbx\n";
			assembler.push("rax");
		}

		void operator()(const node::BinExprSub* sub) const
		{
			assembler.assemble_expr(sub->right);
			assembler.assemble_expr(sub->left);
			assembler.pop("rax");
			assembler.pop("rbx");
			assembler.m_output << "    sub rax, rbx\n";
			assembler.push("rax");
		}

		void operator()(const node::BinExprMul* mul) const
		{
			assembler.assemble_expr(mul->right);
			assembler.assemble_expr(mul->left);
			assembler.pop("rax");
			assembler.pop("rbx");
			assembler.m_output << "    mul rbx\n";
			assembler.push("rax");
		}

		void operator()(const node::BinExprDiv* div) const
		{
			assembler.assemble_expr(div->right);
			assembler.assemble_expr(div->left);
			assembler.pop("rax");
			assembler.pop("rbx");
			assembler.m_output << "    div rbx\n";
			assembler.push("rax");
		}
	};

	BinExprVisitor visitor{ .assembler = *this };
	std::visit(visitor, bin_expr->expr);
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
			assembler->assemble_bin_expr(bin_expr);
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

void Assembler::create_scope(const node::Scope* scope)
{
	begin_scope();

	for (const node::Statement* stmt : scope->statements)
		assemble_statement(stmt);

	end_scope();
}

void Assembler::begin_scope()
{
	m_scopes.push_back(m_vars.size());
}

void Assembler::end_scope()
{
	size_t vars_count = m_vars.size() - m_scopes.back();

	m_output << "    add rsp, " << vars_count * 8 << "\n";

	for (size_t i = 0; i < vars_count; i++)
		m_vars.pop_back();

	m_scopes.pop_back();
}

std::string Assembler::create_label()
{
	return std::format("label_{}", m_label_count++);
}
