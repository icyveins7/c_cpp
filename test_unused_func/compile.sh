g++ test_unused_func.cpp -o plain
g++ test_unused_func.cpp -O1 -o plain_O1
g++ test_unused_func.cpp -O2 -o plain_O2
g++ test_unused_func.cpp -O3 -o plain_O3
g++ test_unused_func.cpp -ffunction-sections -Wl,-dead_strip -o stripped
g++ test_unused_func.cpp -ffunction-sections -Wl,-dead_strip -O1 -o stripped_O1
g++ test_unused_func.cpp -ffunction-sections -Wl,-dead_strip -O2 -o stripped_O2
g++ test_unused_func.cpp -ffunction-sections -Wl,-dead_strip -O3 -o stripped_O3
