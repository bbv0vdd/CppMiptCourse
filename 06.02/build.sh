#!/bin/bash

rm -rf build
mkdir build
cd build

echo "=== Измерение времени сборки модуля ==="

# 1. Прекомпиляция модуля (.pcm файл)
time clang++-18 -std=c++20 -stdlib=libc++ -fmodules --precompile ../rational.cppm -o rational.pcm

# 2. Компиляция интерфейса модуля
time clang++-18 -std=c++20 -stdlib=libc++ -fmodules -fprebuilt-module-path=. -c ../rational.cppm -o rational_interface.o

# 3. Компиляция реализации модуля
time clang++-18 -std=c++20 -stdlib=libc++ -fmodules -fprebuilt-module-path=. -c ../rational.cpp -o rational_impl.o

# 4. Компиляция основной программы
time clang++-18 -std=c++20 -stdlib=libc++ -fmodules -fprebuilt-module-path=. -c ../06.02.cpp -o 06.02.o

# 5. Компоновка
time clang++-18 -std=c++20 -stdlib=libc++ rational_interface.o rational_impl.o 06.02.o -o 06.02.out

ls -lh rational.pcm rational_interface.o rational_impl.o 06.02.o 06.02.out