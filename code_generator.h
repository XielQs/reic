#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <vector>
#include <string>
#include <memory>
#include "ast.h"

class CodeGenerator {
private:
    std::vector<std::string> generateCodeForNode(ASTNode* node);
    std::vector<std::string> code;
    std::vector<std::string> includes;
    std::string inferType(ASTNode* node);

public:
    void processPart(const std::unique_ptr<ASTNode>& nodes);
    std::vector<std::string> generateCode();

};

#endif // CODE_GENERATOR_H