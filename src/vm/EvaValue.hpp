#ifndef __EvaValue_hpp
#define __EvaValue_hpp

#include "../Logger.hpp"
#include <sstream>
#include <string>
#include <vector>

enum class EvaValueType {
  NUMBER,
  BOOLEAN,
  OBJECT,
};

enum class ObjectType { STRING, CODE };

struct Object {
  Object(ObjectType type) : type(type) {}
  ObjectType type;
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
  CodeObject(const std::string &name) : Object(ObjectType::CODE), name(name) {}

  std::string name;

  std::vector<EvaValue> constants;

  std::vector<uint8_t> code;

  size_t scopeLevel = 0;

  std::vector<LocalVar> locals;

  void addLocal(const std::string &name) {
    locals.push_back({name, scopeLevel});
  }

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
};

#define NUMBER(value) ((EvaValue){EvaValueType::NUMBER, .number = value})
#define BOOLEAN(value) ((EvaValue){EvaValueType::BOOLEAN, .boolean = value})
#define ALLOC_STRING(value)                                                    \
  ((EvaValue){EvaValueType::OBJECT,                                            \
              .object = (Object *)new StringObject(value)})
#define ALLOC_CODE(name)                                                       \
  ((EvaValue){EvaValueType::OBJECT, .object = (Object *)new CodeObject(name)})

#define AS_NUMBER(evaValue) ((double)(evaValue).number)
#define AS_BOOLEAN(evaValue) ((bool)(evaValue).boolean)
#define AS_STRING(evaValue) ((StringObject *)(evaValue).object)
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue)->string)
#define AS_OBJECT(evaValue) ((Object *)(evaValue).object)
#define AS_CODE(evaValue) ((CodeObject *)(evaValue).object)

#define IS_NUMBER(evaValue) ((evaValue).type == EvaValueType::NUMBER)
#define IS_BOOLEAN(evaValue) ((evaValue).type == EvaValueType::BOOLEAN)
#define IS_OBJECT(evaValue) ((evaValue).type == EvaValueType::OBJECT)
#define IS_OBJECT_TYPE(evaValue, objectType)                                   \
  (IS_OBJECT(evaValue) && AS_OBJECT(evaValue)->type == objectType)
#define IS_STRING(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::STRING)
#define IS_CODE(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::CODE)

std::string evaValueToTypeString(const EvaValue &evaValue) {
  if (IS_NUMBER(evaValue)) {
    return "NUMBER";
  } else if (IS_BOOLEAN(evaValue)) {
    return "BOOLEAN";
  } else if (IS_STRING(evaValue)) {
    return "STRING";
  } else if (IS_CODE(evaValue)) {
    return "CODE";
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
    ss << "code" << code << ": " << code->name;
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
