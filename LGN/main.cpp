#include <fstream>
#include "Lexer.h"
#include "Parser.h"
#include "Assembler.h"
#include "Assembler_Win32.h"

#define MODE_ELF64 1
#define MODE_WIN32 2

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: lgn <input> ([win32 - elf64])" << std::endl;
        return EXIT_FAILURE;
    }

    int mode = MODE_ELF64;

    if (argv[2]) {
        if (std::strcmp(argv[2], "win32") == 0)
            mode = MODE_WIN32;
        else if (std::strcmp(argv[2], "elf64") == 0)
            mode = MODE_ELF64;
        else {
            std::cerr << "Unsupported mode: '" << argv[2] << "'" << std::endl;
            std::cerr << "Supported modes: win32, elf64" << std::endl;
            return EXIT_FAILURE;
        }
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

    if (mode == MODE_ELF64) {
        lgn::Assembler assembler(ast.value());
        {
            std::fstream output("out.asm", std::ios::out);
            output << assembler.assemble();
        }

        system("nasm -felf64 out.asm -o out.o");
        system("ld out.o -o out.exe");

        return EXIT_SUCCESS;
    } else if (mode == MODE_WIN32) {
        lgn::Assembler_Win32 assembler(ast.value());
        {
            std::fstream output("out.asm", std::ios::out);
            output << assembler.assemble();
        }

        system("nasm -fwin32 out.asm -o out.o");
        system("ld -m i386pe out.o -o out.exe");
    }

    return EXIT_SUCCESS;
}