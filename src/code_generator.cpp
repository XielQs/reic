#include "code_generator.h"
#include <iostream>
#include <string>

CodeGenerator::CodeGenerator(std::vector<std::unique_ptr<ASTNode>>&& nodes) : currentIndex(0), nodes(std::move(nodes)) {}

std::vector<std::string> CodeGenerator::generateCode() {
    std::vector<std::string> generatedCode;
    std::vector<std::string> processedCode;

    //* process all nodes
    for (currentIndex = 0; currentIndex < nodes.size(); ++currentIndex) {
        auto nodeCode = generateCodeForNode(nodes[currentIndex].get());
        processedCode.insert(processedCode.end(), nodeCode.begin(), nodeCode.end());
    }

    //* include headers
    for (auto& include : includes) {
        generatedCode.push_back("#include " + include);
    }

    if (!includes.empty()) {
        generatedCode.push_back("");
    }

    generatedCode.push_back("int main() {");

    //* add processed code
    for (const auto& line : processedCode) {
        generatedCode.push_back("    " + line);
    }

    generatedCode.push_back("    return 0;");
    generatedCode.push_back("}");

    return generatedCode;
}

std::string CodeGenerator::inferType(ASTNode* node) {
    switch (node->type) {
        case NodeType::STRING:
            return "char*";
        case NodeType::BINARY_OP:
        case NodeType::NUMBER:
            return "int";
        case NodeType::IDENTIFIER:
        case NodeType::ASSIGNMENT:
            return "";
        default:
            return "void";
    }
}

ASTNode* CodeGenerator::peek(int offset = 0) {
    size_t index = currentIndex + offset;

    if (index < nodes.size()) {
        return nodes[index].get();
    }
    return nullptr;
}

ASTNode* CodeGenerator::advance() {
    return currentIndex < nodes.size() ? nodes[currentIndex++].get() : nullptr;
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
        if (!binaryOpNode->left || !binaryOpNode->right) {
            std::cerr << "[warn]: Binary operation with null operand" << std::endl;
            return code;
        }
        std::string leftCode = generateCodeForNode(binaryOpNode->left.get())[0];
        std::string rightCode = generateCodeForNode(binaryOpNode->right.get())[0];
        code.push_back(leftCode + " " + binaryOpNode->op + " " + rightCode);
    } else if (auto identifierNode = dynamic_cast<IdentifierNode*>(node)) {
        code.push_back(identifierNode->name);
    } else if (auto keywordNode = dynamic_cast<KeywordNode*>(node)) {
        if (keywordNode->name == "print") {
            if (std::find(includes.begin(), includes.end(), "<stdio.h>") == includes.end()) {
                includes.push_back("<stdio.h>");
            }
            ASTNode* nextNode = peek(1);
            if (nextNode) {
                std::string printArg = generateCodeForNode(nextNode)[0];
                // todo: handle different types
                code.push_back("printf(\"%s\\n\", " + printArg + ");");
                advance(); // Skip the next node since it's already processed
            } else {
                std::cerr << "[warn]: Missing argument for print statement" << std::endl;
            }
        } else {
            std::cerr << "[warn]: Unknown keyword: " << keywordNode->name << std::endl;
        }
    } else {
        std::cerr << "[warn]: Unknown AST node type" << std::endl;
    }

    return code;
}
