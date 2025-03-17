#include "parser.h"
#include <iostream>
#include <string>

Parser::Parser(const std::vector<Token>& tokens, const std::string& fileName, const std::string& content)
    : tokens(tokens), currentFileName(fileName), fileContent(content), pos(0) {}

Token Parser::peek(int offset = 0, bool skipWhitespace = true) {
    if (skipWhitespace) {
        while (pos + offset < tokens.size() && tokens[pos + offset].type == TokenType::WHITESPACE) {
            offset++;
        }
    }
    int index = pos + offset;

    if (index < 0 || index >= static_cast<int>(tokens.size())) {
        return Token{ TokenType::END_OF_FILE, "EOF", 0, 0 };
    }

    return tokens[index];
}

Token Parser::advance(bool skipWhitespace = true) {
    if (skipWhitespace) {
        while (pos < tokens.size() && tokens[pos].type == TokenType::WHITESPACE) {
            pos++;
        }
    }
    return pos < tokens.size() ? tokens[pos++] : Token{ TokenType::END_OF_FILE, "EOF", 0, 0 };
}

void Parser::handleSyntaxError(const Token& current) {
    std::vector<std::string> lines;
    size_t start = 0;
    size_t end = fileContent.find('\n');
    while (end != std::string::npos) {
        lines.push_back(fileContent.substr(start, end - start));
        start = end + 1;
        end = fileContent.find('\n', start);
    }
    lines.push_back(fileContent.substr(start));

    std::string line = lines[current.lineNumber - 1];

    size_t lineStart = line.rfind('\n', current.columnNumber - 2);
    if (lineStart == std::string::npos || lineStart < 1)
        lineStart = -1;

    size_t lineEnd = line.find('\n', current.columnNumber);
    if (lineEnd == std::string::npos)
        lineEnd = line.length();

    size_t charCountUntilSpace = 0;

    for (size_t i = current.columnNumber - 2; i < lineEnd; ++i) {
        if (line[i] == ' ')
            break;
        charCountUntilSpace++;
    }

    std::cerr << currentFileName << ":" << current.lineNumber << std::endl;
    std::cerr << line << std::endl;
    size_t spaceCount = current.columnNumber - lineStart - 1;
    if (spaceCount >= 2)
        spaceCount -= 2;
    std::cerr << std::string(spaceCount, ' ')
              << std::string(charCountUntilSpace, '^') << std::endl;

    std::cerr << "[error]: Unexpected " << current.humanize() << std::endl;
    if (current.type == TokenType::PAREN_OPEN && peek(-2).type == TokenType::KEYWORD)
        std::cerr << "[info]: The \"" << peek(-2).value << "\" keyword should be used WITHOUT parentheses." << std::endl;

    exit(1);
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token current = advance();
    if (current.type == TokenType::NUMBER)
        return std::make_unique<NumberNode>(std::stoi(current.value));
    else if (current.type == TokenType::STRING)
        return std::make_unique<StringNode>(current.value);
    else if (current.type == TokenType::IDENTIFIER)
        return std::make_unique<IdentifierNode>(current.value);
    else if (current.type == TokenType::NEWLINE || current.type == TokenType::END_OF_FILE || current.type == TokenType::WHITESPACE)
        return nullptr;
    else if (current.type == TokenType::KEYWORD) {
        // todo: maybe add support for keywords like if and while
        if (peek(0, false).type != TokenType::WHITESPACE) {
            handleSyntaxError(current);
            return nullptr; //! Unreachable
        }
        return std::make_unique<KeywordNode>(current.value);
    } else if (current.type == TokenType::PAREN_OPEN) {
        auto node = parseExpression();
        if (peek().type == TokenType::PAREN_CLOSE) {
            advance(); // consume the closing parenthesis
            return node;
        } else {
            handleSyntaxError(current);
            return nullptr; //! Unreachable
        }
    } else if (current.type == TokenType::PAREN_CLOSE) {
        handleSyntaxError(current);
        return nullptr; //! Unreachable
    } else {
        handleSyntaxError(current);
        return nullptr; //! Unreachable
    }
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto node = parseFactor();
    while (peek().value == "*" || peek().value == "/") {
        std::string op = advance().value;
        auto right = parseFactor();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto node = parseTerm();
    while (peek().value == "+" || peek().value == "-") {
        std::string op = advance().value;
        auto right = parseTerm();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

// parse assignments like x = 10 + 20
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    if (peek().type == TokenType::IDENTIFIER && peek(1).value == "=") {
        std::string varName = advance().value;
        advance();
        auto value = parseExpression();
        if (!value) { 
            std::cerr << "[warn]: Assignment value is null at line " << peek().lineNumber << std::endl;
            return nullptr;
        }
        return std::make_unique<AssignmentNode>(varName, std::move(value));
    }
    return parseExpression();
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (pos < tokens.size()) {
        auto node = parseAssignment();
        if (node) {
            statements.push_back(std::move(node));
        }
        while (peek().value == "\n" || peek().value == ";")
            advance();
    }
    return statements;
}
