#pragma once
#include <variant>

#include "arena.h"
#include "tokenizer.h"

struct NodeTermIntLit{
    Token int_lit;
};

struct NodeTermIdent{
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

// struct NodeBinExprMulti{
//     NodeExpr* lhs;
//     NodeExpr* rhs;
// };

struct NodeBinExpr{
    NodeBinExprAdd* add;
};

struct NodeTerm{
    std::variant<NodeTermIntLit*, NodeTermIdent*> var;
};

struct NodeExpr{
    std::variant<NodeTerm*, NodeBinExpr*> var;
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

    std::optional<NodeBinExpr*> parse_bin_expr(){
        if (const auto lhs = parse_expr()) {
            std::cerr << "Unsupported binary operator" << std::endl;
            exit(EXIT_FAILURE);
        }
        return {};
    }

    std::optional<NodeTerm*> parse_term(){
        if (const auto int_lit = try_consume(TokenType::int_literal)) {
            auto* term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        if (const auto ident = try_consume(TokenType::ident)) {
            auto* term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        }
        return {};
    }

    std::optional<NodeExpr*> parse_expr(){
        if (auto term = parse_term()) {
            if (try_consume(TokenType::plus).has_value()) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                const auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                const auto lhs_expr = m_allocator.alloc<NodeExpr>();
                lhs_expr->var = term.value();
                bin_expr_add->lhs = lhs_expr;
                if (const auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->add = bin_expr_add;
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;
                    return expr;
                }
                std::cerr << "Expected expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = term.value();
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

            try_consume(TokenType::close_paren, "Expected `)`");
            try_consume(TokenType::semicolon, "Expected `;`");

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

            try_consume(TokenType::semicolon, "Expected `;`");

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

    std::optional<Token> try_consume(const TokenType type){
        if (peek().has_value() && peek()->type == type) {
            return consume();
        }
        return {};
    }

    Token try_consume(const TokenType type, const std::string& err_msg){
        if (peek().has_value() && peek()->type == type) {
            return consume();
        }
        std::cerr << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }
};
