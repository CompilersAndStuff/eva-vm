#ifndef __EvaDisassembler_h
#define __EvaDisassembler_h

#include "../vm/EvaValue.h"
#include "../vm/Global.h"
#include <memory>

class EvaDisassembler {
public:
  EvaDisassembler(std::shared_ptr<Global> global);
  void disassemble(CodeObject *co);

private:
  std::shared_ptr<Global> global;
  size_t disassembleInstruction(CodeObject *co, size_t offset);
  size_t disassembleSimple(CodeObject *co, uint8_t opcode, size_t offset);
  size_t disassembleWord(CodeObject *co, uint8_t opcode, size_t offset);
  size_t disassembleConst(CodeObject *co, uint8_t opcode, size_t offset);
  size_t disassembleCompare(CodeObject *co, uint8_t opcode, size_t offset);
  static std::array<std::string, 6> inverseCompareOps_;
  size_t disassembleJump(CodeObject *co, uint8_t opcode, size_t offset);
  size_t disassembleGlobal(CodeObject *co, uint8_t opcode, size_t offset);
  size_t disassembleLocal(CodeObject *co, uint8_t opcode, size_t offset);
  size_t disassembleCell(CodeObject *co, uint8_t opcode, size_t offset);
  size_t disassembleMakeFunction(CodeObject *co, uint8_t opcode, size_t offset);
  uint16_t readWordAtOffset(CodeObject *co, size_t offset);
  void dumpBytes(CodeObject *co, size_t offset, size_t count);
  void printOpCode(uint8_t opcode);
};

#endif // __EvaDisassembler_h
