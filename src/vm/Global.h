#ifndef __Global_h
#define __Global_h

#include "EvaValue.h"

struct GlobalVar {
  std::string name;
  EvaValue value;
};

struct Global {
  void define(const std::string &name);

  GlobalVar &get(size_t index);

  void set(size_t index, const EvaValue &value);

  void addNativeFunction(const std::string &name, std::function<void()> fn,
                         size_t arity);

  void addConst(const std::string &name, double value);

  int getGlobalIndex(const std::string &name);

  bool exists(const std::string &name);

  std::vector<GlobalVar> globals;
};

#endif // !__Global_h
