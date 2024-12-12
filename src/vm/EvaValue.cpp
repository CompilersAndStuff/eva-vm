#include "EvaValue.h"
#include "../Logger.h"
#include <functional>
#include <sstream>
#include <string>
#include <vector>

void *Traceable::operator new(size_t size) {
  void *object = ::operator new(size);
  ((Traceable *)object)->size = size;
  Traceable::objects.push_back((Traceable *)object);
  Traceable::bytesAllocated += size;
  return object;
}

void Traceable::operator delete(void *object, std::size_t sz) {
  Traceable::bytesAllocated -= ((Traceable *)object)->size;
  ::operator delete(object, sz);
}

void Traceable::cleanup() {
  for (auto &object : objects) {
    delete object;
  }
  objects.clear();
}

void Traceable::printStats() {
  std::cout << "---------------------------\n";
  std::cout << "Memory stats:\n\n";
  std::cout << "Objects allocated: " << std::dec << Traceable::objects.size()
            << "\n";
  std::cout << "Bytes allocated: " << std::dec << Traceable::bytesAllocated
            << "\n\n";
}

size_t Traceable::bytesAllocated{0};

std::list<Traceable *> Traceable::objects{};

Object::Object(ObjectType type) : type(type) {}

using NativeFn = std::function<void()>;

NativeObject::NativeObject(NativeFn function, const std::string &name,
                           size_t arity)
    : Object(ObjectType::NATIVE), function(function), name(name), arity(arity) {
}

StringObject::StringObject(const std::string &str)
    : Object(ObjectType::STRING), string(str) {}

CodeObject::CodeObject(const std::string &name, size_t arity)
    : Object(ObjectType::CODE), name(name), arity(arity) {}

void CodeObject::addLocal(const std::string &name) {
  locals.push_back({name, scopeLevel});
}

void CodeObject::addConst(const EvaValue &value) { constants.push_back(value); }

int CodeObject::getLocalIndex(const std::string &name) {
  if (locals.size() > 0) {
    for (auto i = (int)locals.size() - 1; i >= 0; i--) {
      if (locals[i].name == name) {
        return i;
      }
    }
  }
  return -1;
}

int CodeObject::getCellIndex(const std::string &name) {
  if (cellNames.size() > 0) {
    for (auto i = (int)cellNames.size() - 1; i >= 0; i--) {
      if (cellNames[i] == name) {
        return i;
      }
    }
  }
  return -1;
}

CellObject::CellObject(EvaValue value)
    : Object(ObjectType::CELL), value(value) {}

FunctionObject::FunctionObject(CodeObject *co)
    : Object(ObjectType::FUNCTION), co(co) {}

std::string evaValueToTypeString(const EvaValue &evaValue) {
  if (IS_NUMBER(evaValue)) {
    return "NUMBER";
  } else if (IS_BOOLEAN(evaValue)) {
    return "BOOLEAN";
  } else if (IS_STRING(evaValue)) {
    return "STRING";
  } else if (IS_CODE(evaValue)) {
    return "CODE";
  } else if (IS_NATIVE(evaValue)) {
    return "NATIVE";
  } else if (IS_FUNCTION(evaValue)) {
    return "FUNCTION";
  } else if (IS_CELL(evaValue)) {
    return "CELL";
  } else {
    DIE << "evaValueToTypeString: unknown type " << (int)evaValue.type;
  }
  return "";
}

std::string evaValueToConstantString(const EvaValue &evaValue) {
  std::stringstream ss;

  if (IS_NUMBER(evaValue)) {
    ss << AS_NUMBER(evaValue);
  } else if (IS_BOOLEAN(evaValue)) {
    ss << ((AS_BOOLEAN(evaValue) == true) ? "true" : "false");
  } else if (IS_STRING(evaValue)) {
    ss << AS_CPPSTRING(evaValue);
  } else if (IS_CODE(evaValue)) {
    auto code = AS_CODE(evaValue);
    ss << "code" << code << ": " << code->name << "/" << code->arity;
  } else if (IS_FUNCTION(evaValue)) {
    auto fn = AS_FUNCTION(evaValue);
    ss << fn->co->name << "/" << fn->co->arity;
  } else if (IS_NATIVE(evaValue)) {
    auto fn = AS_NATIVE(evaValue);
    ss << fn->name << "/" << fn->arity;
  } else if (IS_CELL(evaValue)) {
    auto cell = AS_CELL(evaValue);
    ss << "cell: " << evaValueToConstantString(cell->value);
  } else {
    DIE << "evaValueToConstantString: unknown type " << (int)evaValue.type;
  }
  return ss.str();
}

std::ostream &operator<<(std::ostream &os, const EvaValue &evaValue) {
  return os << "EvaValue (" << evaValueToTypeString(evaValue)
            << "): " << evaValueToConstantString(evaValue);
}
