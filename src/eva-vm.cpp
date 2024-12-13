#include "Logger.h"
#include "vm/EvaVm.h"
#include <iostream>

int main(int argc, char *argv[]) {
  {
    EvaVm vm;

    auto program = R"(
      (def t (a)
        (lambda (b)
          (begin 
            (+ a b)
            (lambda (c) (+ a (+ b c) ) ) ) ) )

      (((t 1) 2) 3)


    )";

    log(program);

    auto result = vm.exec(program);

    log(result);
  }
  std::cout << "All done!\n";

  return 0;
}
