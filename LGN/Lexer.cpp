#include "Lexer.h"
using namespace lgn;

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    std::string buf;

    while (peek().has_value()) {
        if (std::isalpha(peek().value())) {
            buf.push_back(consume());

            while (peek().has_value() && std::isalnum(peek().value())) {
                buf.push_back(consume());
            }

            if (buf == "exit") {
                tokens.push_back({ .type = TokenType::tok_kw, .value = "exit"});
                buf.clear();
                continue;
            } else if (buf == "let") {
                tokens.push_back({ .type = TokenType::tok_kw, .value = "let"});
                buf.clear();
                continue;
            }

            tokens.push_back({ .type = TokenType::tok_id, .value = buf });
            buf.clear();
        } else if (std::isdigit(peek().value())) {
            buf.push_back(consume());

            while (peek().has_value() && std::isdigit(peek().value())) {
                buf.push_back(consume());
            }

            tokens.push_back({ .type = TokenType::tok_int, .value = buf });
            buf.clear();
        } else if (peek().value() == '(') {
            consume();
            tokens.push_back({ .type = TokenType::tok_lparen });
        } else if (peek().value() == ')') {
            consume();
            tokens.push_back({ .type = TokenType::tok_rparen });
        } else if (peek().value() == ';') {
            consume();
            tokens.push_back({ .type = TokenType::tok_semi });
        } else if (std::isspace(peek().value())) {
            consume();
        } else if (peek().value() == '=') {
            consume();
            tokens.push_back({ .type = TokenType::tok_eq });
        } else if (peek().value() == '+') {
            consume();
            tokens.push_back({ .type = TokenType::tok_plus });
        }
        else if (peek().value() == '#') {
            consume();

            while (peek().value() != '\n') {
                consume();
            }
        } else {
            std::cerr << "Invalid character: ' " << peek().value() << std::endl;
            exit(EXIT_SUCCESS);
        }
    }

    m_idx = 0;
    return tokens;
}

std::optional<char> Lexer::peek(int count) const
{
    if (m_idx + count >= m_src.length())
        return {};

    return m_src.at(m_idx + count);
}

char Lexer::consume()
{
    return m_src.at(m_idx++);
}
