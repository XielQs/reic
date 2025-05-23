#pragma once
#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <unordered_set>

enum class TokenType {
    IDENTIFIER,
    KEYWORD,
    NUMBER,
    STRING,
    OPERATOR,
    PAREN_OPEN,
    PAREN_CLOSE,
    COLON,
    INDENT,
    NEWLINE,
    END_OF_FILE,
    WHITESPACE
};

struct Token {
    std::string humanize() const {
        switch (type) {
            case TokenType::IDENTIFIER: return "identifier";
            case TokenType::KEYWORD: return "keyword";
            case TokenType::NUMBER: return "number";
            case TokenType::STRING: return "string";
            case TokenType::OPERATOR: return "operator";
            case TokenType::PAREN_OPEN: return "open parenthesis";
            case TokenType::PAREN_CLOSE: return "close parenthesis";
            case TokenType::COLON: return "colon";
            case TokenType::INDENT: return "indent";
            case TokenType::NEWLINE: return "newline";
            case TokenType::WHITESPACE: return "whitespace";
            case TokenType::END_OF_FILE: return "end of file";
            default: return "unknown token";
        }
    }

    TokenType type;
    std::string value;
    size_t lineNumber;
    size_t columnNumber;
};

class Lexer {
public:
    Lexer(const std::string& src);

    std::vector<Token> tokenize();

private:
    char peek();
    char advance();
    Token readIdentifier();
    Token readWhitespace();
    Token readNumber();
    Token readString();

    std::string source;
    size_t pos;
    size_t lineNumber;
    size_t columnNumber;
    std::unordered_set<std::string> keywords;
};

#endif // LEXER_HPP
