#include "lexer.h"

Lexer::Lexer(const std::string& src) : source(src), pos(0), lineNumber(1), columnNumber(1), keywords({"if", "while", "return", "print"}) {}

char Lexer::peek() {
    return pos < source.size() ? source[pos] : '\0';
}

char Lexer::advance() {
    if (peek() == '\n') {
        lineNumber++;
        columnNumber = 1;
    } else {
        columnNumber++;
    }
    return pos < source.size() ? source[pos++] : '\0';
}

Token Lexer::readWhitespace() {
    std::string value;
    while (isspace(peek()) && peek() != '\n' && peek() != '\0')
        value += advance();
    return { TokenType::WHITESPACE, value, lineNumber, columnNumber };
}

Token Lexer::readIdentifier() {
    std::string value;
    while (!isspace(peek()) && peek() != '\0' && std::string("+-*/=><()\"").find(peek()) == std::string::npos)
        value += advance();
    if (keywords.find(value) != keywords.end())
        return { TokenType::KEYWORD, value, lineNumber, columnNumber };
    return { TokenType::IDENTIFIER, value, lineNumber, columnNumber };
}

Token Lexer::readNumber() {
    std::string value;
    while (isdigit(peek()))
        value += advance();
    return { TokenType::NUMBER, value, lineNumber, columnNumber };
}

Token Lexer::readString() {
    advance(); // Skip opening quote
    std::string value;
    while (peek() != '"' && peek() != '\0')
        value += advance();
    advance(); // Skip closing quote
    return { TokenType::STRING, value, lineNumber, columnNumber };
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < source.size()) {
        char current = peek();
        if (current == '\0')
            break;
        else if (current == '\n') {
            tokens.push_back({ TokenType::NEWLINE, "\\n", lineNumber, columnNumber });
            advance();
        }
        else if (isspace(current))
            tokens.push_back(readWhitespace());
        else if (isdigit(current))
            tokens.push_back(readNumber());
        else if (current == '"')
            tokens.push_back(readString());
        else if (std::string("+-*/=><").find(current) != std::string::npos)
            tokens.push_back({ TokenType::OPERATOR, std::string(1, advance()), lineNumber, columnNumber });
        else if (current == '(')
            tokens.push_back({ TokenType::PAREN_OPEN, std::string(1, advance()), lineNumber, columnNumber });
        else if (current == ')')
            tokens.push_back({ TokenType::PAREN_CLOSE, std::string(1, advance()), lineNumber, columnNumber });
        else if (current == ':')
            tokens.push_back({ TokenType::COLON, std::string(1, advance()), lineNumber, columnNumber });
        else
            tokens.push_back(readIdentifier());
    }

    tokens.push_back({ TokenType::END_OF_FILE, "EOF", lineNumber, columnNumber });
    return tokens;
}
