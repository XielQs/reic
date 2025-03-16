#define EXTENSION_NAME ".reic"

#include "parser.h"
#include "lexer.h"
#include "code_generator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

void try_read_file(std::string &filename, std::string *content) {
    std::ifstream file(filename);
    if (!file) {
        if (!filename.ends_with(EXTENSION_NAME)) {
            filename += EXTENSION_NAME;
            try_read_file(filename, content);
            return;
        }
        std::cerr << "[error]: Error opening file: " << filename << std::endl;
        exit(1);
    }

    std::cout << "File opened successfully." << std::endl;
    std::string line;
    while (getline(file, line)) {
        content->append(line + "\n");
    }

    if (file.bad()) {
        std::cerr << "[error]: Error reading file: " << filename << std::endl;
        exit(1);
    }

    std::cout << "File read successfully." << std::endl;
    file.close();
}


void printAST(const ASTNode* node, int indent = 2) {
    if (!node) return;
    std::string padding(indent, ' ');

    switch (node->type) {
        case NodeType::STRING:
            std::cout << padding << "String: " << dynamic_cast<const StringNode*>(node)->value << std::endl;
            break;
        case NodeType::NUMBER:
            std::cout << padding << "Number: " << dynamic_cast<const NumberNode*>(node)->value << std::endl;
            break;
        case NodeType::IDENTIFIER:
            std::cout << padding << "Identifier: " << dynamic_cast<const IdentifierNode*>(node)->name << std::endl;
            break;
        case NodeType::KEYWORD:
            std::cout << padding << "Keyword: " << dynamic_cast<const KeywordNode*>(node)->name << std::endl;
            break;
        case NodeType::BINARY_OP: {
            auto binaryNode = dynamic_cast<const BinaryOpNode*>(node);
            std::cout << padding << "BinaryOp: " << binaryNode->op << std::endl;
            printAST(binaryNode->left.get(), indent + 2);
            printAST(binaryNode->right.get(), indent + 2);
            break;
        }
        case NodeType::ASSIGNMENT: {
            auto assignNode = dynamic_cast<const AssignmentNode*>(node);
            std::cout << padding << "Assignment: " << assignNode->variable << std::endl;
            printAST(assignNode->value.get(), indent + 2);
            std::cout << padding << "  Value Type: "
                        << (assignNode->value ? assignNode->value->getType() : "null") << std::endl;
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "[error]: Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::cout << "File name: " << filename << std::endl;
    std::string content;

    //* get absolute path
    filename = std::filesystem::absolute(filename).string();

    try_read_file(filename, &content);

    std::cout << "File content: " << content << std::endl;

    Lexer lexer(content);
    auto tokens = lexer.tokenize();

    std::cout << "Tokens:" << std::endl;
    for (const auto &token : tokens) {
        std::cout << "Type: " << token.humanize() << ", Value: " << token.value << std::endl;
    }
    std::cout << "Token count: " << tokens.size() << std::endl;

    Parser parser(tokens, filename, content);

    auto ast = parser.parse();
    if (!ast.empty()) {
        std::cout << "AST created successfully." << std::endl;
        std::cout << "AST size: " << ast.size() << std::endl;
        for (const auto& node : ast) {
            printAST(node.get());
        }
    } else {
        std::cerr << "[error]: Failed to create AST." << std::endl;
        return 1;
    }

    CodeGenerator codeGen;
    for (auto& node : ast) {
        codeGen.processPart(node);
    }
    auto code = codeGen.generateCode();

    std::cout << "Üretilen C Kodu:" << std::endl;
    for (const auto& line : code) {
        std::cout << line << std::endl;
    }

    return 0;
}
