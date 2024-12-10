#ifndef __EvaValue_h
#define __EvaValue_h

#include <cstdint>
#include <functional>
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
  Object(ObjectType type);
  ObjectType type;
};

using NativeFn = std::function<void()>;

struct NativeObject : public Object {
  NativeObject(NativeFn function, const std::string &name, size_t arity);

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
  StringObject(const std::string &str);
  std::string string;
};

struct LocalVar {
  std::string name;
  size_t scopeLevel;
};

struct CodeObject : public Object {
  CodeObject(const std::string &name, size_t arity);

  std::string name;

  size_t arity;

  std::vector<EvaValue> constants;

  std::vector<uint8_t> code;

  size_t scopeLevel = 0;

  std::vector<LocalVar> locals;

  std::vector<std::string> cellNames;

  size_t nonCellFnParams = 0;

  size_t freeCount = 0;

  void addLocal(const std::string &name);

  void addConst(const EvaValue &value);

  int getLocalIndex(const std::string &name);

  int getCellIndex(const std::string &name);
};

struct CellObject : public Object {
  CellObject(EvaValue value);
  EvaValue value;
};

struct FunctionObject : public Object {
  FunctionObject(CodeObject *co);
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

std::string evaValueToTypeString(const EvaValue &evaValue);

std::string evaValueToConstantString(const EvaValue &evaValue);

std::ostream &operator<<(std::ostream &os, const EvaValue &evaValue);

#endif // !__EvaValue_h
