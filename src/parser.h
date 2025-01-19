#pragma once
#include <variant>

#include "tokenizer.h"

struct NodeExprIntLit{
    Token int_lit;
};

struct NodeExprIdent{
    Token ident;
};

struct NodeExpr{
    std::variant<NodeExprIntLit, NodeExprIdent> var;
};

struct NodeStmtExit{
    NodeExpr expr;
};

struct NodeStmtLet{
    Token ident;
    NodeExpr expr;
};

struct NodeStmt{
    std::variant<NodeStmtExit, NodeStmtLet> var;
};

struct NodeProg{
    std::vector<NodeStmt> stmts;
};

class Parser{
public:
    explicit Parser(std::vector<Token> tokens): m_tokens(std::move(tokens)){}

    std::optional<NodeExpr> parse_expr(){
        if (peek().has_value() && peek()->type == TokenType::int_literal) {
            return NodeExpr{.var = NodeExprIntLit{.int_lit = consume()}};
        }
        if (peek().has_value() && peek()->type == TokenType::ident) {
            return NodeExpr{.var = NodeExprIdent{.ident = consume()}};
        }
        return {};
    }

    std::optional<NodeStmt> parse_stmt(){
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1)->type == TokenType::open_paren) {
            consume();
            consume();
            NodeStmtExit stmt_exit;
            if (auto node_expr = parse_expr()) {
                stmt_exit = {.expr = node_expr.value()};
            }
            else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek()->type == TokenType::close_paren) {
                consume();
            }
            else {
                std::cerr << "Expected `)`" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek()->type == TokenType::semicolon) {
                consume();
            }
            else {
                std::cerr << "Expected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            return NodeStmt{.var = stmt_exit};
        }
        if (
            peek().has_value() && peek()->type == TokenType::let
            && peek(1).has_value() && peek(1)->type == TokenType::ident
            && peek(2).has_value() && peek(2)->type == TokenType::eq
        ) {
            consume();
            auto stmt_let = NodeStmtLet{.ident = consume()};
            consume();
            if (auto expr = parse_expr()) {
                stmt_let.expr = expr.value();
            }
            else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek()->type == TokenType::semicolon) {
                consume();
            }
            else {
                std::cerr << "Expected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            return NodeStmt{.var = stmt_let};
        }
        return {};
    }

    std::optional<NodeProg> parse_prog(){
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            }
            else {
                std::cerr << "Invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    [[nodiscard]] std::optional<Token> peek(int offset = 0) const{
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens.at(m_index + offset);
    }

    Token consume(){
        return m_tokens.at(m_index++);
    }
};
