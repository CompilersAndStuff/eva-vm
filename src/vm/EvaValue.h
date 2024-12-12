#ifndef __EvaValue_h
#define __EvaValue_h

#include <iostream>
#include <list>
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

struct Traceable {
  bool marked;

  size_t size;

  static void *operator new(size_t size);

  static void operator delete(void *object);

  static void cleanup();

  static void printStats();

  static size_t bytesAllocated;

  static std::list<Traceable *> objects;
};
struct Object : public Traceable {
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

EvaValue makeNumber(double value);

EvaValue makeBoolean(bool value);

EvaValue makeObject(Object *value);

EvaValue allocString(std::string s);

EvaValue allocCode(const std::string &name, size_t arity);

EvaValue allocNative(NativeFn fn, const std::string &name, size_t arity);

EvaValue allocFunction(CodeObject *co);

EvaValue allocCell(EvaValue co);

EvaValue cell(CellObject *cellObject);

std::string evaValueToTypeString(const EvaValue &evaValue);

std::string evaValueToConstantString(const EvaValue &evaValue);

std::ostream &operator<<(std::ostream &os, const EvaValue &evaValue);

double asNumber(const EvaValue &evaValue);
bool asBoolean(const EvaValue &evaValue);
StringObject *asString(const EvaValue &evaValue);
std::string asCppString(const EvaValue &evaValue);
Object *asObject(const EvaValue &evaValue);
CodeObject *asCode(const EvaValue &evaValue);
NativeObject *asNative(const EvaValue &evaValue);
FunctionObject *asFunction(const EvaValue &evaValue);
CellObject *asCell(const EvaValue &evaValue);

bool isNumber(const EvaValue &evaValue);
bool isBoolean(const EvaValue &evaValue);
bool isObject(const EvaValue &evaValue);
bool isObjectType(const EvaValue &evaValue, ObjectType objectType);
bool isString(const EvaValue &evaValue);
bool isCode(const EvaValue &evaValue);
bool isNative(const EvaValue &evaValue);
bool isFunction(const EvaValue &evaValue);
bool isCell(const EvaValue &evaValue);

#endif // !__EvaValue_h
