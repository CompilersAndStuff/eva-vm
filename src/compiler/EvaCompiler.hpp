#ifndef __EvaCompiler_hpp
#define __EvaCompiler_hpp

#include "../bytecode/OpCode.hpp"
#include "../disassembler/EvaDisassembler.hpp"
#include "../parser/EvaParser.hpp"
#include "../vm/EvaValue.hpp"
#include "../vm/Global.hpp"
#include <cstdint>
#include <memory>
#include <vector>

#define ALLOC_CONST(tester, converter, allocator, value)                       \
  do {                                                                         \
    for (auto i = 0; i < co->constants.size(); i++) {                          \
      if (!tester(co->constants[i])) {                                         \
        continue;                                                              \
      }                                                                        \
      if (converter(co->constants[i]) == value) {                              \
        return i;                                                              \
      }                                                                        \
    }                                                                          \
    co->constants.push_back(allocator(value));                                 \
  } while (false)

#define GEN_BINARY_OP(op)                                                      \
  do {                                                                         \
    gen(exp.list[1]);                                                          \
    gen(exp.list[2]);                                                          \
    emit(op);                                                                  \
  } while (false)

class EvaCompiler {
public:
  EvaCompiler(std::shared_ptr<Global> global)
      : global(global),
        disassembler(std::make_unique<EvaDisassembler>(global)) {}

  CodeObject *compile(const Exp &exp) {
    co = AS_CODE(ALLOC_CODE("main"));

    gen(exp);
    emit(OP_HALT);

    return co;
  }

  void gen(const Exp &exp) {
    switch (exp.type) {
    case ExpType::NUMBER:
      emit(OP_CONST);
      emit(numericConstIdx(exp.number));
      break;

    case ExpType::STRING:
      emit(OP_CONST);
      emit(stringConstIdx(exp.string));
      break;

    case ExpType::SYMBOL:
      if (exp.string == "true" || exp.string == "false") {
        emit(OP_CONST);
        emit(booleanConstIdx(exp.string == "true" ? true : false));
      } else {
        auto varName = exp.string;

        auto localIndex = co->getLocalIndex(varName);

        if (localIndex != -1) {
          emit(OP_GET_LOCAL);
          emit(localIndex);
        } else {
          if (!global->exists(varName)) {
            DIE << "[EvaCompiler]: Reference error:" << varName;
          }

          emit(OP_GET_GLOBAL);
          emit(global->getGlobalIndex(varName));
        }
      }
      break;

    case ExpType::LIST:
      auto tag = exp.list[0];

      if (tag.type == ExpType::SYMBOL) {
        auto op = tag.string;

        if (op == "+") {
          GEN_BINARY_OP(OP_ADD);
        } else if (op == "-") {
          GEN_BINARY_OP(OP_SUB);
        } else if (op == "*") {
          GEN_BINARY_OP(OP_MUL);
        } else if (op == "/") {
          GEN_BINARY_OP(OP_DIV);
        } else if (compareOps_.count(op) != 0) {
          gen(exp.list[1]);
          gen(exp.list[2]);
          emit(OP_COMPARE);
          emit(compareOps_[op]);
        } else if (op == "if") {
          gen(exp.list[1]); // Generate tester expr
          emit(OP_JMP_IF_FALSE);
          emit(0); // Set a placeholder for the address that will later point to
                   // the beginning of the ELSE branch
          emit(0); // The placeholder consists of two consecutive 8-bit cells
                   // representing a 16-bit address
          auto elseJmpPlaceholderAddr =
              getOffset() - 2; // Store the address of the placeholder
          gen(exp.list[2]);    // Generate the consequent branch
          emit(OP_JMP);
          emit(0); // Set a placeholder for the address that will later point to
                   // the beginning of the ELSE branch
          emit(0); // The placeholder consists of two consecutive 8-bit cells
                   // representing a 16-bit address
          auto jmpPlaceholderAddr = getOffset() - 2;

          auto elseBranchBeginAddr = getOffset();
          patchJumpAddress(elseJmpPlaceholderAddr, elseBranchBeginAddr);

          if (exp.list.size() == 4) {
            gen(exp.list[3]);
          }

          auto elseBranchEndAddr = getOffset();
          patchJumpAddress(jmpPlaceholderAddr, elseBranchEndAddr);
        } else if (op == "var") {
          auto varName = exp.list[1].string;

          gen(exp.list[2]);

          if (isGlobalScope()) {
            global->define(varName);
            emit(OP_SET_GLOBAL);
            emit(global->getGlobalIndex(varName));
          }

          else {
            co->addLocal(varName);
            emit(OP_SET_LOCAL);
            emit(co->getLocalIndex(varName));
          }

        } else if (op == "set") {
          auto varName = exp.list[1].string;

          gen(exp.list[2]);

          auto localIndex = co->getLocalIndex(varName);

          if (localIndex != -1) {
            emit(OP_SET_LOCAL);
            emit(localIndex);

          } else {
            auto globalIndex = global->getGlobalIndex(exp.list[1].string);
            if (globalIndex == -1) {
              DIE << "Reference error: " << varName << " is not defined.";
            }
            emit(OP_SET_GLOBAL);
            emit(globalIndex);
          }

        } else if (op == "begin") {
          scopeEnter();

          for (auto i = 1; i < exp.list.size(); i++) {
            bool isLast = i == exp.list.size() - 1;

            gen(exp.list[i]);

            if (!isLast && !isWhileLoop(exp.list[i])) {
              emit(OP_POP);
            }
          }

          scopeExit();
        }

        else if (op == "while") {
          auto loopStartAddr = getOffset();
          gen(exp.list[1]); // Tester expr
          emit(OP_JMP_IF_FALSE);
          emit(0); // Placeholder for the address of the loop end
          emit(0);
          auto loopEndJmpPlaceholderAddr = getOffset() - 2;
          auto loopBody = exp.list[2];
          if (!isTaggedList(
                  exp.list[2],
                  "begin")) { // if loop body is not wrapped in begin block
            std::vector<Exp> beginList;
            std::string beginStr = "begin";
            beginList.push_back(Exp(beginStr));
            beginList.push_back(exp.list[2]);
            loopBody = Exp(beginList);
          }
          gen(exp.list[2]); // Loop body
          emit(OP_JMP);
          emit(0); // Placeholder for the address of the loop start
          emit(0);
          patchJumpAddress(loopEndJmpPlaceholderAddr, getOffset());
          patchJumpAddress(getOffset() - 2, loopStartAddr);
        }
      }
    }
  }

  void disassembleBytecode() { disassembler->disassemble(co); }

private:
  std::shared_ptr<Global> global;
  std::unique_ptr<EvaDisassembler> disassembler;

  void scopeEnter() { co->scopeLevel++; }
  void scopeExit() {
    auto varsCount = getVarsCountOnScopeExit();
    if (varsCount > 0) {
      emit(OP_SCOPE_EXIT);
      emit(varsCount);
    }
    co->scopeLevel--;
  }

  bool isGlobalScope() { return co->name == "main" && co->scopeLevel == 1; }

  bool isWhileLoop(const Exp &exp) { return isTaggedList(exp, "while"); }

  bool isVarDeclaration(const Exp &exp) { return isTaggedList(exp, "var"); }

  bool isTaggedList(const Exp &exp, const std::string &tag) {
    return exp.type == ExpType::LIST && exp.list[0].type == ExpType::SYMBOL &&
           exp.list[0].string == tag;
  }

  size_t getVarsCountOnScopeExit() {
    auto varsCount = 0;

    if (co->locals.size() > 0) {
      while (co->locals.back().scopeLevel == co->scopeLevel) {
        co->locals.pop_back();
        varsCount++;
      }
    }

    return varsCount;
  }

  size_t getOffset() { return co->code.size(); }

  size_t numericConstIdx(double value) {
    ALLOC_CONST(IS_NUMBER, AS_NUMBER, NUMBER, value);
    return co->constants.size() - 1;
  }

  size_t stringConstIdx(const std::string &value) {
    ALLOC_CONST(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
    return co->constants.size() - 1;
  }

  size_t booleanConstIdx(bool value) {
    ALLOC_CONST(IS_BOOLEAN, AS_BOOLEAN, BOOLEAN, value);
    return co->constants.size() - 1;
  }

  void emit(uint8_t code) { co->code.push_back(code); }

  void writeByteAtOffset(size_t offset, uint8_t value) {
    co->code[offset] = value;
  }

  void patchJumpAddress(size_t offset, uint16_t value) {
    writeByteAtOffset(offset, (value >> 8) & 0xff);
    writeByteAtOffset(offset + 1, value & 0xff);
  }

  CodeObject *co;

  static std::map<std::string, uint8_t> compareOps_;
};

std::map<std::string, uint8_t> EvaCompiler::compareOps_ = {
    {"<", 0}, {">", 1}, {"==", 2}, {">=", 3}, {"<=", 4}, {"!=", 5},
};

#endif // __EvaCompiler_hpp
