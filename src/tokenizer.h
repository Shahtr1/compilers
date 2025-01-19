#pragma once

#include <utility>
#include<vector>
#include<string>

enum class TokenType{
    exit,
    int_literal,
    semicolon,
};

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
                    tokens.push_back(Token(TokenType::exit));
                    buf.clear();
                }
                else {
                    std::cerr << "You messed up!" << std::endl;
                    exit(EXIT_FAILURE);
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
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semicolon});
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

    [[nodiscard]] std::optional<char> peek() const{
        if (m_index >= m_src.size()) {
            return {};
        }
        return m_src[m_index];
    }

    char consume(){
        return m_src[m_index++];
    }
};
