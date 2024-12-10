#ifndef __EvaVM_h
#define __EvaVM_h

#include "../compiler/EvaCompiler.h"
#include "EvaValue.h"
#include "Global.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>

constexpr size_t STACK_LIMIT = 512;

struct Frame {
  uint8_t *ra;
  EvaValue *bp;
  FunctionObject *fn;
};

class EvaVm {
  uint8_t readByte();
  uint16_t readShort();
  uint8_t *toAddress(size_t index);
  EvaValue &getConst();

  void binaryOp(double (*op)(double, double));

  template <typename T>
  void compareValues(uint8_t &op, T v1, T v2) {
    bool res;
    switch (op) {
    case 0:
      res = v1 < v2;
      break;
    case 1:
      res = v1 > v2;
      break;
    case 2:
      res = v1 == v2;
      break;
    case 3:
      res = v1 >= v2;
      break;
    case 4:
      res = v1 <= v2;
      break;
    case 5:
      res = v1 != v2;
      break;
    }
    push(makeBoolean(res));
  }

public:
  EvaVm();

  void push(const EvaValue &value);

  EvaValue peek(size_t offset = 0);

  EvaValue pop();

  void popN(size_t count);

  EvaValue exec(const std::string &program);

  EvaValue eval();

  void setGlobalVariables();

  std::shared_ptr<Global> global;

  std::unique_ptr<EvaCompiler> compiler;

  uint8_t *ip;

  EvaValue *sp;

  EvaValue *bp;

  std::array<EvaValue, STACK_LIMIT> stack;

  std::stack<Frame> callStack;

  FunctionObject *fn;

  void dumpStack();
};

#endif // !__EvaVM_h
