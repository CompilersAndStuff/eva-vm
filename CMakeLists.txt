cmake_minimum_required(VERSION 3.10)
project(EvaVm VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/eva-vm.cpp
    src/Logger.cpp
    src/parser/Expression.cpp
    src/vm/EvaValue.cpp
    src/vm/EvaVm.cpp
    src/vm/Global.cpp
    src/disassembler/EvaDisassembler.cpp
    src/compiler/EvaCompiler.cpp
    src/compiler/Scope.cpp
    src/bytecode/OpCode.cpp
    src/gc/EvaCollector.cpp
)

set(SANITIZERS
	-fsanitize=address
	-fsanitize=undefined
)

add_compile_options(-fsized-deallocation)

add_executable(EvaVm ${SOURCES})
target_compile_features(EvaVm PUBLIC cxx_std_17)

add_executable(EvaVmSanitizers ${SOURCES})
target_compile_features(EvaVmSanitizers PUBLIC cxx_std_17)
target_compile_options(EvaVmSanitizers PRIVATE ${SANITIZERS})
target_link_options(EvaVmSanitizers PRIVATE ${SANITIZERS})
