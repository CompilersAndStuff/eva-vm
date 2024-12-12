#ifndef __EvaCollector_h
#define __EvaCollector_h

#include "../vm/EvaValue.h"

#include <set>
struct EvaCollector {
  void gc(const std::set<Traceable *> &roots);

  void mark(const std::set<Traceable *> &roots);

  std::set<Traceable *> getPointers(const Traceable *object);

  void sweep();
};

#endif // !__EvaCollector_h
