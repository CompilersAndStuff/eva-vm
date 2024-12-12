#ifndef __Scope_hpp
#define __Scope_hpp

#include "../Logger.hpp"
#include "../bytecode/OpCode.hpp"
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class ScopeType {
  GLOBAL,
  FUNCTION,
  BLOCK,
};

enum class AllocType {
  GLOBAL,
  LOCAL,
  CELL,
};

struct Scope {
  Scope(ScopeType type, std::shared_ptr<Scope> parent)
      : type(type), parent(parent) {}

  void addLocal(const std::string &name) {
    allocInfo[name] =
        type == ScopeType::GLOBAL ? AllocType::GLOBAL : AllocType::LOCAL;
  }

  void pushBackIfNotPresent(std::vector<std::string> &v,
                            const std::string &name) {
    auto maybeFound = std::find(v.begin(), v.end(), name);
    if (maybeFound == v.end()) {
      v.push_back(name);
    }
  }

  void addCell(const std::string &name) {
    pushBackIfNotPresent(cells, name);
    allocInfo[name] = AllocType::CELL;
  }

  void addFree(const std::string &name) {
    pushBackIfNotPresent(free, name);
    allocInfo[name] = AllocType::CELL;
  }

  void maybePromote(const std::string &name) {
    auto initAllocType =
        type == ScopeType::GLOBAL ? AllocType::GLOBAL : AllocType::LOCAL;

    if (allocInfo.count(name) != 0) {
      initAllocType = allocInfo[name];
    }

    if (initAllocType == AllocType::CELL) {
      return;
    }

    auto [ownerScope, allocType] = resolve(name, initAllocType);

    allocInfo[name] = allocType;

    if (allocType == AllocType::CELL) {
      promote(name, ownerScope);
    }
  }

  void promote(const std::string &name, Scope *ownerScope) {
    ownerScope->addCell(name);

    auto scope = this;
    while (scope != ownerScope) {
      scope->addFree(name);
      scope = scope->parent.get();
    }
  }

  std::pair<Scope *, AllocType> resolve(const std::string &name,
                                        AllocType allocType) {
    if (allocInfo.count(name) != 0) {
      return std::make_pair(this, allocType);
    }

    if (type == ScopeType::FUNCTION) {
      allocType = AllocType::CELL;
    }

    if (parent == nullptr) {
      DIE << "[Scope] Reference error: " << name << " is not defined.";
    }

    if (parent->type == ScopeType::GLOBAL) {
      allocType = AllocType::GLOBAL;
    }

    return parent->resolve(name, allocType);
  }

  int getNameGetter(const std::string &name) {
    switch (allocInfo[name]) {
    case AllocType::GLOBAL:
      return OP_GET_GLOBAL;
    case AllocType::LOCAL:
      return OP_GET_LOCAL;
    case AllocType::CELL:
      return OP_GET_CELL;
    }
  }

  int getNameSetter(const std::string &name) {
    switch (allocInfo[name]) {
    case AllocType::GLOBAL:
      return OP_SET_GLOBAL;
    case AllocType::LOCAL:
      return OP_SET_LOCAL;
    case AllocType::CELL:
      return OP_SET_CELL;
    }
  }

  ScopeType type;

  std::shared_ptr<Scope> parent;

  std::map<std::string, AllocType> allocInfo;

  std::vector<std::string> free;

  std::vector<std::string> cells;
};

#endif // __Scope_hpp
