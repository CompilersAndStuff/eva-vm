cmake_minimum_required(VERSION 3.10)
project(EvaVm VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    src/gc/EvaCollector.cpp
)

target_compile_features(EvaVm PUBLIC cxx_std_17)
target_compile_options(EvaVm PUBLIC -fsized-deallocation)
