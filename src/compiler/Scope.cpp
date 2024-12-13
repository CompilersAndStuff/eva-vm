#include "Scope.h"
#include "../Logger.h"
#include "../bytecode/OpCode.h"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

Scope::Scope(ScopeType type, std::shared_ptr<Scope> parent)
    : type(type), parent(parent) {}

void Scope::addLocal(const std::string &name) {
  allocInfo[name] =
      type == ScopeType::GLOBAL ? AllocType::GLOBAL : AllocType::LOCAL;
}

void Scope::pushBackIfNotPresent(std::vector<std::string> &v,
                                 const std::string &name) {
  auto maybeFound = std::find(v.begin(), v.end(), name);
  if (maybeFound == v.end()) {
    v.push_back(name);
  }
}

void Scope::addCell(const std::string &name) {
  pushBackIfNotPresent(cells, name);
  allocInfo[name] = AllocType::CELL;
}

void Scope::addFree(const std::string &name) {
  pushBackIfNotPresent(free, name);
  allocInfo[name] = AllocType::CELL;
}

void Scope::maybePromote(const std::string &name) {
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

void Scope::promote(const std::string &name, Scope *ownerScope) {
  ownerScope->addCell(name);

  auto scope = this;
  while (scope != ownerScope) {
    scope->addFree(name);
    scope = scope->parent.get();
  }
}

std::pair<Scope *, AllocType> Scope::resolve(const std::string &name,
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

int Scope::getNameGetter(const std::string &name) {
  switch (allocInfo[name]) {
  case AllocType::GLOBAL:
    return static_cast<int>(OpCode::GET_GLOBAL);
  case AllocType::LOCAL:
    return static_cast<int>(OpCode::GET_LOCAL);
  case AllocType::CELL:
    return static_cast<int>(OpCode::GET_CELL);
  }
}

int Scope::getNameSetter(const std::string &name) {
  switch (allocInfo[name]) {
  case AllocType::GLOBAL:
    return static_cast<int>(OpCode::SET_GLOBAL);
  case AllocType::LOCAL:
    return static_cast<int>(OpCode::SET_LOCAL);
  case AllocType::CELL:
    return static_cast<int>(OpCode::SET_CELL);
  }
}
