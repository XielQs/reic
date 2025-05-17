#pragma once
#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <string>

enum class NodeType {
    STRING,
    NUMBER,
    IDENTIFIER,
    KEYWORD,
    BINARY_OP,
    ASSIGNMENT
};

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual std::string getType() const {
        switch (type) {
            case NodeType::STRING: return "String";
            case NodeType::NUMBER: return "Number";
            case NodeType::IDENTIFIER: return "Identifier";
            case NodeType::KEYWORD: return "Keyword";
            case NodeType::BINARY_OP: return "BinaryOp";
            case NodeType::ASSIGNMENT: return "Assignment";
            default: return "Unknown";
        }
    }

    NodeType type;
};

struct StringNode : ASTNode {
    StringNode(const std::string& val) : value(val) { type = NodeType::STRING; }

    std::string value;
};

struct NumberNode : ASTNode {
    NumberNode(int val) : value(val) { type = NodeType::NUMBER; }

    int value;
};

struct IdentifierNode : ASTNode {
    IdentifierNode(const std::string& n) : name(n) { type = NodeType::IDENTIFIER; }

    std::string name;
};

struct KeywordNode : ASTNode {
    KeywordNode(const std::string& n) : name(n) { type = NodeType::KEYWORD; }

    std::string name;
};

struct BinaryOpNode : ASTNode {
    BinaryOpNode(std::unique_ptr<ASTNode> l, std::string o, std::unique_ptr<ASTNode> r)
        : left(std::move(l)), right(std::move(r)), op(o) {
        type = NodeType::BINARY_OP;
    }

    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    std::string op;
};

struct AssignmentNode : ASTNode {
    AssignmentNode(const std::string& var, std::unique_ptr<ASTNode> val)
        : variable(var), value(std::move(val)) {
        valueType = value->type;
        type = NodeType::ASSIGNMENT;
    }

    std::string variable;
    std::unique_ptr<ASTNode> value;
    NodeType valueType;
};

#endif // AST_HPP
