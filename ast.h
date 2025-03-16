#ifndef AST_H
#define AST_H

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
    NodeType type;
    virtual ~ASTNode() = default;
    virtual std::string getType() const {
        switch (type) {
            case NodeType::STRING: return "String";
            case NodeType::NUMBER: return "Number";
            case NodeType::IDENTIFIER: return "Identifier";
            case NodeType::BINARY_OP: return "BinaryOp";
            case NodeType::ASSIGNMENT: return "Assignment";
            default: return "Unknown";
        }
    }
};

struct StringNode : ASTNode {
    std::string value;
    StringNode(const std::string& val) : value(val) { type = NodeType::STRING; }
};

struct NumberNode : ASTNode {
    int value;
    NumberNode(int val) : value(val) { type = NodeType::NUMBER; }
};

struct IdentifierNode : ASTNode {
    std::string name;
    IdentifierNode(const std::string& n) : name(n) { type = NodeType::IDENTIFIER; }
};

struct KeywordNode : ASTNode {
    std::string name;
    KeywordNode(const std::string& n) : name(n) { type = NodeType::KEYWORD; }
};

struct BinaryOpNode : ASTNode {
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    std::string op;

    BinaryOpNode(std::unique_ptr<ASTNode> l, std::string o, std::unique_ptr<ASTNode> r)
        : left(std::move(l)), right(std::move(r)), op(o) {
        type = NodeType::BINARY_OP;
    }
};

struct AssignmentNode : ASTNode {
    std::string variable;
    std::unique_ptr<ASTNode> value;
    NodeType valueType;

    AssignmentNode(const std::string& var, std::unique_ptr<ASTNode> val)
        : variable(var), value(std::move(val)) {
        valueType = value->type;
        type = NodeType::ASSIGNMENT;
    }
};

#endif // AST_H
