#ifndef __EvaCollector_hpp
#define __EvaCollector_hpp

#include "../vm/EvaValue.hpp"

#include <set>
struct EvaCollector {
  void gc(const std::set<Traceable *> &roots) {
    mark(roots);
    sweep();
  }

  void mark(const std::set<Traceable *> &roots) {
    std::vector<Traceable *> worklist(roots.begin(), roots.end());

    while (!worklist.empty()) {
      auto object = worklist.back();
      worklist.pop_back();

      if (!object->marked) {
        object->marked = true;
        for (auto &p : getPointers(object)) {
          worklist.push_back(p);
        }
      }
    }
  }

  std::set<Traceable *> getPointers(const Traceable *object) {
    std::set<Traceable *> pointers;

    auto evaValue = OBJECT((Object *)object);

    if (IS_FUNCTION(evaValue)) {
      auto fn = AS_FUNCTION(evaValue);
      for (auto &cell : fn->cells) {
        pointers.insert((Traceable *)cell);
      }
    }

    return pointers;
  }

  void sweep() {
    auto it = Traceable::objects.begin();
    while (it != Traceable::objects.end()) {
      auto object = (Traceable *)*it;
      if (object->marked) {
        object->marked = false;
        ++it;
      } else {
        it = Traceable::objects.erase(it);
        delete object;
      }
    }
  }
};

#endif // !__EvaCollector_hpp
