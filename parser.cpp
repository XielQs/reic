#include "parser.h"
#include <iostream>
#include <string>

Parser::Parser(const std::vector<Token>& tokens, const std::string& fileName, const std::string& content)
    : tokens(tokens), currentFileName(fileName), fileContent(content), pos(0) {}

Token Parser::peek(int offset = 0) {
    int index = pos + offset;

    if (index < 0 || index >= static_cast<int>(tokens.size())) {
        return Token{ TokenType::END_OF_FILE, "EOF", 0, 0 };
    }

    return tokens[index];
}


Token Parser::advance() {
    return pos < tokens.size() ? tokens[pos++] : Token{ TokenType::END_OF_FILE, "EOF", 0, 0 };
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token current = advance();
    if (current.type == TokenType::NUMBER) {
        return std::make_unique<NumberNode>(std::stoi(current.value));
    } else if (current.type == TokenType::STRING) {
        return std::make_unique<StringNode>(current.value);
    } else if (current.type == TokenType::IDENTIFIER) {
        return std::make_unique<IdentifierNode>(current.value);
    } else if (current.type == TokenType::NEWLINE || current.type == TokenType::END_OF_FILE) {
        return nullptr;
    } else if (current.type == TokenType::KEYWORD) {
        // todo: maybe add support for keywords like if and while
        return std::make_unique<KeywordNode>(current.value);
    } else {
        //? Handle unexpected tokens
        //* log the entire line
        size_t lineStart = fileContent.rfind('\n', current.columnNumber);
        size_t lineEnd = fileContent.find('\n', current.columnNumber);
        if (lineEnd == std::string::npos) {
            lineEnd = fileContent.length();
        }
        std::string line = fileContent.substr(lineStart + 1, lineEnd - lineStart - 1);
        size_t charCountUntilSpace = 0;
        for (size_t i = current.columnNumber - 2; i < lineEnd; ++i) {
            if (fileContent[i] == ' ') {
                break;
            }
            charCountUntilSpace++;
        }
        std::cerr << currentFileName << ":" << current.lineNumber << std::endl;
        std::cerr << line << std::endl;
        std::cerr << std::string(current.columnNumber - lineStart - 3, ' ') 
                  << std::string(charCountUntilSpace, '^') 
                  << std::endl;

        std::cerr << "[error]: Unexpected " << current.humanize() << std::endl;
        // if prev token is a keyword, warn to not use paranthesis
        if (current.type == TokenType::PAREN_OPEN && peek(-2).type == TokenType::KEYWORD) {
            std::cerr << "[info]: The \"" << peek(-2).value << "\" keyword should be used WITHOUT parentheses." << std::endl;
        }
        exit(1);
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
    if (peek().type == TokenType::IDENTIFIER && tokens[pos + 1].value == "=") {
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
        statements.push_back(parseAssignment());
        while (peek().value == "\n" || peek().value == ";") {
            advance();
        }
    }
    return statements;
}
