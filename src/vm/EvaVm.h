#ifndef __EvaVM_h
#define __EvaVM_h

#include "../compiler/EvaCompiler.h"
#include "../gc/EvaCollector.h"
#include "EvaValue.h"
#include "Global.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>

#define READ_BYTE() *ip++
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define TO_ADDRESS(index) &fn->co->code[index]
#define STACK_LIMIT 512
#define GET_CONST() fn->co->constants[READ_BYTE()]

#define BINARY_OP(op)                                                          \
  do {                                                                         \
    auto op2 = AS_NUMBER(pop());                                               \
    auto op1 = AS_NUMBER(pop());                                               \
    push(NUMBER(op1 op op2));                                                  \
  } while (false)

#define COMPARE_VALUES(op, v1, v2)                                             \
  do {                                                                         \
    bool res;                                                                  \
    switch (op) {                                                              \
    case 0:                                                                    \
      res = v1 < v2;                                                           \
      break;                                                                   \
    case 1:                                                                    \
      res = v1 > v2;                                                           \
      break;                                                                   \
    case 2:                                                                    \
      res = v1 == v2;                                                          \
      break;                                                                   \
    case 3:                                                                    \
      res = v1 >= v2;                                                          \
      break;                                                                   \
    case 4:                                                                    \
      res = v1 <= v2;                                                          \
      break;                                                                   \
    case 5:                                                                    \
      res = v1 != v2;                                                          \
      break;                                                                   \
    }                                                                          \
    push(BOOLEAN(res));                                                        \
  } while (false)

struct Frame {
  uint8_t *ra;
  EvaValue *bp;
  FunctionObject *fn;
};

class EvaVm {
public:
  EvaVm();

  ~EvaVm();

  void push(const EvaValue &value);

  EvaValue peek(size_t offset = 0);

  EvaValue pop();

  void popN(size_t count);

  std::set<Traceable *> getGCRoots();

  std::set<Traceable *> getStackGCRoots();

  std::set<Traceable *> getConstantGCRoots();

  std::set<Traceable *> getGlobalGCRoots();

  void maybeGC();

  EvaValue exec(const std::string &program);

  EvaValue eval();

  void setGlobalVariables();

  std::shared_ptr<Global> global;

  std::unique_ptr<EvaCompiler> compiler;

  std::unique_ptr<EvaCollector> collector;

  uint8_t *ip;

  EvaValue *sp;

  EvaValue *bp;

  std::array<EvaValue, STACK_LIMIT> stack;

  std::stack<Frame> callStack;

  FunctionObject *fn;

  void dumpStack();
};

#endif // !__EvaVM_h
