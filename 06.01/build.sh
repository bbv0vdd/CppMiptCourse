#!/bin/bash

# 1. Компиляция rational.cpp 
time g++ -std=c++23 -Wall -Wextra -c rational.cpp -o rational.o

# 2. Компиляция 06.01.cpp 
time g++ -std=c++23 -Wall -Wextra -c 06.01.cpp -o 06.01.o

# 3. Компоновка
time g++ rational.o 06.01.o -o 06.01.out

# 4. Проверка размеров файлов
ls -lh rational.o 06.01.o 06.01.out
