#pragma once
#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>

class Parser {
public:
    Parser(const std::vector<Token>& tokens, const std::string& fileName, const std::string& content);

    std::vector<std::unique_ptr<ASTNode>> parse();

private:
    Token peek(int offset, bool skipWhitespace);
    Token advance(bool skipWhitespace);
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parseAssignment();
    void handleSyntaxError(const Token& current);

    std::vector<Token> tokens;
    std::string currentFileName;
    std::string fileContent;
    size_t pos;
};

#endif // PARSER_HPP
