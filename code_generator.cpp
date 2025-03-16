#include "code_generator.h"
#include <iostream>
#include <string>

void CodeGenerator::processPart(const std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    auto nodeCode = generateCodeForNode(node.get());
    code.insert(code.end(), nodeCode.begin(), nodeCode.end());
}

std::vector<std::string> CodeGenerator::generateCode() {
    auto mainFunctionCode = "int main() {";
    auto endFunctionCode = "return 0;\n}";
    includes.insert(includes.begin(), "<stdio.h>");
    std::vector<std::string> generatedCode;
    for (auto& include : includes) {
        generatedCode.push_back("#include " + include);
    }
    generatedCode.push_back(mainFunctionCode);
    for (const auto& line : code) {
        generatedCode.push_back(line);
    }
    generatedCode.push_back(endFunctionCode);
    return generatedCode;
}

std::string CodeGenerator::inferType(ASTNode* node) {
    switch (node->type) {
        case NodeType::STRING:
            return "char*";
        case NodeType::NUMBER:
            return "int";
        case NodeType::IDENTIFIER:
        case NodeType::BINARY_OP:
        case NodeType::ASSIGNMENT:
            return "";
        default:
            return "void";
    }
}

std::vector<std::string> CodeGenerator::generateCodeForNode(ASTNode* node) {
    std::vector<std::string> code;

    if (auto assignmentNode = dynamic_cast<AssignmentNode*>(node)) {
        std::string varName = assignmentNode->variable;
        std::string valueCode = generateCodeForNode(assignmentNode->value.get())[0];
        std::string varType = inferType(assignmentNode->value.get());
        if (varType.empty()) {
            code.push_back(varName + " = " + valueCode + ";");
        } else {
            code.push_back(varType + " " + varName + " = " + valueCode + ";");
        }
    } else if (auto stringNode = dynamic_cast<StringNode*>(node)) {
        code.push_back("\"" + stringNode->value + "\"");
    } else if (auto numberNode = dynamic_cast<NumberNode*>(node)) {
        code.push_back(std::to_string(numberNode->value));
    } else if (auto binaryOpNode = dynamic_cast<BinaryOpNode*>(node)) {
        std::string leftCode = generateCodeForNode(binaryOpNode->left.get())[0];
        std::string rightCode = generateCodeForNode(binaryOpNode->right.get())[0];
        code.push_back(leftCode + " " + binaryOpNode->op + " " + rightCode);
    } else if (auto identifierNode = dynamic_cast<IdentifierNode*>(node)) {
        code.push_back(identifierNode->name);
    } else if (auto keywordNode = dynamic_cast<KeywordNode*>(node)) {
        code.push_back(keywordNode->name);
    } else {
        std::cerr << "[warn]: Unknown AST node type" << std::endl;
    }

    return code;
}
