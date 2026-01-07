build main.cpp
```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
cmake --build build -j
```

execute command
```bash
./build/ch1
```