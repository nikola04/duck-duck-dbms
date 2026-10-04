#pragma once

#include <cstddef>
#include <string>

namespace duck::sql {

enum class TokenType {
    END,
    SELECT,
    FROM,
    JOIN,
    ON,
    WHERE,
    LIMIT,
    AND,
    OR,
    NOT,
    IDENTIFIER,
    INTEGER,
    STRING,
    STAR,
    COMMA,
    DOT,
    LEFT_PAREN,
    RIGHT_PAREN,
    EQUAL,
    NOT_EQUAL,
    GREATER,
    GREATER_EQUAL
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t position;
};

} // namespace duck::sql