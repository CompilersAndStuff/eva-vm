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

uint8_t EvaVm::readByte() { return *ip++; }

uint16_t EvaVm::readShort() {
  ip += 2;
  return (((uint16_t)ip[-2]) << 8) | ip[-1];
}

uint8_t *EvaVm::toAddress(size_t index) { return &fn->co->code[index]; }

EvaValue &EvaVm::getConst() { return fn->co->constants[readByte()]; }

void EvaVm::binaryOp(double (*op)(double, double)) {
  auto op2 = asNumber(pop());
  auto op1 = asNumber(pop());
  push(makeNumber(op(op1, op2)));
}

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
    if (isObject(*stackEntry)) {
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
    if (isObject(global.value)) {
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

  /*std::cout << "---------- BEFORE GC STATS ----------\n";*/
  /*Traceable::printStats();*/
  collector->gc(roots);
  /*std::cout << "---------- AFTER GC STATS ----------\n";*/
  /*Traceable::printStats();*/
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
    auto opcode = readByte();
    switch (static_cast<OpCode>(opcode)) {
    case OpCode::HALT:
      return pop();

    case OpCode::CONST:
      push(getConst());
      break;

    case OpCode::ADD: {
      auto op2 = pop();
      auto op1 = pop();

      if (isNumber(op1) && isNumber(op2)) {
        auto v1 = asNumber(op1);
        auto v2 = asNumber(op2);
        push(makeNumber(v1 + v2));
      }

      else if (isString(op1) && isString(op2)) {
        auto v1 = asCppString(op1);
        auto v2 = asCppString(op2);
        maybeGC();
        push(allocString(v1 + v2));
      }

      break;
    }

    case OpCode::SUB:
      binaryOp([](auto a, auto b) { return a - b; });
      break;

    case OpCode::MUL:
      binaryOp([](auto a, auto b) { return a * b; });
      break;

    case OpCode::DIV:
      binaryOp([](auto a, auto b) { return a / b; });
      break;

    case OpCode::COMPARE: {
      auto op = readByte();
      auto op2 = pop();
      auto op1 = pop();

      if (isNumber(op1) && isNumber(op2)) {
        auto v1 = asNumber(op1);
        auto v2 = asNumber(op2);
        compareValues(op, v1, v2);
      } else if (isString(op1) && isString(op2)) {
        auto v1 = asCppString(op1);
        auto v2 = asCppString(op2);
        compareValues(op, v1, v2);
      }
      break;
    }
    case OpCode::JMP_IF_FALSE: {
      auto cond = asBoolean(pop());
      auto address = readShort();

      if (!cond) {
        ip = toAddress(address);
      }

      break;
    }

    case OpCode::JMP: {
      ip = toAddress(readShort());
      break;
    }

    case OpCode::GET_GLOBAL: {
      auto globalIndex = readByte();
      push(global->get(globalIndex).value);
      break;
    }

    case OpCode::SET_GLOBAL: {
      auto globalIndex = readByte();
      auto value = pop();
      global->set(globalIndex, value);
      break;
    }

    case OpCode::POP: {
      pop();
      break;
    }

    case OpCode::GET_LOCAL: {
      auto localIndex = readByte();
      if (localIndex < 0 || localIndex >= stack.size()) {
        DIE << "OP_GET_LOCAL: invalid variable index: " << (int)localIndex;
      }
      push(bp[localIndex]);
      break;
    }

    case OpCode::SET_LOCAL: {
      auto localIndex = readByte();
      auto value = peek(0);
      if (localIndex < 0 || localIndex >= stack.size()) {
        DIE << "OP_SET_LOCAL: invalid variable index: " << (int)localIndex;
      }
      bp[localIndex] = value;
      break;
    }

    case OpCode::SCOPE_EXIT: {
      auto count = readByte();

      *(sp - 1 - count) = peek(0);

      popN(count);
      break;
    }

    case OpCode::CALL: {
      auto argsCount = readByte();
      auto fnValue = peek(argsCount);

      if (isNative(fnValue)) {
        asNative(fnValue)->function();
        auto result = pop();

        popN(argsCount + 1);

        push(result);

        break;
      }

      auto callee = asFunction(fnValue);

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
      auto cellIndex = readByte();
      push(fn->cells[cellIndex]->value);
      break;
    }

    case OpCode::SET_CELL: {
      auto cellIndex = readByte();
      auto value = peek(0);
      if (fn->cells.size() <= cellIndex) {
        maybeGC();
        fn->cells.push_back(asCell(allocCell(value)));
      } else {
        fn->cells[cellIndex]->value = value;
      }
      break;
    }

    case OpCode::LOAD_CELL: {
      auto cellIndex = readByte();
      push(cell(fn->cells[cellIndex]));
      break;
    }

    case OpCode::MAKE_FUNCTION: {
      auto co = asCode(pop());
      auto cellsCount = readByte();
      maybeGC();
      auto fnValue = allocFunction(co);
      auto fn = asFunction(fnValue);

      for (auto i = 0; i < cellsCount; i++) {
        fn->cells.push_back(asCell(pop()));
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
        auto x = asNumber(peek(0));
        push(makeNumber(x * x));
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
