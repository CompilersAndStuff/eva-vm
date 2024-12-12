#include "EvaVm.h"
#include "../Logger.h"
#include "../bytecode/OpCode.h"
#include "../compiler/EvaCompiler.h"
#include "../parser/EvaParser.h"
#include "EvaValue.h"
#include "Global.h"
#include <array>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <memory>
#include <string>
#include <vector>

using syntax::EvaParser;

#define READ_BYTE() *ip++
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define TO_ADDRESS(index) &fn->co->code[index]
#define STACK_LIMIT 512
#define GC_TRESHOLD 417
#define GET_CONST() fn->co->constants[READ_BYTE()]
#define MEM(allocator, ...) (maybeGC(), allocator(__VA_ARGS__))

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

EvaVm::EvaVm()
    : global(std::make_unique<Global>()),
      compiler(std::make_unique<EvaCompiler>(global)),
      collector(std::make_unique<EvaCollector>()) {
  setGlobalVariables();
}

EvaVm::~EvaVm() { Traceable::cleanup(); }

void EvaVm::push(const EvaValue &value) {
  if ((size_t)(sp - stack.begin()) == STACK_LIMIT) {
    DIE << "push(): Stack overflow.\n";
  }
  *sp = value;
  sp++;
};

EvaValue EvaVm::peek(size_t offset) {
  if (stack.size() == 0) {
    DIE << "peek(): empty stack\n";
  }

  return *(sp - 1 - offset);
}

EvaValue EvaVm::pop() {
  if (sp == stack.begin()) {
    DIE << "pop(): empty stack.\n";
  }
  --sp;
  return *sp;
};

void EvaVm::popN(size_t count) {
  if (stack.size() == 0) {
    DIE << "popN(): empty stack.\n";
  }

  sp -= count;
}

std::set<Traceable *> EvaVm::getGCRoots() {
  auto roots = getStackGCRoots();

  auto constantRoots = getConstantGCRoots();
  roots.insert(constantRoots.begin(), constantRoots.end());

  auto globalRoots = getGlobalGCRoots();
  roots.insert(globalRoots.begin(), globalRoots.end());

  return roots;
}

std::set<Traceable *> EvaVm::getStackGCRoots() {
  std::set<Traceable *> roots;
  auto stackEntry = sp;
  while (stackEntry-- != stack.begin()) {
    if (IS_OBJECT(*stackEntry)) {
      roots.insert((Traceable *)stackEntry->object);
    }
  }

  return roots;
}

std::set<Traceable *> EvaVm::getConstantGCRoots() {
  return compiler->getConstantObjects();
}

std::set<Traceable *> EvaVm::getGlobalGCRoots() {
  std::set<Traceable *> roots;

  for (const auto &global : global->globals) {
    if (IS_OBJECT(global.value)) {
      roots.insert((Traceable *)global.value.object);
    }
  }

  return roots;
}

void EvaVm::maybeGC() {
  if (Traceable::bytesAllocated < GC_TRESHOLD) {
    return;
  }

  auto roots = getGCRoots();

  if (roots.size() == 0) {
    return;
  }

  std::cout << "---------- BEFORE GC STATS ----------\n";
  Traceable::printStats();
  collector->gc(roots);
  std::cout << "---------- AFTER GC STATS ----------\n";
  Traceable::printStats();
}

EvaValue EvaVm::exec(const std::string &program) {
  auto ast = EvaParser().parse("(begin" + program + ")");

  compiler->compile(ast);

  fn = compiler->getMainFunction();

  sp = &stack[0];

  bp = sp;

  ip = &fn->co->code[0];

  compiler->disassembleBytecode();

  return eval();
}

EvaValue EvaVm::eval() {
  for (;;) {
    /*dumpStack();*/
    auto opcode = READ_BYTE();
    switch (static_cast<OpCode>(opcode)) {
    case OpCode::HALT:
      return pop();

    case OpCode::CONST:
      push(GET_CONST());
      break;

    case OpCode::ADD: {
      auto op2 = pop();
      auto op1 = pop();

      if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
        auto v1 = AS_NUMBER(op1);
        auto v2 = AS_NUMBER(op2);
        push(NUMBER(v1 + v2));
      }

      else if (IS_STRING(op1) && IS_STRING(op2)) {
        auto v1 = AS_CPPSTRING(op1);
        auto v2 = AS_CPPSTRING(op2);
        push(MEM(ALLOC_STRING, v1 + v2));
      }

      break;
    }

    case OpCode::SUB:
      BINARY_OP(-);
      break;

    case OpCode::MUL:
      BINARY_OP(*);
      break;

    case OpCode::DIV:
      BINARY_OP(/);
      break;

    case OpCode::COMPARE: {
      auto op = READ_BYTE();
      auto op2 = pop();
      auto op1 = pop();

      if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
        auto v1 = AS_NUMBER(op1);
        auto v2 = AS_NUMBER(op2);
        COMPARE_VALUES(op, v1, v2);
      } else if (IS_STRING(op1) && IS_STRING(op2)) {
        auto v1 = AS_CPPSTRING(op1);
        auto v2 = AS_CPPSTRING(op2);
        COMPARE_VALUES(op, v1, v2);
      }
      break;
    }
    case OpCode::JMP_IF_FALSE: {
      auto cond = AS_BOOLEAN(pop());
      auto address = READ_SHORT();

      if (!cond) {
        ip = TO_ADDRESS(address);
      }

      break;
    }

    case OpCode::JMP: {
      ip = TO_ADDRESS(READ_SHORT());
      break;
    }

    case OpCode::GET_GLOBAL: {
      auto globalIndex = READ_BYTE();
      push(global->get(globalIndex).value);
      break;
    }

    case OpCode::SET_GLOBAL: {
      auto globalIndex = READ_BYTE();
      auto value = pop();
      global->set(globalIndex, value);
      break;
    }

    case OpCode::POP: {
      pop();
      break;
    }

    case OpCode::GET_LOCAL: {
      auto localIndex = READ_BYTE();
      if (localIndex < 0 || localIndex >= stack.size()) {
        DIE << "OP_GET_LOCAL: invalid variable index: " << (int)localIndex;
      }
      push(bp[localIndex]);
      break;
    }

    case OpCode::SET_LOCAL: {
      auto localIndex = READ_BYTE();
      auto value = peek(0);
      if (localIndex < 0 || localIndex >= stack.size()) {
        DIE << "OP_SET_LOCAL: invalid variable index: " << (int)localIndex;
      }
      bp[localIndex] = value;
      break;
    }

    case OpCode::SCOPE_EXIT: {
      auto count = READ_BYTE();

      *(sp - 1 - count) = peek(0);

      popN(count);
      break;
    }

    case OpCode::CALL: {
      auto argsCount = READ_BYTE();
      auto fnValue = peek(argsCount);

      if (IS_NATIVE(fnValue)) {
        AS_NATIVE(fnValue)->function();
        auto result = pop();

        popN(argsCount + 1);

        push(result);

        break;
      }

      auto callee = AS_FUNCTION(fnValue);

      callStack.push(Frame{ip, bp, fn});

      fn = callee;
      fn->cells.resize(fn->co->freeCount);
      bp = sp - argsCount - 1;
      ip = &callee->co->code[0];

      break;
    }

    case OpCode::RETURN: {
      auto callerFrame = callStack.top();
      ip = callerFrame.ra;
      bp = callerFrame.bp;
      fn = callerFrame.fn;
      callStack.pop();

      break;
    }

    case OpCode::GET_CELL: {
      auto cellIndex = READ_BYTE();
      push(fn->cells[cellIndex]->value);
      break;
    }

    case OpCode::SET_CELL: {
      auto cellIndex = READ_BYTE();
      auto value = peek(0);
      if (fn->cells.size() <= cellIndex) {
        fn->cells.push_back(AS_CELL(MEM(ALLOC_CELL, value)));
      } else {
        fn->cells[cellIndex]->value = value;
      }
      break;
    }

    case OpCode::LOAD_CELL: {
      auto cellIndex = READ_BYTE();
      push(CELL(fn->cells[cellIndex]));
      break;
    }

    case OpCode::MAKE_FUNCTION: {
      auto co = AS_CODE(pop());
      auto cellsCount = READ_BYTE();
      auto fnValue = MEM(ALLOC_FUNCTION, co);
      auto fn = AS_FUNCTION(fnValue);

      for (auto i = 0; i < cellsCount; i++) {
        fn->cells.push_back(AS_CELL(pop()));
      }
      push(fnValue);
      break;
    }

    default:
      DIE << "Unknown opcode: " << std::hex << std::setw(2) << std::uppercase
          << std::setfill('0') << (int)opcode;
    }
  }
}

void EvaVm::setGlobalVariables() {
  global->addNativeFunction(
      "native-square",
      [&]() {
        auto x = AS_NUMBER(peek(0));
        push(NUMBER(x * x));
      },
      1);

  global->addConst("VERSION", 1);
}

void EvaVm::dumpStack() {
  std::cout << "\n-------------- Stack --------------\n";
  if (sp == stack.begin()) {
    std::cout << "(empty)";
  }
  auto csp = sp - 1;
  while (csp >= stack.begin()) {
    std::cout << *csp-- << "\n";
  }
  std::cout << "\n";
}
