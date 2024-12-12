#ifndef __EvaCompiler_h
#define __EvaCompiler_h

#include "../disassembler/EvaDisassembler.h"
#include "../vm/EvaValue.h"
#include "../vm/Global.h"
#include "../parser/Expression.h"
#include "Scope.h"
#include <cstdint>
#include <memory>
#include <set>
#include <stack>
#include <vector>

class EvaCompiler {
public:
  EvaCompiler(std::shared_ptr<Global> global);

  void compile(const Exp &exp);

  void analyze(const Exp &exp, std::shared_ptr<Scope> scope);

  void gen(const Exp &exp);

  void disassembleBytecode();

  FunctionObject *getMainFunction();

  std::set<Traceable *> &getConstantObjects();

private:
  std::shared_ptr<Global> global;
  std::unique_ptr<EvaDisassembler> disassembler;

  EvaValue createCodeObjectValue(const std::string &name, size_t arity = 0);

  void compileFunction(const Exp &exp, const std::string fnName,
                       const Exp &params, const Exp &body);

  void blockEnter();
  void blockExit();

  bool isGlobalScope();

  bool isFunctionBody();

  bool isWhileLoop(const Exp &exp);

  bool isBlock(const Exp &exp);

  bool isLambda(const Exp &exp);

  bool isVarDeclaration(const Exp &exp);

  bool isFunctionDeclaration(const Exp &exp);

  bool isTaggedList(const Exp &exp, const std::string &tag);

  size_t getVarsCountOnScopeExit();

  size_t getOffset();

  size_t numericConstIdx(double value);

  size_t stringConstIdx(const std::string &value);

  size_t booleanConstIdx(bool value);

  void emit(uint8_t code);

  void writeByteAtOffset(size_t offset, uint8_t value);

  void patchJumpAddress(size_t offset, uint16_t value);

  std::map<const Exp *, std::shared_ptr<Scope>> scopeInfo_;

  std::stack<std::shared_ptr<Scope>> scopeStack_;

  CodeObject *co;

  FunctionObject *main;

  std::vector<CodeObject *> codeObjects_;

  std::set<Traceable *> constantObjects_;

  static std::map<std::string, uint8_t> compareOps_;

  static std::set<std::string> keywords;
};

#endif // __EvaCompiler_h
