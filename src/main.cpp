#define EXTENSION_NAME ".reic"

#include "parser.h"
#include "lexer.h"
#include "code_generator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <format>
#include <cstdlib>
#include <ctime>
#include <chrono>

bool isVerbose = false;

// todo: move to a separate file
class Timer {
public:
    Timer() : start(std::chrono::high_resolution_clock::now()) {}

    void reset() {
        start = std::chrono::high_resolution_clock::now();
    }

    double elapsed() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return static_cast<double>(duration.count()) / 1000.0;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

void verbose(const std::string &message) {
    if (!isVerbose) return;
    std::cout << "[verbose]: " << message << std::endl;
}

void tryReadFile(std::string &filename, std::string *content) {
    std::ifstream file(filename);
    if (!file) {
        if (!filename.ends_with(EXTENSION_NAME)) {
            filename += EXTENSION_NAME;
            tryReadFile(filename, content);
            return;
        }
        std::cerr << "[error]: Error opening file: " << filename << std::endl;
        exit(1);
    }

    verbose(std::format("Reading file: {}", filename));
    std::string line;
    while (getline(file, line)) {
        content->append(line + "\n");
    }

    if (file.bad()) {
        std::cerr << "[error]: Error reading file: " << filename << std::endl;
        exit(1);
    }

    verbose("File read successfully.");
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

void displayHelp() {
    std::cout << "Usage: <filename> [-v|--verbose] [-h|--help] [-o <output_file>] [--compile] [--run]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -v, --verbose   Enable verbose output" << std::endl;
    std::cout << "  -h, --help      Show this help message" << std::endl;
    std::cout << "  -o <filename>   Specify output file name" << std::endl;
    std::cout << "      --compile   Compile the output file using clang" << std::endl;
    std::cout << "      --run       Run file after compilation" << std::endl;
}

std::string generateRandomString(size_t length) {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string randomString;
    std::srand(std::time(0));
    for (size_t i = 0; i < length; ++i) {
        randomString += chars[std::rand() % chars.size()];
    }
    return randomString;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "[error]: No input file specified." << std::endl;
        displayHelp();
        return 1;
    }

    std::string filename = argv[1];
    std::string outputFileName = filename.substr(0, filename.find_last_of('.')) + ".c";
    bool compile = false;
    bool run = false;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose") {
            isVerbose = true;
        } else if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            displayHelp();
            return 0;
        } else if (std::string(argv[i]) == "-o" || std::string(argv[i]) == "--output") {
            if (i + 1 < argc) {
                outputFileName = argv[++i];
            } else {
                std::cerr << "[error]: No output file name specified after -o" << std::endl;
                return 1;
            }
        } else if (std::string(argv[i]) == "--compile") {
            compile = true;
        } else if (std::string(argv[i]) == "--run") {
            run = true;
        } else if (std::string(argv[i]) == "--no-compile") {
            compile = false;
        } else if (std::string(argv[i]) == "--no-run") {
            run = false;
        } else {
            if (i == 1) continue; // Skip the first argument (filename)
            std::cerr << "[error]: Unknown option: " << argv[i] << std::endl;
            return 1;
        }
    }

    verbose(std::format("File name: {}", filename));
    std::string content;

    //* get absolute path
    filename = std::filesystem::absolute(filename).string();

    tryReadFile(filename, &content);

    verbose(std::format("File content: {}", content));

    Timer timer;

    Lexer lexer(content);
    auto tokens = lexer.tokenize();

    verbose("Tokens:");
    for (const auto &token : tokens) {
        verbose(std::format("Type: {}, Value: {}", token.humanize(), token.value));
    }
    verbose(std::format("Token count: {}", tokens.size()));

    Parser parser(tokens, filename, content);

    auto ast = parser.parse();
    if (!ast.empty()) {
        verbose("AST created successfully.");
        verbose(std::format("AST size: {}", ast.size()));
        if (isVerbose) {
            for (const auto& node : ast) {
                printAST(node.get());
            }
        }
    } else {
        std::cerr << "[error]: Failed to create AST." << std::endl;
        return 1;
    }

    CodeGenerator codeGen(std::move(ast));
    auto code = codeGen.generateCode();

    verbose("Generated Code:");
    if (isVerbose) {
        for (const auto& line : code) {
            std::cout << line << std::endl;
        }
    }

    if (run) {
        std::string outputFileName = std::filesystem::temp_directory_path().string() + "/" + generateRandomString(8) + ".c";
    }
    verbose(std::format("Output file: {}", outputFileName));
    std::ofstream outputFile(outputFileName);
    if (!outputFile) {
        std::cerr << "[error]: Error opening output file: " << outputFileName << std::endl;
        return 1;
    }
    for (const auto& line : code) {
        outputFile << line << std::endl;
    }
    outputFile.close();
    if (compile || run) {
        std::string command = "clang -g -o " + outputFileName.substr(0, outputFileName.find_last_of('.')) + " " + outputFileName;
        verbose(std::format("Compiling with command: {}", command));
        int result = std::system(command.c_str());
        double elapsedTime = timer.elapsed();
        timer.reset();
        if (result != 0) {
            std::cerr << "[error]: Compilation failed." << std::endl;
            return 1;
        }
        std::cout << std::format("Compiled development build [notstripped + debuginfo] in {:.2f}s", elapsedTime) << std::endl;
        if (run) {
            std::string runCommand = "./" + outputFileName.substr(0, outputFileName.find_last_of('.'));
            std::cout << "Running build" << std::endl;
            int runResult = std::system(runCommand.c_str());
            std::filesystem::remove(outputFileName);
            if (runResult != 0) {
                std::cerr << "[error]: Execution failed." << std::endl;
                return 1;
            }
        }
    }

    return 0;
}
