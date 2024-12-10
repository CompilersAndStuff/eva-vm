#include "EvaValue.h"
#include "../Logger.h"
#include <functional>
#include <sstream>
#include <string>
#include <vector>

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
  if (isNumber(evaValue)) {
    return "NUMBER";
  } else if (isBoolean(evaValue)) {
    return "BOOLEAN";
  } else if (isString(evaValue)) {
    return "STRING";
  } else if (isCode(evaValue)) {
    return "CODE";
  } else if (isNative(evaValue)) {
    return "NATIVE";
  } else if (isFuntion(evaValue)) {
    return "FUNCTION";
  } else if (isCell(evaValue)) {
    return "CELL";
  } else {
    DIE << "evaValueToTypeString: unknown type " << (int)evaValue.type;
  }
  return "";
}

std::string evaValueToConstantString(const EvaValue &evaValue) {
  std::stringstream ss;

  if (isNumber(evaValue)) {
    ss << asNumber(evaValue);
  } else if (isBoolean(evaValue)) {
    ss << (asBoolean(evaValue) ? "true" : "false");
  } else if (isString(evaValue)) {
    ss << asCppString(evaValue);
  } else if (isCode(evaValue)) {
    auto code = asCode(evaValue);
    ss << "code" << code << ": " << code->name << "/" << code->arity;
  } else if (isFuntion(evaValue)) {
    auto fn = asFunction(evaValue);
    ss << fn->co->name << "/" << fn->co->arity;
  } else if (isNative(evaValue)) {
    auto fn = asNative(evaValue);
    ss << fn->name << "/" << fn->arity;
  } else if (isCell(evaValue)) {
    auto cell = asCell(evaValue);
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

EvaValue makeNumber(double value) {
  return {.type = EvaValueType::NUMBER, .number = value};
}

EvaValue makeBoolean(bool value) {
  return {.type = EvaValueType::BOOLEAN, .boolean = value};
}

EvaValue makeObject(Object *value) {
  return {.type = EvaValueType::OBJECT, .object = value};
}

EvaValue allocString(std::string value) {
  return {
    .type = EvaValueType::OBJECT,
    .object = (Object *)new StringObject(value)
  };
}

EvaValue allocCode(const std::string &name, size_t arity) {
  return {
    .type = EvaValueType::OBJECT,
    .object = (Object *)new CodeObject(name, arity)
  };
}

EvaValue allocNative(NativeFn fn, const std::string &name, size_t arity) {
  return {
    .type = EvaValueType::OBJECT,
    .object = (Object *)new NativeObject(fn, name, arity)
  };
}

EvaValue allocFunction(CodeObject *co) {
  return {.type = EvaValueType::OBJECT, .object = (Object *)new FunctionObject(co)};
}

EvaValue allocCell(EvaValue co) {
  return {.type = EvaValueType::OBJECT, .object = (Object *)new CellObject(co)};
}

EvaValue cell(CellObject *cellObject) {
  return makeObject((Object *)cellObject);
}

double asNumber(const EvaValue &evaValue) {
  return evaValue.number;
}

bool asBoolean(const EvaValue &evaValue) {
  return evaValue.boolean;
}

StringObject *asString(const EvaValue &evaValue) {
  return (StringObject *)evaValue.object;
}

std::string asCppString(const EvaValue &evaValue) {
  return asString(evaValue)->string;
}

Object *asObject(const EvaValue &evaValue) {
  return evaValue.object;
}

CodeObject *asCode(const EvaValue &evaValue) {
  return (CodeObject*)evaValue.object;
}

NativeObject *asNative(const EvaValue &evaValue) {
  return (NativeObject*)evaValue.object;
}

FunctionObject *asFunction(const EvaValue &evaValue) {
  return (FunctionObject*)evaValue.object;
}
CellObject *asCell(const EvaValue &evaValue) {
  return (CellObject*)evaValue.object;
}

bool isNumber(const EvaValue &evaValue) {
  return evaValue.type == EvaValueType::NUMBER;
}
bool isBoolean(const EvaValue &evaValue) {
  return evaValue.type == EvaValueType::BOOLEAN;
}
bool isObject(const EvaValue &evaValue) {
  return evaValue.type == EvaValueType::OBJECT;
}
bool isObjectType(const EvaValue &evaValue, ObjectType objectType) {
  return isObject(evaValue) && asObject(evaValue)->type == objectType;
}

bool isString(const EvaValue &evaValue) {
  return isObjectType(evaValue, ObjectType::STRING);
}
bool isCode(const EvaValue &evaValue) {
  return isObjectType(evaValue, ObjectType::CODE);
}
bool isNative(const EvaValue &evaValue) {
  return isObjectType(evaValue, ObjectType::NATIVE);
}
bool isFuntion(const EvaValue &evaValue) {
  return isObjectType(evaValue, ObjectType::FUNCTION);
}
bool isCell(const EvaValue &evaValue) {
  return isObjectType(evaValue, ObjectType::CELL);
}
