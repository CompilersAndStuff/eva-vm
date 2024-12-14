#include "Logger.h"
#include "vm/EvaVm.h"
#include <iostream>

int main(int argc, char *argv[]) {
  {
    EvaVm vm;

    auto program = R"(
      (def t (a q)
        (begin 
          (print (+ a q))
          (lambda (b)
            (lambda (c) (+ a (+ b c) ) ) )) )

      (((t 1 10) 2) 3)

      /*(print (+ 1 1))*/

    )";

    log(program);

    auto result = vm.exec(program);

    log(result);
  }
  std::cout << "All done!\n";

  return 0;
}
