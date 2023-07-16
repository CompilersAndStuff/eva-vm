#ifndef EvaVM_h
#define EvaVM_h

#include <array>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../Logger.h"
#include "../bytecode/OpCode.h"
#include "../compiler/EvaCompiler.h"
#include "../parser/EvaParser.h"
#include "EvaValue.h"

using syntax::EvaParser;

#define READ_BYTE() *ip++

#define GET_CONST() co->constants[READ_BYTE()]

#define STACK_LIMIT 512

#define BINARY_OP(op)                                                          \
  do {                                                                         \
    auto op2 = AS_NUMBER(pop());                                               \
    auto op1 = AS_NUMBER(pop());                                               \
    push(NUMBER(op1 op op2));                                                  \
  } while (false)

class EvaVM {
public:
  EvaVM()
      : parser(std::make_unique<EvaParser>()),
        compiler(std::make_unique<EvaCompiler>()) {}

  void push(const EvaValue &value) {
    if ((size_t)(sp - stack.begin()) == STACK_LIMIT) {
      DIE << "push(): Stack overflow.\n";
    }
    *sp = value;
    sp++;
  }

  EvaValue pop() {
    if (sp == stack.begin()) {
      DIE << "pop(): empty stack.\n";
    }
    --sp;
    return *sp;
  }

  EvaValue exec(const std::string &program) {
    auto ast = parser->parse(program);

    co = compiler->compile(ast);

    ip = &co->code[0];

    sp = &stack[0];

    return eval();
  }

  EvaValue eval() {
    for (;;) {
      auto opcode = READ_BYTE();
      log(opcode);
      switch (opcode) {
      case OP_HALT:
        return pop();

      case OP_CONST:
        push(GET_CONST());
        break;

      case OP_ADD: {
        auto op1 = pop();
        auto op2 = pop();

        if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
          auto v1 = AS_NUMBER(op1);
          auto v2 = AS_NUMBER(op2);
          push(NUMBER(v1 + v2));
        }

        else if (IS_STRING(op1) && IS_STRING(op2)) {
          auto s2 = AS_CPPSTRING(op1);
          auto s1 = AS_CPPSTRING(op2);
          push(ALLOC_STRING(s1 + s2));
        }

        break;
      }

      case OP_SUB: {
        BINARY_OP(-);
        break;
      }

      case OP_MUL: {
        BINARY_OP(*);
        break;
      }

      case OP_DIV: {
        BINARY_OP(/);
        break;
      }

      default:
        DIE << "Unknown opcode: " << std::hex << opcode;
      }
    }
  }

  std::unique_ptr<EvaParser> parser;

  std::unique_ptr<EvaCompiler> compiler;

  uint8_t *ip;

  EvaValue *sp;

  std::array<EvaValue, STACK_LIMIT> stack;

  CodeObject *co;
};

#endif
