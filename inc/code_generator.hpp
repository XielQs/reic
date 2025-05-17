#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <vector>
#include <string>
#include <memory>
#include "ast.hpp"
#include <map>
#include <set>

class CodeGenerator {
public:
    CodeGenerator(std::vector<std::unique_ptr<ASTNode>>&& nodes);
    std::vector<std::string> generateCode();

private:
    size_t currentIndex;
    std::vector<std::string> includes;
    std::vector<std::string> code;
    std::vector<std::unique_ptr<ASTNode>> nodes;
    std::map<std::string, std::string> variableTypes;
    std::set<std::string> declaredVariables;

    std::string inferType(ASTNode* node);
    ASTNode* peek(int offset);
    ASTNode* advance();
    std::vector<std::string> generateCodeForNode(ASTNode* node);
};

#endif // CODE_GENERATOR_H
