#include "code_generator.hpp"
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
        ASTNode* rhsNode = assignmentNode->value.get();
        std::string valueCode = generateCodeForNode(rhsNode)[0];
        std::string resolvedVarType;

        if (rhsNode->type == NodeType::IDENTIFIER) {
            auto rhsIdentifierNode = dynamic_cast<IdentifierNode*>(rhsNode);
            if (variableTypes.count(rhsIdentifierNode->name)) {
                resolvedVarType = variableTypes[rhsIdentifierNode->name];
            } else {
                // Attempting to use an undeclared variable on the RHS.
                // This should ideally be an error caught earlier.
                std::cerr << "[warn]: Variable '" << rhsIdentifierNode->name << "' used on RHS of assignment to '" << varName << "' has unknown type. Defaulting to int." << std::endl;
                resolvedVarType = "int"; // Defaulting, but this is risky.
            }
        } else {
            resolvedVarType = inferType(rhsNode);
        }

        if (resolvedVarType.empty() || resolvedVarType == "void") {
             std::cerr << "[warn]: Could not reliably infer C type for RHS of assignment to '" << varName << "'. Defaulting to int." << std::endl;
             resolvedVarType = "int"; // Fallback type
        }

        variableTypes[varName] = resolvedVarType; // Store/update variable's C type

        if (declaredVariables.find(varName) == declaredVariables.end()) {
             code.push_back(resolvedVarType + " " + varName + " = " + valueCode + ";");
             declaredVariables.insert(varName);
        } else {
             code.push_back(varName + " = " + valueCode + ";"); // Re-assignment
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
                std::string cTypeToPrint;

                switch (nextNode->type) {
                    case NodeType::STRING:
                        cTypeToPrint = "char*";
                        break;
                    case NodeType::NUMBER:
                        cTypeToPrint = "int";
                        break;
                    case NodeType::BINARY_OP: // Assuming binary operations result in int
                        cTypeToPrint = "int";
                        break;
                    case NodeType::IDENTIFIER: {
                        auto idNode = dynamic_cast<IdentifierNode*>(nextNode);
                        if (variableTypes.count(idNode->name)) {
                            cTypeToPrint = variableTypes[idNode->name];
                        } else {
                            std::cerr << "[warn]: Printing undeclared variable '" << idNode->name << "'. Type unknown, cannot generate print statement." << std::endl;
                            cTypeToPrint = "unknown_type"; // Mark as unknown
                        }
                        break;
                    }
                    default:
                        std::cerr << "[warn]: Attempting to print an unsupported AST node type: " << nextNode->getType() << ". Cannot generate print statement." << std::endl;
                        cTypeToPrint = "unsupported_type"; // Mark as unsupported
                        break;
                }

                std::string formatSpecifier;
                if (cTypeToPrint == "int") {
                    formatSpecifier = "%d";
                } else if (cTypeToPrint == "char*") {
                    formatSpecifier = "%s";
                } else {
                    if (cTypeToPrint != "unknown_type" && cTypeToPrint != "unsupported_type") {
                        // This case means a known variable has a C type we don't explicitly handle for printing yet.
                        std::cerr << "[warn]: Variable '" << printArg << "' has C type '" << cTypeToPrint << "' which may not print correctly with default format. Defaulting to %s." << std::endl;
                        formatSpecifier = "%s"; // Default for other C types, might be incorrect.
                    }
                    // If type is "unknown_type" or "unsupported_type", formatSpecifier remains empty, and no printf is generated.
                }

                if (!formatSpecifier.empty()) {
                    code.push_back("printf(\"" + formatSpecifier + "\\n\", " + printArg + ");");
                }
                advance(); // Skip the next node since it's already processed
            } else {
                std::cerr << "[warn]: Missing argument for print statement" << std::endl;
            }
        } else {
            std::cerr << "[warn]: Unsupported keyword: " << keywordNode->name << std::endl;
        }
    } else {
        std::cerr << "[warn]: Unknown AST node type" << std::endl;
    }

    return code;
}
