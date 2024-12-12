#include "EvaDisassembler.h"
#include "../bytecode/OpCode.h"
#include "../vm/EvaValue.h"
#include "../vm/Global.h"
#include "../Logger.h"
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>

EvaDisassembler::EvaDisassembler(std::shared_ptr<Global> global)
    : global(global) {}

void EvaDisassembler::disassemble(CodeObject *co) {
  std::cout << "\n-------------- Disassembly: " << co->name
            << " ---------------\n\n";
  size_t offset = 0;
  while (offset < co->code.size()) {
    offset = disassembleInstruction(co, offset);
    std::cout << "\n";
  }
}

size_t EvaDisassembler::disassembleInstruction(CodeObject *co, size_t offset) {
  std::ios_base::fmtflags f(std::cout.flags());

  std::cout << std::uppercase << std::hex << std::setfill('0') << std::setw(4)
            << offset << "    ";

  auto opcode = co->code[offset];

  switch (opcode) {
  case OP_HALT:
    return disassembleSimple(co, opcode, offset);
  case OP_CONST:
    return disassembleConst(co, opcode, offset);
  case OP_ADD:
  case OP_SUB:
  case OP_MUL:
  case OP_DIV:
  case OP_POP:
  case OP_RETURN:
    return disassembleSimple(co, opcode, offset);
  case OP_SCOPE_EXIT:
  case OP_CALL:
    return disassembleWord(co, opcode, offset);
  case OP_COMPARE:
    return disassembleCompare(co, opcode, offset);
  case OP_JMP_IF_FALSE:
  case OP_JMP:
    return disassembleJump(co, opcode, offset);
  case OP_GET_GLOBAL:
  case OP_SET_GLOBAL:
    return disassembleGlobal(co, opcode, offset);
  case OP_GET_LOCAL:
  case OP_SET_LOCAL:
    return disassembleLocal(co, opcode, offset);
  case OP_GET_CELL:
  case OP_SET_CELL:
  case OP_LOAD_CELL:
    return disassembleCell(co, opcode, offset);
  case OP_MAKE_FUNCTION:
    return disassembleMakeFunction(co, opcode, offset);
  default:
    DIE << "disassembleInstruction: no disassembly for "
        << opcodeToString(opcode);
  }

  std::cout.flags(f);

  return 0;
}

size_t EvaDisassembler::disassembleSimple(CodeObject *co, uint8_t opcode,
                                          size_t offset) {
  dumpBytes(co, offset, 1);
  printOpCode(opcode);
  return offset + 1;
}

size_t EvaDisassembler::disassembleWord(CodeObject *co, uint8_t opcode,
                                        size_t offset) {
  dumpBytes(co, offset, 2);
  printOpCode(opcode);
  std::cout << (int)co->code[offset + 1];
  return offset + 2;
}

size_t EvaDisassembler::disassembleConst(CodeObject *co, uint8_t opcode,
                                         size_t offset) {
  dumpBytes(co, offset, 2);
  printOpCode(opcode);
  auto constIndex = co->code[offset + 1];
  std::cout << (int)constIndex << " ("
            << evaValueToConstantString(co->constants[constIndex]) << ")";
  return offset + 2;
}

size_t EvaDisassembler::disassembleCompare(CodeObject *co, uint8_t opcode,
                                           size_t offset) {
  dumpBytes(co, offset, 2);
  printOpCode(opcode);
  auto compareOp = co->code[offset + 1];
  std::cout << (int)compareOp << " (" << inverseCompareOps_[compareOp] << ")";
  return offset + 2;
}

size_t EvaDisassembler::disassembleJump(CodeObject *co, uint8_t opcode,
                                        size_t offset) {
  std::ios_base::fmtflags f(std::cout.flags());
  dumpBytes(co, offset, 3);
  printOpCode(opcode);
  uint16_t address = readWordAtOffset(co, offset + 1);

  std::cout << std::uppercase << std::hex << std::setfill('0') << std::setw(4)
            << (int)address << " ";

  std::cout.flags(f);
  return offset + 3;
}

size_t EvaDisassembler::disassembleGlobal(CodeObject *co, uint8_t opcode,
                                          size_t offset) {
  dumpBytes(co, offset, 2);
  printOpCode(opcode);
  auto globalIndex = co->code[offset + 1];
  std::cout << (int)globalIndex << " (" << global->get(globalIndex).name << ")";
  return offset + 2;
}

size_t EvaDisassembler::disassembleLocal(CodeObject *co, uint8_t opcode,
                                         size_t offset) {
  dumpBytes(co, offset, 2);
  printOpCode(opcode);
  auto localIndex = co->code[offset + 1];
  std::cout << (int)localIndex << " (" << co->locals[localIndex].name << ")";
  return offset + 2;
}

size_t EvaDisassembler::disassembleCell(CodeObject *co, uint8_t opcode,
                                        size_t offset) {
  dumpBytes(co, offset, 2);
  printOpCode(opcode);
  auto cellIndex = co->code[offset + 1];
  std::cout << (int)cellIndex << " (" << co->cellNames[cellIndex] << ")";
  return offset + 2;
}

size_t EvaDisassembler::disassembleMakeFunction(CodeObject *co, uint8_t opcode,
                                                size_t offset) {
  return disassembleWord(co, opcode, offset);
}

uint16_t EvaDisassembler::readWordAtOffset(CodeObject *co, size_t offset) {
  return (uint16_t)((co->code[offset] << 8) | co->code[offset + 1]);
}

void EvaDisassembler::dumpBytes(CodeObject *co, size_t offset, size_t count) {
  std::ios_base::fmtflags f(std::cout.flags());
  std::stringstream ss;

  for (auto i = 0; i < count; i++) {
    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
       << (((int)co->code[offset + i]) & 0xff) << " ";
  }

  std::cout << std::left << std::setfill(' ') << std::setw(12) << ss.str();
  std::cout.flags(f);
}

void EvaDisassembler::printOpCode(uint8_t opcode) {
  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << std::left << std::setfill(' ') << std::setw(20)
            << opcodeToString(opcode) << " ";
  std::cout.flags(f);
}

std::array<std::string, 6> EvaDisassembler::inverseCompareOps_ = {
    "<", ">", "==", ">=", "<=", "!=",
};
