#include "vm/EvaVm.h"
#include "Logger.h"
#include <iostream>

int main(int argc, char *argv[]) {
  EvaVm vm;

  auto program = R"(
    /*(def cons (h t) (lambda (sig1 _sig2) (if sig1 h t)))*/
    /*(def first (c) (c true false))*/
    /*(def rest (c) (c false true))*/
    /*(first (cons 3 (cons 2 1)))*/

    /*(def curry1 (f ca) (lambda (a) (f ca a)))*/
    /*(def sum (a b) (+ a b))*/
    /*(var add5 (curry1 sum 5))*/
    /*(add5 10)*/

    (def t (a q) 
    (begin 
      (lambda (b) 
       (lambda (c) (+ a (+ b c)))))
     )

    (((t 1 10) 2) 3)
  )";

  log(program);

  auto result = vm.exec(program);

  log(result);

  std::cout << "All done!\n";

  return 0;
}
