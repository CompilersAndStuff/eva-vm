#ifndef __OpCode_h
#define __OpCode_h

#include <cstdint>
#include <string>

#define OP_HALT 0x00
#define OP_CONST 0x01
#define OP_ADD 0x02
#define OP_SUB 0x03
#define OP_MUL 0x04
#define OP_DIV 0x05
#define OP_COMPARE 0x06
#define OP_JMP_IF_FALSE 0x07
#define OP_JMP 0x08
#define OP_GET_GLOBAL 0x09
#define OP_SET_GLOBAL 0xA
#define OP_POP 0xB
#define OP_GET_LOCAL 0xC
#define OP_SET_LOCAL 0xD
#define OP_SCOPE_EXIT 0xE
#define OP_CALL 0xF
#define OP_RETURN 0x10
#define OP_GET_CELL 0x11
#define OP_SET_CELL 0x12
#define OP_LOAD_CELL 0x13
#define OP_MAKE_FUNCTION 0x14

#define OP_STR(op)                                                             \
  case OP_##op:                                                                \
    return #op

std::string opcodeToString(uint8_t opcode);

#endif // __OpCode_h
