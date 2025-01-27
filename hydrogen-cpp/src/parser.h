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

struct NodeTermParen{
    NodeExpr* expr;
};

struct NodeBinExprAdd{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMulti{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr{
    std::variant<NodeBinExprMulti*, NodeBinExprAdd*, NodeBinExprDiv*, NodeBinExprSub*> var;
};

struct NodeTerm{
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr{
    std::variant<NodeTerm*, NodeBinExpr*> var;
};


struct NodeStmtExit{
    NodeExpr* expr;
};

struct NodeStmtLet{
    Token ident;
    NodeExpr* expr{};
};

struct NodeStmt;

struct NodeScope{
    std::vector<NodeStmt*> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif{
    NodeExpr* expr;
    NodeScope* scope;
    std::optional<NodeIfPred*> pred;
};

struct NodeIfPredElse{
    NodeScope* scope;
};

struct NodeIfPred{
    std::variant<NodeIfPredElif*, NodeIfPredElse*> var;
};

struct NodeStmtIf{
    NodeExpr* expr;
    NodeScope* scope;
    std::optional<NodeIfPred*> pred;
};

struct NodeStmtAssign{
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt{
    std::variant<NodeStmtExit*, NodeStmtLet*, NodeScope*, NodeStmtIf*, NodeStmtAssign*> var;
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
        if (parse_expr()) {
            std::cerr << "Unsupported binary operator" << std::endl;
            exit(EXIT_FAILURE);
        }
        return {};
    }

    void error_expected(const std::string& msg) const{
        std::cerr << "[Parser Error] Expected " << msg << " on line " << peek(-1).value().line << std::endl;
        exit(EXIT_FAILURE);
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
        if (const auto open_paren = try_consume(TokenType::open_paren)) {
            const auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("expression");
            }
            try_consume_error(TokenType::close_paren);
            auto term_paren = m_allocator.alloc<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_paren;
            return term;
        }
        return {};
    }

    std::optional<NodeExpr*> parse_expr(int const min_prec = 0){
        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) return {};

        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_lhs.value();

        while (true) {
            std::optional<Token> curr_tok = peek();
            std::optional<int> prec;
            if (curr_tok.has_value()) {
                prec = bin_prec(curr_tok->type);
                if (!prec.has_value() || prec < min_prec) break;
            }
            else break;
            auto [type, line, value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                error_expected("expression");
            }

            auto expr = m_allocator.alloc<NodeBinExpr>();
            auto const expr_lhs2 = m_allocator.alloc<NodeExpr>();
            if (type == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                expr_lhs2->var = expr_lhs->var;
                add->lhs = expr_lhs2;
                add->rhs = expr_rhs.value();
                expr->var = add;
            }
            else if (type == TokenType::star) {
                auto multi = m_allocator.alloc<NodeBinExprMulti>();
                expr_lhs2->var = expr_lhs->var;
                multi->lhs = expr_lhs2;
                multi->rhs = expr_rhs.value();
                expr->var = multi;
            }
            else if (type == TokenType::minus) {
                auto sub = m_allocator.alloc<NodeBinExprSub>();
                expr_lhs2->var = expr_lhs->var;
                sub->lhs = expr_lhs2;
                sub->rhs = expr_rhs.value();
                expr->var = sub;
            }
            else if (type == TokenType::fslash) {
                auto div = m_allocator.alloc<NodeBinExprDiv>();
                expr_lhs2->var = expr_lhs->var;
                div->lhs = expr_lhs2;
                div->rhs = expr_rhs.value();
                expr->var = div;
            }
            expr_lhs->var = expr;
        }

        return expr_lhs;
    }

    std::optional<NodeScope*> parse_scope(){
        if (!try_consume(TokenType::open_curly).has_value()) return {};
        auto scope = m_allocator.alloc<NodeScope>();
        while (auto stmt = parse_stmt()) {
            scope->stmts.push_back(stmt.value());
        }
        try_consume_error(TokenType::close_curly);
        return scope;
    }

    std::optional<NodeIfPred*> parse_if_pred(){
        if (try_consume(TokenType::elif)) {
            try_consume_error(TokenType::open_paren);
            auto const elif = m_allocator.alloc<NodeIfPredElif>();
            if (auto const expr = parse_expr()) {
                elif->expr = expr.value();
            }
            else {
                error_expected("expression");
            }

            try_consume_error(TokenType::close_paren);
            if (auto const scope = parse_scope()) {
                elif->scope = scope.value();
            }
            else {
                error_expected("scope");
            }
            elif->pred = parse_if_pred();
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = elif;
            return pred;
        }
        if (try_consume(TokenType::else_)) {
            const auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (const auto scope = parse_scope()) {
                else_->scope = scope.value();
            }
            else {
                error_expected("scope");
            }
            const auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = else_;
            return pred;
        }
        return {};
    }

    std::optional<NodeStmt*> parse_stmt(){
        if (peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1)->type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (const auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            }
            else {
                error_expected("expression");
            }

            try_consume_error(TokenType::close_paren);
            try_consume_error(TokenType::semicolon);

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
                error_expected("expression");
            }

            try_consume_error(TokenType::semicolon);

            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        }

        if (peek().has_value() && peek()->type == TokenType::ident
            && peek(1).has_value() && peek(1)->type == TokenType::eq) {
            const auto assign = m_allocator.alloc<NodeStmtAssign>();
            assign->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                assign->expr = expr.value();
            }
            else {
                error_expected("expression");
            }
            try_consume_error(TokenType::semicolon);
            const auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = assign;
            return stmt;
        }

        if (peek().has_value() && peek()->type == TokenType::open_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = scope.value();
                return stmt;
            }
            error_expected("scope");
        }

        if (auto if_ = try_consume(TokenType::if_)) {
            try_consume_error(TokenType::open_paren);
            auto const stmt_if = m_allocator.alloc<NodeStmtIf>();
            if (auto const expr = parse_expr()) {
                stmt_if->expr = expr.value();
            }
            else {
                error_expected("expression");
            }
            try_consume_error(TokenType::close_paren);
            if (auto const scope = parse_scope()) {
                stmt_if->scope = scope.value();
            }
            else {
                error_expected("scope");
            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_if;
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

    Token try_consume_error(const TokenType type){
        if (peek().has_value() && peek()->type == type) {
            return consume();
        }

        error_expected(to_string(type));
        return {};
    }
};
