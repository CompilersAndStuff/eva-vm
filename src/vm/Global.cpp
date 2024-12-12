#include "Global.h"
#include "../Logger.h"
#include "EvaValue.h"

void Global::define(const std::string &name) {
  auto index = getGlobalIndex(name);

  if (index != -1) {
    return;
  }

  globals.push_back({name, NUMBER(0)});
}

GlobalVar &Global::get(size_t index) { return globals[index]; }

void Global::set(size_t index, const EvaValue &value) {
  if (index >= globals.size()) {
    DIE << "Global " << index << " doesn't exist.";
  }
  globals[index].value = value;
}

void Global::addNativeFunction(const std::string &name,
                               std::function<void()> fn, size_t arity) {
  if (exists(name)) {
    return;
  }

  globals.push_back({name, ALLOC_NATIVE(fn, name, arity)});
}

void Global::addConst(const std::string &name, double value) {
  if (exists(name)) {
    return;
  }

  globals.push_back({name, NUMBER(value)});
}

int Global::getGlobalIndex(const std::string &name) {
  if (globals.size() > 0) {
    for (auto i = (int)globals.size() - 1; i >= 0; i--) {
      if (globals[i].name == name) {
        return i;
      }
    }
  }
  return -1;
}

bool Global::exists(const std::string &name) {
  return getGlobalIndex(name) != -1;
}
