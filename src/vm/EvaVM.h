#ifndef EvaVM_h
#define EvaVM_h

#include <array>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../Logger.h"
#include "../bytecode/OpCode.h"
#include "EvaValue.h"

#define READ_BYTE() *ip++

#define GET_CONST() constants[READ_BYTE()]

#define STACK_LIMIT 512

#define BINARY_OP(op)                                                          \
  do {                                                                         \
    auto op2 = AS_NUMBER(pop());                                               \
    auto op1 = AS_NUMBER(pop());                                               \
    push(NUMBER(op1 op op2));                                                  \
  } while (false)

class EvaVM {
public:
  EvaVM() {}

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

    constants.push_back(NUMBER(10));
    constants.push_back(NUMBER(3));
    constants.push_back(NUMBER(10));

    code = {OP_CONST, 0, OP_CONST, 1, OP_MUL, OP_CONST, 2, OP_SUB, OP_HALT};

    ip = &code[0];

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
        BINARY_OP(+);
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

  uint8_t *ip;

  EvaValue *sp;

  std::array<EvaValue, STACK_LIMIT> stack;

  std::vector<EvaValue> constants;

  std::vector<uint8_t> code;
};

#endif
