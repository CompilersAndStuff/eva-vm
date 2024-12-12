#ifndef __Scope_h
#define __Scope_h

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
  Scope(ScopeType type, std::shared_ptr<Scope> parent);

  void addLocal(const std::string &name);

  void pushBackIfNotPresent(std::vector<std::string> &v,
                            const std::string &name);

  void addCell(const std::string &name);

  void addFree(const std::string &name);

  void maybePromote(const std::string &name);

  void promote(const std::string &name, Scope *ownerScope);

  std::pair<Scope *, AllocType> resolve(const std::string &name,
                                        AllocType allocType);

  int getNameGetter(const std::string &name);

  int getNameSetter(const std::string &name);

  ScopeType type;

  std::shared_ptr<Scope> parent;

  std::map<std::string, AllocType> allocInfo;

  std::vector<std::string> free;

  std::vector<std::string> cells;
};

#endif // __Scope_h
