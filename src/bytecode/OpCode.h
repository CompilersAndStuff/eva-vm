#ifndef __OpCode_h
#define __OpCode_h

#include <cstdint>
#include <string>

enum class OpCode {
  HALT = 0x00,
  CONST = 0x01,
  ADD = 0x02,
  SUB = 0x03,
  MUL = 0x04,
  DIV = 0x05,
  COMPARE = 0x06,
  JMP_IF_FALSE = 0x07,
  JMP = 0x08,
  GET_GLOBAL = 0x09,
  SET_GLOBAL = 0xA,
  POP = 0xB,
  GET_LOCAL = 0xC,
  SET_LOCAL = 0xD,
  SCOPE_EXIT = 0xE,
  CALL = 0xF,
  RETURN = 0x10,
  GET_CELL = 0x11,
  SET_CELL = 0x12,
  LOAD_CELL = 0x13,
  MAKE_FUNCTION = 0x14
};

std::string opcodeToString(uint8_t opcode);

#endif // __OpCode_h
