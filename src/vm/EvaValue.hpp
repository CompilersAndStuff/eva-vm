#ifndef __EvaValue_hpp
#define __EvaValue_hpp

#include "../Logger.hpp"
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

enum class EvaValueType {
  NUMBER,
  BOOLEAN,
  OBJECT,
};

enum class ObjectType {
  STRING,
  CODE,
  NATIVE,
  FUNCTION,
  CELL,
};

struct Object {
  Object(ObjectType type) : type(type) {}
  ObjectType type;
};

using NativeFn = std::function<void()>;

struct NativeObject : public Object {
  NativeObject(NativeFn function, const std::string &name, size_t arity)
      : Object(ObjectType::NATIVE), function(function), name(name),
        arity(arity) {}

  NativeFn function;

  std::string name;

  size_t arity;
};

struct EvaValue {
  EvaValueType type;
  union {
    double number;
    bool boolean;
    Object *object;
  };
};

struct StringObject : public Object {
  StringObject(const std::string &str)
      : Object(ObjectType::STRING), string(str) {}
  std::string string;
};

struct LocalVar {
  std::string name;
  size_t scopeLevel;
};

struct CodeObject : public Object {
  CodeObject(const std::string &name, size_t arity)
      : Object(ObjectType::CODE), name(name), arity(arity) {}

  std::string name;

  size_t arity;

  std::vector<EvaValue> constants;

  std::vector<uint8_t> code;

  size_t scopeLevel = 0;

  std::vector<LocalVar> locals;

  std::vector<std::string> cellNames;

  size_t nonCellFnParams = 0;

  size_t freeCount = 0;

  void addLocal(const std::string &name) {
    locals.push_back({name, scopeLevel});
  }

  void addConst(const EvaValue &value) { constants.push_back(value); }

  int getLocalIndex(const std::string &name) {
    if (locals.size() > 0) {
      for (auto i = (int)locals.size() - 1; i >= 0; i--) {
        if (locals[i].name == name) {
          return i;
        }
      }
    }
    return -1;
  }

  int getCellIndex(const std::string &name) {
    if (cellNames.size() > 0) {
      for (auto i = (int)cellNames.size() - 1; i >= 0; i--) {
        if (cellNames[i] == name) {
          return i;
        }
      }
    }
    return -1;
  }
};

struct CellObject : public Object {
  CellObject(EvaValue value) : Object(ObjectType::CELL), value(value) {}
  EvaValue value;
};

struct FunctionObject : public Object {
  FunctionObject(CodeObject *co) : Object(ObjectType::FUNCTION), co(co) {}
  CodeObject *co;

  std::vector<CellObject *> cells;
};

#define NUMBER(value) ((EvaValue){EvaValueType::NUMBER, .number = value})
#define BOOLEAN(value) ((EvaValue){EvaValueType::BOOLEAN, .boolean = value})
#define OBJECT(value) ((EvaValue){EvaValueType::OBJECT, .object = value})

#define ALLOC_STRING(value)                                                    \
  ((EvaValue){EvaValueType::OBJECT,                                            \
              .object = (Object *)new StringObject(value)})
#define ALLOC_CODE(name, arity)                                                \
  ((EvaValue){EvaValueType::OBJECT,                                            \
              .object = (Object *)new CodeObject(name, arity)})
#define ALLOC_NATIVE(fn, name, arity)                                          \
  ((EvaValue){EvaValueType::OBJECT,                                            \
              .object = (Object *)new NativeObject(fn, name, arity)})
#define ALLOC_FUNCTION(co)                                                     \
  ((EvaValue){EvaValueType::OBJECT, .object = (Object *)new FunctionObject(co)})

#define ALLOC_CELL(co)                                                         \
  ((EvaValue){EvaValueType::OBJECT, .object = (Object *)new CellObject(co)})

#define CELL(cellObject) OBJECT((Object *)cellObject)

#define AS_NUMBER(evaValue) ((double)(evaValue).number)
#define AS_BOOLEAN(evaValue) ((bool)(evaValue).boolean)
#define AS_STRING(evaValue) ((StringObject *)(evaValue).object)
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue)->string)
#define AS_OBJECT(evaValue) ((Object *)(evaValue).object)
#define AS_CODE(evaValue) ((CodeObject *)(evaValue).object)
#define AS_NATIVE(evaValue) ((NativeObject *)(evaValue).object)
#define AS_FUNCTION(evaValue) ((FunctionObject *)(evaValue).object)
#define AS_CELL(evaValue) ((CellObject *)(evaValue).object)

#define IS_NUMBER(evaValue) ((evaValue).type == EvaValueType::NUMBER)
#define IS_BOOLEAN(evaValue) ((evaValue).type == EvaValueType::BOOLEAN)
#define IS_OBJECT(evaValue) ((evaValue).type == EvaValueType::OBJECT)
#define IS_OBJECT_TYPE(evaValue, objectType)                                   \
  (IS_OBJECT(evaValue) && AS_OBJECT(evaValue)->type == objectType)
#define IS_STRING(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::STRING)
#define IS_CODE(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::CODE)
#define IS_NATIVE(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::NATIVE)
#define IS_FUNCTION(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::FUNCTION)
#define IS_CELL(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::CELL)

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

#endif // !__EvaValue_hpp
