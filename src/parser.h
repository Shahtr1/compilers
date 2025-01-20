#pragma once
#include <variant>

#include "arena.h"
#include "tokenizer.h"

struct NodeExprIntLit{
    Token int_lit;
};

struct NodeExprIdent{
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMulti{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr{
    std::variant<NodeBinExprAdd*, NodeBinExprMulti*> var;
};

struct NodeExpr{
    std::variant<NodeExprIntLit*, NodeExprIdent*, NodeBinExpr*> var;
};


struct NodeStmtExit{
    NodeExpr* expr;
};

struct NodeStmtLet{
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt{
    std::variant<NodeStmtExit*, NodeStmtLet*> var;
};

struct NodeProg{
    std::vector<NodeStmt*> stmts;
};

class Parser{
public:
    explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)),
          m_allocator(1024 * 1024 * 4) // 4mb
    {}

    std::optional<NodeExpr*> parse_expr(){
        if (peek().has_value() && peek()->type == TokenType::int_literal) {
            auto* expr_int_lit = m_allocator.alloc<NodeExprIntLit>();
            expr_int_lit->int_lit = consume();
            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = expr_int_lit;
            return expr;
        }
        if (peek().has_value() && peek()->type == TokenType::ident) {
            auto* expr_ident = m_allocator.alloc<NodeExprIdent>();
            expr_ident->ident = consume();
            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = expr_ident;
            return expr;
        }
        return {};
    }

    std::optional<NodeStmt*> parse_stmt(){
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1)->type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (const auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
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
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }
        if (
            peek().has_value() && peek()->type == TokenType::let
            && peek(1).has_value() && peek(1)->type == TokenType::ident
            && peek(2).has_value() && peek(2)->type == TokenType::eq
        ) {
            consume();
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
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
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
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
    ArenaAllocator m_allocator;

    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const{
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens.at(m_index + offset);
    }

    Token consume(){
        return m_tokens.at(m_index++);
    }
};
