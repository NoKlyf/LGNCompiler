#pragma once
#include <optional>
#include <string>

enum class TokenType {
    tok_kw,
    tok_int,
    tok_semi,
    tok_lparen,
    tok_rparen,
    tok_lbrace,
    tok_rbrace,
    tok_id,
    tok_eq,
    tok_plus,
    tok_min,
    tok_mul,
    tok_div,
    tok_mod
};

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};
