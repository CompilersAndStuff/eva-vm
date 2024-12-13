#include "OpCode.h"
#include "../Logger.h"

std::string opcodeToString(uint8_t opcode) {
  switch (static_cast<OpCode>(opcode)) {
  case OpCode::HALT:
    return "HALT";
  case OpCode::CONST:
    return "CONST";
  case OpCode::ADD:
    return "ADD";
  case OpCode::SUB:
    return "SUB";
  case OpCode::MUL:
    return "MUL";
  case OpCode::DIV:
    return "DIV";
  case OpCode::COMPARE:
    return "COMPARE";
  case OpCode::JMP_IF_FALSE:
    return "JMP_IF_FALSE";
  case OpCode::JMP:
    return "JMP";
  case OpCode::GET_GLOBAL:
    return "GET_GLOBAL";
  case OpCode::SET_GLOBAL:
    return "SET_GLOBAL";
  case OpCode::POP:
    return "POP";
  case OpCode::GET_LOCAL:
    return "GET_LOCAL";
  case OpCode::SET_LOCAL:
    return "SET_LOCAL";
  case OpCode::SCOPE_EXIT:
    return "SCOPE_EXIT";
  case OpCode::CALL:
    return "CALL";
  case OpCode::RETURN:
    return "RETURN";
  case OpCode::GET_CELL:
    return "GET_CELL";
  case OpCode::SET_CELL:
    return "SET_CELL";
  case OpCode::LOAD_CELL:
    return "LOAD_CELL";
  case OpCode::MAKE_FUNCTION:
    return "MAKE_FUNCTION";

  default:
    DIE << "opcodeToString: unknown opcode: " << (int)opcode;
  }
  return "Unknown";
}
