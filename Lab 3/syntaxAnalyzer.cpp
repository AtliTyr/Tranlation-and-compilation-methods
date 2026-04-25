#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <regex>

enum class TokenType {
    DELIMITER, KEYWORD, OPERATOR, IDENTIFIER, CONSTANT_INT, CONSTANT_STRING
};

struct Token {
    TokenType type;
    std::string value;
    size_t position;
};

enum class NodeType {
    Program,
    PreprocessorSection,
    FuncDeclSection,
    FuncDefSection,

    Preprocessor,
    FunctionDecl,
    FunctionDef,
    Param,
    VarDecl,
    Assign,
    IfStmt,
    Else,
    ForStmt,
    ReturnStmt,
    Call,
    BinaryOp,
    UnaryOp,
    Literal,
    Identifier,
    Block,
    ExpressionStmt
};

struct ASTNode {
    NodeType type;
    std::string value;
    std::vector<std::shared_ptr<ASTNode>> children;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

    ASTNode(NodeType t, std::string v = "") : type(t), value(std::move(v)) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current = 0;

    Token peek() const {
        if (current >= tokens.size()) return tokens.back();
        return tokens[current];
    }

    Token advance() {
        return tokens[current++];
    }

    bool check(TokenType type, const std::string& value = "") {
        if (current >= tokens.size()) return false;
        if (tokens[current].type != type) return false;
        if (!value.empty() && tokens[current].value != value) return false;
        return true;
    }

    Token consume(TokenType type, const std::string& value, const std::string& msg) {
        if (check(type, value)) return advance();
        error(msg, peek());
        return peek();
    }

    void error(const std::string& msg, const Token& tok) {
        std::cerr << "Syntax Error: " << msg
                  << " at position " << tok.position
                  << " (token: " << tok.value << ")\n";
        throw std::runtime_error("Parse error");
    }

    // ===== EXPRESSIONS =====

    std::shared_ptr<ASTNode> parsePrimary() {
        if (check(TokenType::CONSTANT_INT)) {
            return std::make_shared<ASTNode>(NodeType::Literal, advance().value);
        }
        if (check(TokenType::CONSTANT_STRING)) {
            return std::make_shared<ASTNode>(NodeType::Literal, advance().value);
        }
        if (check(TokenType::IDENTIFIER)) {
            auto idTok = advance();
            auto node = std::make_shared<ASTNode>(NodeType::Identifier, idTok.value);

            if (check(TokenType::DELIMITER, "(")) {
                advance();
                auto call = std::make_shared<ASTNode>(NodeType::Call, idTok.value);

                if (!check(TokenType::DELIMITER, ")")) {
                    do {
                        call->children.push_back(parseExpression());
                    } while (check(TokenType::DELIMITER, ",") && advance().value == ",");
                }

                consume(TokenType::DELIMITER, ")", "Expected ')'");
                return call;
            }
            return node;
        }

        if (check(TokenType::DELIMITER, "(")) {
            advance();
            auto expr = parseExpression();
            consume(TokenType::DELIMITER, ")", "Expected ')'");
            return expr;
        }

        error("Unexpected token in expression", peek());
        return nullptr;
    }

    std::shared_ptr<ASTNode> parsePostfix() {
        auto node = parsePrimary();

        while (check(TokenType::OPERATOR, "++")) {
            auto op = advance().value;
            auto unary = std::make_shared<ASTNode>(NodeType::UnaryOp, op);
            unary->left = node;
            node = unary;
        }

        return node;
    }

    std::shared_ptr<ASTNode> parseUnary() {
        if (check(TokenType::OPERATOR, "-") || check(TokenType::OPERATOR, "++")) {
            auto op = advance().value;
            auto node = std::make_shared<ASTNode>(NodeType::UnaryOp, op);
            node->left = parseUnary();
            return node;
        }
        return parsePostfix();
    }

    std::shared_ptr<ASTNode> parseMul() {
        auto left = parseUnary();
        while (check(TokenType::OPERATOR, "*") || check(TokenType::OPERATOR, "/")) {
            auto op = advance().value;
            auto right = parseUnary();
            auto node = std::make_shared<ASTNode>(NodeType::BinaryOp, op);
            node->left = left;
            node->right = right;
            left = node;
        }
        return left;
    }

    std::shared_ptr<ASTNode> parseAdd() {
        auto left = parseMul();
        while (check(TokenType::OPERATOR, "+") || check(TokenType::OPERATOR, "-")) {
            auto op = advance().value;
            auto right = parseMul();
            auto node = std::make_shared<ASTNode>(NodeType::BinaryOp, op);
            node->left = left;
            node->right = right;
            left = node;
        }
        return left;
    }

    std::shared_ptr<ASTNode> parseCompare() {
        auto left = parseAdd();
        while (check(TokenType::OPERATOR, ">") || check(TokenType::OPERATOR, "<") || check(TokenType::OPERATOR, "<=")) {
            auto op = advance().value;
            auto right = parseAdd();
            auto node = std::make_shared<ASTNode>(NodeType::BinaryOp, op);
            node->left = left;
            node->right = right;
            left = node;
        }
        return left;
    }

    std::shared_ptr<ASTNode> parseAssignment() {
        auto left = parseCompare();

        if (check(TokenType::OPERATOR, "=")) {
            advance();
            auto right = parseAssignment();
            auto node = std::make_shared<ASTNode>(NodeType::Assign);
            node->left = left;
            node->right = right;
            return node;
        }

        return left;
    }

    std::shared_ptr<ASTNode> parseExpression() {
        return parseAssignment();
    }

    std::shared_ptr<ASTNode> parseBlock() {
        consume(TokenType::DELIMITER, "{", "Expected '{'");
        auto block = std::make_shared<ASTNode>(NodeType::Block);

        while (!check(TokenType::DELIMITER, "}")) {
            block->children.push_back(parseStatement());
        }

        consume(TokenType::DELIMITER, "}", "Expected '}'");
        return block;
    }

    std::shared_ptr<ASTNode> parseVarDecl() {
        consume(TokenType::KEYWORD, "int", "Expected 'int'");
        auto id = consume(TokenType::IDENTIFIER, "", "Expected identifier");

        auto node = std::make_shared<ASTNode>(NodeType::VarDecl, id.value);

        if (check(TokenType::OPERATOR, "=")) {
            advance();
            node->children.push_back(parseExpression());
        }

        consume(TokenType::DELIMITER, ";", "Expected ';'");
        return node;
    }

    std::shared_ptr<ASTNode> parseIf() {
        consume(TokenType::KEYWORD, "if", "Expected 'if'");
        consume(TokenType::DELIMITER, "(", "Expected '('");

        auto cond = parseExpression();
        consume(TokenType::DELIMITER, ")", "Expected ')'");

        auto node = std::make_shared<ASTNode>(NodeType::IfStmt);
        node->children.push_back(cond);
        node->children.push_back(parseBlock());

        if (check(TokenType::KEYWORD, "else")) {
            advance();
            if (check(TokenType::KEYWORD, "if")) {
                node->children.push_back(parseIf());
            } else {
                auto elseNode = std::make_shared<ASTNode>(NodeType::Else);
                elseNode->children.push_back(parseBlock());
                node->children.push_back(elseNode);
            }
        }

        return node;
    }

    std::shared_ptr<ASTNode> parseFor() {
        consume(TokenType::KEYWORD, "for", "Expected 'for'");
        consume(TokenType::DELIMITER, "(", "Expected '('");

        auto init = parseExpression();
        consume(TokenType::DELIMITER, ";", "Expected ';'");

        auto cond = parseExpression();
        consume(TokenType::DELIMITER, ";", "Expected ';'");

        auto incr = parseExpression();
        consume(TokenType::DELIMITER, ")", "Expected ')'");

        auto body = parseBlock();

        auto node = std::make_shared<ASTNode>(NodeType::ForStmt);
        node->children = {init, cond, incr, body};
        return node;
    }

    std::shared_ptr<ASTNode> parseReturn() {
        consume(TokenType::KEYWORD, "return", "Expected 'return'");
        auto node = std::make_shared<ASTNode>(NodeType::ReturnStmt);

        if (!check(TokenType::DELIMITER, ";")) {
            node->children.push_back(parseExpression());
        }

        consume(TokenType::DELIMITER, ";", "Expected ';'");
        return node;
    }

    std::shared_ptr<ASTNode> parseStatement() {
        if (check(TokenType::KEYWORD, "int")) return parseVarDecl();
        if (check(TokenType::KEYWORD, "if")) return parseIf();
        if (check(TokenType::KEYWORD, "for")) return parseFor();
        if (check(TokenType::KEYWORD, "return")) return parseReturn();

        auto expr = parseExpression();
        consume(TokenType::DELIMITER, ";", "Expected ';'");

        auto stmt = std::make_shared<ASTNode>(NodeType::ExpressionStmt);
        stmt->children.push_back(expr);
        return stmt;
    }

    // ===== FUNCTIONS =====

    std::shared_ptr<ASTNode> parseFunction() {
        consume(TokenType::KEYWORD, "int", "Expected return type");
        auto name = consume(TokenType::IDENTIFIER, "", "Expected function name");

        consume(TokenType::DELIMITER, "(", "Expected '('");

        auto node = std::make_shared<ASTNode>(NodeType::FunctionDef, name.value);

        if (!check(TokenType::DELIMITER, ")")) {
            do {
                consume(TokenType::KEYWORD, "int", "Expected param type");
                auto paramName = consume(TokenType::IDENTIFIER, "", "Expected param name");

                auto p = std::make_shared<ASTNode>(NodeType::Param, paramName.value);
                node->children.push_back(p);

            } while (check(TokenType::DELIMITER, ",") && advance().value == ",");
        }

        consume(TokenType::DELIMITER, ")", "Expected ')'");

        if (check(TokenType::DELIMITER, ";")) {
            advance();
            node->type = NodeType::FunctionDecl;
            return node;
        }

        node->children.push_back(parseBlock());
        return node;
    }

    std::shared_ptr<ASTNode> parsePreprocessor() {
        consume(TokenType::DELIMITER, "#", "Expected '#'");
        consume(TokenType::KEYWORD, "include", "Expected include");
        consume(TokenType::OPERATOR, "<", "Expected '<'");
        auto lib = consume(TokenType::IDENTIFIER, "", "Expected header");
        consume(TokenType::OPERATOR, ">", "Expected '>'");

        auto node = std::make_shared<ASTNode>(NodeType::Preprocessor, "include");
        node->children.push_back(std::make_shared<ASTNode>(NodeType::Identifier, lib.value));
        return node;
    }

public:
    std::shared_ptr<ASTNode> parse(const std::vector<Token>& input) {
        tokens = input;
        current = 0;

        auto program = std::make_shared<ASTNode>(NodeType::Program);

        auto preprocSection = std::make_shared<ASTNode>(NodeType::PreprocessorSection);
        auto declSection    = std::make_shared<ASTNode>(NodeType::FuncDeclSection);
        auto defSection     = std::make_shared<ASTNode>(NodeType::FuncDefSection);

        while (current < tokens.size()) {

            if (check(TokenType::DELIMITER, "#")) {
                preprocSection->children.push_back(parsePreprocessor());
                continue;
            }

            auto func = parseFunction();

            if (func->type == NodeType::FunctionDecl) {
                declSection->children.push_back(func);
            } else {
                defSection->children.push_back(func);
            }
        }

        if (!preprocSection->children.empty())
            program->children.push_back(preprocSection);

        if (!declSection->children.empty())
            program->children.push_back(declSection);

        if (!defSection->children.empty())
            program->children.push_back(defSection);

        return program;
    }
};

std::string nodeTypeToString(NodeType type) {
    switch (type) {
        case NodeType::Program: return "Program";
        case NodeType::PreprocessorSection: return "PreprocessorSection";
        case NodeType::FuncDeclSection: return "FunctionDeclSection";
        case NodeType::FuncDefSection: return "FunctionDefSection";
        
        case NodeType::Preprocessor: return "Preprocessor";
        case NodeType::FunctionDecl: return "FunctionDecl";
        case NodeType::FunctionDef: return "FunctionDef";
        case NodeType::Param: return "Param";
        case NodeType::VarDecl: return "VarDecl";
        case NodeType::Assign: return "Assign";
        case NodeType::IfStmt: return "IfStmt";
        case NodeType::Else: return "Else";
        case NodeType::ForStmt: return "ForStmt";
        case NodeType::ReturnStmt: return "ReturnStmt";
        case NodeType::Call: return "Call";
        case NodeType::BinaryOp: return "BinaryOp";
        case NodeType::UnaryOp: return "UnaryOp";
        case NodeType::Literal: return "Literal";
        case NodeType::Identifier: return "Identifier";
        case NodeType::Block: return "Block";
        case NodeType::ExpressionStmt: return "ExpressionStmt";
        default: return "Unknown";
    }
}

void printAST(std::shared_ptr<ASTNode> node, std::ostream& out, int indent = 0) {
    std::string ind(indent, ' ');

    out << ind << "{\n";
    out << ind << "  \"type\": \"" << nodeTypeToString(node->type) << "\",\n";
    out << ind << "  \"value\": \"" << node->value << "\"";

    if (!node->children.empty() || node->left || node->right) {
        out << ",\n";
    } else {
        out << "\n";
    }

    if (node->left) {
        out << ind << "  \"left\":\n";
        printAST(node->left, out, indent + 4);
        if (node->right || !node->children.empty()) out << ",\n";
    }

    if (node->right) {
        out << ind << "  \"right\":\n";
        printAST(node->right, out, indent + 4);
        if (!node->children.empty()) out << ",\n";
    }

    if (!node->children.empty()) {
        out << ind << "  \"children\": [\n";
        for (size_t i = 0; i < node->children.size(); ++i) {
            printAST(node->children[i], out, indent + 4);
            if (i + 1 != node->children.size()) out << ",\n";
        }
        out << "\n" << ind << "  ]\n";
    }

    out << ind << "}";
}

TokenType parseType(const std::string& s) {
    if (s == "KEYWORD") return TokenType::KEYWORD;
    if (s == "IDENTIFIER") return TokenType::IDENTIFIER;
    if (s == "CONSTANT_INT") return TokenType::CONSTANT_INT;
    if (s == "CONSTANT_STRING") return TokenType::CONSTANT_STRING;
    if (s == "OPERATOR") return TokenType::OPERATOR;
    if (s == "DELIMITER") return TokenType::DELIMITER;
    return TokenType::IDENTIFIER;
}

std::vector<Token> parseTokenFile(const std::string& text) {
    std::vector<Token> tokens;

    std::regex pattern(R"(\(\s*([^,]+?)\s*,\s*(.+?)\s*\))");
    auto begin = std::sregex_iterator(text.begin(), text.end(), pattern);
    auto end = std::sregex_iterator();

    size_t pos = 0;

    for (auto it = begin; it != end; ++it) {
        std::string typeStr = (*it)[1];
        std::string lexeme = (*it)[2];

        tokens.push_back({
            parseType(typeStr),
            lexeme,
            pos++
        });
    }

    return tokens;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);

    if (!inputFile.is_open()) {
        std::cout << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    // Читаем весь файл
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string tokenStream = buffer.str();


    std::vector<Token> inputTokens = parseTokenFile(tokenStream);

    try {
        Parser parser;
        std::shared_ptr<ASTNode> ast = parser.parse(inputTokens);

        std::cout << "AST successfully built!\n";
        
        std::string outFileName = argv[1];
        std::ofstream out(outFileName.substr(0, outFileName.find('.')) + ".json");
        printAST(ast, out);
        out.close();

        std::cout << "AST saved in " << outFileName.substr(0, outFileName.find('.')) + ".json";
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}