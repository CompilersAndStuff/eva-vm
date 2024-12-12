#include "OpCode.h"
#include "../Logger.h"

std::string opcodeToString(uint8_t opcode) {
  switch (opcode) {
    OP_STR(HALT);
    OP_STR(CONST);
    OP_STR(ADD);
    OP_STR(SUB);
    OP_STR(MUL);
    OP_STR(DIV);
    OP_STR(COMPARE);
    OP_STR(JMP_IF_FALSE);
    OP_STR(JMP);
    OP_STR(GET_GLOBAL);
    OP_STR(SET_GLOBAL);
    OP_STR(POP);
    OP_STR(GET_LOCAL);
    OP_STR(SET_LOCAL);
    OP_STR(SCOPE_EXIT);
    OP_STR(CALL);
    OP_STR(RETURN);
    OP_STR(GET_CELL);
    OP_STR(SET_CELL);
    OP_STR(LOAD_CELL);
    OP_STR(MAKE_FUNCTION);

  default:
    DIE << "opcodeToString: unknown opcode: " << (int)opcode;
  }
  return "Unknown";
}
