cmake_minimum_required(VERSION 3.10)
project(EvaVm VERSION 0.1.0 LANGUAGES C CXX)

add_executable(EvaVm
    src/eva-vm.cpp
    src/Logger.cpp
    src/vm/EvaValue.cpp
    src/vm/EvaVm.cpp
    src/vm/Global.cpp
    src/disassembler/EvaDisassembler.cpp
    src/compiler/EvaCompiler.cpp
    src/compiler/Scope.cpp
    src/bytecode/OpCode.cpp
)
