#ifndef __EvaCompiler_hpp
#define __EvaCompiler_hpp

#include "../bytecode/OpCode.hpp"
#include "../disassembler/EvaDisassembler.hpp"
#include "../parser/EvaParser.hpp"
#include "../vm/EvaValue.hpp"
#include "../vm/Global.hpp"
#include "Scope.hpp"
#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#define FUNCTION_CALL(exp)                                                     \
  do {                                                                         \
    gen(exp.list[0]);                                                          \
    for (auto i = 1; i < exp.list.size(); i++) {                               \
      gen(exp.list[i]);                                                        \
    }                                                                          \
    emit(OP_CALL);                                                             \
    emit(exp.list.size() - 1);                                                 \
  } while (false)

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
    co->addConst(allocator(value));                                            \
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

  void compile(const Exp &exp) {
    co = AS_CODE(createCodeObjectValue("main"));
    constantObjects_.insert((Traceable *)co);
    main = AS_FUNCTION(ALLOC_FUNCTION(co));
    constantObjects_.insert((Traceable *)main);

    analyze(exp, nullptr);

    gen(exp);
    emit(OP_HALT);
  }

  void analyze(const Exp &exp, std::shared_ptr<Scope> scope) {
    if (exp.type == ExpType::SYMBOL) {
      if (exp.string == "true" || exp.string == "false") {
      } else {
        scope->maybePromote(exp.string);
      }
    } else if (exp.type == ExpType::LIST) {
      auto tag = exp.list[0];

      if (tag.type == ExpType::SYMBOL) {
        auto op = tag.string;

        if (op == "begin") {
          auto newScope = std::make_shared<Scope>(
              scope == nullptr ? ScopeType::GLOBAL : ScopeType::BLOCK, scope);

          scopeInfo_[&exp] = newScope;

          for (auto i = 1; i < exp.list.size(); ++i) {
            analyze(exp.list[i], newScope);
          }
        } else if (op == "var") {
          scope->addLocal(exp.list[1].string);
          analyze(exp.list[2], scope);
        } else if (op == "def") {
          auto fnName = exp.list[1].string;
          scope->addLocal(fnName);

          auto newScope = std::make_shared<Scope>(ScopeType::FUNCTION, scope);

          scopeInfo_[&exp] = newScope;

          auto arity = exp.list[2].list.size();

          for (auto i = 0; i < arity; i++) {
            newScope->addLocal(exp.list[2].list[i].string);
          }

          analyze(exp.list[3], newScope);
        } else if (op == "lambda") {
          auto newScope = std::make_shared<Scope>(ScopeType::FUNCTION, scope);
          scopeInfo_[&exp] = newScope;

          auto arity = exp.list[1].list.size();

          for (auto i = 0; i < arity; i++) {
            newScope->addLocal(exp.list[1].list[i].string);
          }
          analyze(exp.list[2], newScope);
        } else if (keywords.count(op) != 0) {
          for (auto i = 1; i < exp.list.size(); ++i) {
            analyze(exp.list[i], scope);
          }
        } else {
          for (auto i = 0; i < exp.list.size(); ++i) {
            analyze(exp.list[i], scope);
          }
        }
      } else {
        for (auto i = 0; i < exp.list.size(); ++i) {
          analyze(exp.list[i], scope);
        }
      }
    }
  };

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

        auto opCodeGetter = scopeStack_.top()->getNameGetter(varName);
        emit(opCodeGetter);

        if (opCodeGetter == OP_GET_LOCAL) {
          emit(co->getLocalIndex(varName));
        } else if (opCodeGetter == OP_GET_CELL) {
          emit(co->getCellIndex(varName));
        } else {
          if (!global->exists(varName)) {
            DIE << "[EvaCompiler]: Reference error:" << varName;
          }
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
          emit(0); // Set a placeholder for the address that will later point
                   // to the beginning of the ELSE branch
          emit(0); // The placeholder consists of two consecutive 8-bit cells
                   // representing a 16-bit address
          auto elseJmpPlaceholderAddr =
              getOffset() - 2; // Store the address of the placeholder
          gen(exp.list[2]);    // Generate the consequent branch
          emit(OP_JMP);
          emit(0); // Set a placeholder for the address that will later point
                   // to the beginning of the ELSE branch
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

          auto opCodeSetter = scopeStack_.top()->getNameSetter(varName);

          if (isLambda(exp.list[2])) {
            compileFunction(exp.list[2], varName, exp.list[2].list[1],
                            exp.list[2].list[2]);
          } else {
            gen(exp.list[2]);
          }

          if (opCodeSetter == OP_SET_GLOBAL) {
            global->define(varName);
            emit(OP_SET_GLOBAL);
            emit(global->getGlobalIndex(varName));
          } else if (opCodeSetter == OP_SET_CELL) {
            co->cellNames.push_back(varName);
            emit(OP_SET_CELL);
            emit(co->cellNames.size() - 1);
            emit(OP_POP);
          } else {
            co->addLocal(varName);
          }

        } else if (op == "set") {
          auto varName = exp.list[1].string;

          auto opCodeSetter = scopeStack_.top()->getNameSetter(varName);

          gen(exp.list[2]);

          if (opCodeSetter == OP_SET_LOCAL) {
            emit(OP_SET_LOCAL);
            emit(co->getLocalIndex(varName));
          } else if (opCodeSetter == OP_SET_CELL) {
            emit(OP_SET_CELL);
            emit(co->getCellIndex(varName));
          } else {
            auto globalIndex = global->getGlobalIndex(exp.list[1].string);
            if (globalIndex == -1) {
              DIE << "Reference error: " << varName << " is not defined.";
            }
            emit(OP_SET_GLOBAL);
            emit(globalIndex);
          }

        } else if (op == "begin") {
          scopeStack_.push(scopeInfo_.at(&exp));
          blockEnter();

          for (auto i = 1; i < exp.list.size(); i++) {
            bool isLast = i == exp.list.size() - 1;

            gen(exp.list[i]);

            auto isDecl = isVarDeclaration(exp.list[i]) ||
                          isFunctionDeclaration(exp.list[i]);

            if (!isLast && !isWhileLoop(exp.list[i]) && !isDecl) {
              emit(OP_POP);
            }

            if (isLast && isVarDeclaration(exp.list[i])) {
              gen(exp.list[i].list[1]);
            }
          }

          blockExit();
          scopeStack_.pop();
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
        } else if (op == "def") {
          auto fnName = exp.list[1].string;

          compileFunction(exp, fnName, exp.list[2], exp.list[3]);

          if (isGlobalScope()) {
            global->define(fnName);
            emit(OP_SET_GLOBAL);
            emit(global->getGlobalIndex(fnName));
          } else {
            co->addLocal(fnName);
          }
        } else if (op == "lambda") {
          compileFunction(exp, "lambda", exp.list[1], exp.list[2]);

        } else {
          FUNCTION_CALL(exp);
        }
      } else {
        FUNCTION_CALL(exp);
      }
    }
  }

  void disassembleBytecode() {
    for (auto &co_ : codeObjects_) {
      disassembler->disassemble(co_);
    }
  }

  FunctionObject *getMainFunction() { return main; }

  std::set<Traceable *> &getConstantObjects() { return constantObjects_; }

private:
  std::shared_ptr<Global> global;
  std::unique_ptr<EvaDisassembler> disassembler;

  EvaValue createCodeObjectValue(const std::string &name, size_t arity = 0) {
    auto coValue = ALLOC_CODE(name, arity);
    auto co = AS_CODE(coValue);
    codeObjects_.push_back(co);
    constantObjects_.insert((Traceable *)co);
    return coValue;
  }

  void compileFunction(const Exp &exp, const std::string fnName,
                       const Exp &params, const Exp &body) {

    auto scopeInfo = scopeInfo_.at(&exp);
    scopeStack_.push(scopeInfo);

    auto arity = params.list.size();
    auto prevCo = co;
    auto coValue = createCodeObjectValue(fnName, arity);
    co = AS_CODE(coValue);

    co->freeCount = scopeInfo->free.size();

    co->cellNames.reserve(scopeInfo->free.size() + scopeInfo->cells.size());

    co->cellNames.insert(co->cellNames.end(), scopeInfo->free.begin(),
                         scopeInfo->free.end());
    co->cellNames.insert(co->cellNames.end(), scopeInfo->cells.begin(),
                         scopeInfo->cells.end());

    prevCo->addConst(coValue);

    co->addLocal(fnName);

    for (auto i = 0; i < arity; i++) {
      auto argName = params.list[i].string;
      co->addLocal(argName);

      auto cellIndex = co->getCellIndex(argName);
      if (cellIndex != -1) {
        emit(OP_SET_CELL);
        emit(cellIndex);
        emit(OP_POP);
      } else {
        co->nonCellFnParams++;
      }
    }

    gen(body);

    if (!isBlock(body)) {
      emit(OP_SCOPE_EXIT);
      emit(1 /*function itself*/ + co->nonCellFnParams);
    }

    emit(OP_RETURN);

    if (scopeInfo->free.size() == 0) {
      auto fn = ALLOC_FUNCTION(co);
      constantObjects_.insert((Traceable *)AS_OBJECT(fn));

      co = prevCo;

      co->addConst(fn);

      emit(OP_CONST);
      emit(co->constants.size() - 1);
    } else {
      co = prevCo;

      for (const auto &freeVar : scopeInfo->free) {
        emit(OP_LOAD_CELL);
        emit(prevCo->getCellIndex(freeVar));
      }

      emit(OP_CONST);
      emit(co->constants.size() - 1);

      emit(OP_MAKE_FUNCTION);
      emit(scopeInfo->free.size());
    }

    scopeStack_.pop();
  }

  void blockEnter() { co->scopeLevel++; }
  void blockExit() {
    auto varsCount = getVarsCountOnScopeExit();
    if (varsCount > 0 || co->arity > 0) {
      emit(OP_SCOPE_EXIT);

      if (isFunctionBody()) {
        varsCount += 1 /*Function itself*/ + co->nonCellFnParams;
      }

      emit(varsCount);
    }
    co->scopeLevel--;
  }

  bool isGlobalScope() { return co->name == "main" && co->scopeLevel == 1; }

  bool isFunctionBody() { return co->name != "main" && co->scopeLevel == 1; }

  bool isWhileLoop(const Exp &exp) { return isTaggedList(exp, "while"); }

  bool isBlock(const Exp &exp) { return isTaggedList(exp, "begin"); }

  bool isLambda(const Exp &exp) { return isTaggedList(exp, "lambda"); }

  bool isVarDeclaration(const Exp &exp) { return isTaggedList(exp, "var"); }

  bool isFunctionDeclaration(const Exp &exp) {
    return isTaggedList(exp, "def");
  }

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
    constantObjects_.insert((Traceable*)co->constants.back().object);
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

  std::map<const Exp *, std::shared_ptr<Scope>> scopeInfo_;

  std::stack<std::shared_ptr<Scope>> scopeStack_;

  CodeObject *co;

  FunctionObject *main;

  std::vector<CodeObject *> codeObjects_;

  std::set<Traceable *> constantObjects_;

  static std::map<std::string, uint8_t> compareOps_;

  static std::set<std::string> keywords;
};

std::map<std::string, uint8_t> EvaCompiler::compareOps_ = {
    {"<", 0}, {">", 1}, {"==", 2}, {">=", 3}, {"<=", 4}, {"!=", 5},
};

std::set<std::string> EvaCompiler::keywords = {
    "var", "set", "def", "begin", "while", "if", "lambda", "+", "-",
    "*",   "/",   "<",   ">",     "==",    ">=", "<=",     "!="};

#endif // __EvaCompiler_hpp
