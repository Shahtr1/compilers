#pragma once

#include <utility>
#include<vector>
#include<string>

enum class TokenType{
    exit,
    int_literal,
    semicolon,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    plus,
    star,
    sub,
    div
};

inline std::optional<int> bin_prec(TokenType type){
    switch (type) {
    case TokenType::plus:
    case TokenType::sub:
        return 0;
    case TokenType::star:
    case TokenType::div:
        return 1;
    default:
        return {};
    }
}

struct Token{
    TokenType type;
    std::optional<std::string> value;
};

class Tokenizer{
public:
    explicit Tokenizer(std::string src): m_src(std::move(src)){}

    std::vector<Token> tokenize(){
        std::vector<Token> tokens;
        std::string buf;

        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({.type = TokenType::exit});
                    buf.clear();
                }
                else if (buf == "let") {
                    tokens.push_back({.type = TokenType::let});
                    buf.clear();
                }
                else {
                    tokens.push_back({.type = TokenType::ident, .value = buf});
                    buf.clear();
                }
            }
            else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_literal, .value = buf});
                buf.clear();
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::open_paren});
            }
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::close_paren});
            }
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semicolon});
            }
            else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::eq});
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({.type = TokenType::plus});
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({.type = TokenType::star});
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({.type = TokenType::sub});
            }
            else if (peek().value() == '/') {
                consume();
                tokens.push_back({.type = TokenType::div});
            }
            else if (std::isspace(peek().value())) {
                consume();
            }
            else {
                std::cerr << "You messed up!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    const std::string m_src;

    int m_index = 0;

    [[nodiscard]] std::optional<char> peek(int offset = 0) const{
        if (m_index + offset >= m_src.size()) {
            return {};
        }
        return m_src[m_index + offset];
    }

    char consume(){
        return m_src[m_index++];
    }
};
