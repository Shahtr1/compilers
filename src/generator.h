#pragma once
#include "parser.h"

class Generator{
public:
    explicit Generator(NodeProg root): m_prog(std::move(root)){}

    [[nodiscard]] std::string generate() const{
        std::stringstream output;
        output << "global _start\n_start:\n";
        output << "    mov rax, 60\n";
        output << "    mov rdi, " << m_prog.expr.int_lit.value.value() << "\n";
        output << "    syscall";
        return output.str();
    }

private:
    const NodeProg m_prog;
};
