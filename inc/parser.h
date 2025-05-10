#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

class Parser {
private:
    std::vector<Token> tokens;
    std::string currentFileName;
    std::string fileContent;
    size_t pos;

    Token peek(int offset, bool skipWhitespace);
    Token advance(bool skipWhitespace);
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parseAssignment();
    void handleSyntaxError(const Token& current);

public:
    Parser(const std::vector<Token>& tokens, const std::string& fileName, const std::string& content);
    std::vector<std::unique_ptr<ASTNode>> parse();
};

#endif // PARSER_H
