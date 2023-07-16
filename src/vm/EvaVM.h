#ifndef EvaVM_h
#define EvaVM_h

#include <_types/_uint8_t.h>
#include <string>
#include <vector>
#include <iostream>

#include "../bytecode/OpCode.h"
#include "../Logger.h"

#define READ_BYTE() *ip++

class EvaVM {
public:
  EvaVM() {}

  void exec(const std::string &program) {
    code = {OP_HALT};

    ip = &code[0];

    return eval();
  }

  void eval() {
    for (;;) {
      uint8_t opcode = READ_BYTE();
      log(opcode);
      switch (opcode) {
      case OP_HALT:
        return;
      default:
        DIE << "Unknown opcode: " << std::hex << opcode;
      }
    }
  }

  uint8_t *ip;

  std::vector<uint8_t> code;
};

#endif
