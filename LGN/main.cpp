#include <fstream>
#include "Lexer.h"
#include "Parser.h"
#include "Assembler.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: lgn <input>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string content;
    {
        std::stringstream content_stream;
        std::fstream input(argv[1], std::ios::in);

        content_stream << input.rdbuf();
        content = content_stream.str();
    }

    lgn::Lexer lexer(std::move(content));
    std::vector<Token> tokens = lexer.tokenize();

    lgn::Parser parser(std::move(tokens));
    std::optional<lgn::node::Program> ast = parser.parse();

    if (!ast.has_value()) {
        std::cerr << "No statements found" << std::endl;
        return EXIT_SUCCESS;
    }

    lgn::Assembler assembler(ast.value());
    {
        std::fstream output("out.asm", std::ios::out);
        output << assembler.assemble();
    }

    system("nasm -felf64 out.asm -o out.o");
    system("ld out.o -o out.exe");

    return EXIT_SUCCESS;
}
