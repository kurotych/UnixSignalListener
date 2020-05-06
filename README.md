# C++ wrapper for processing UNIX signals. 
It is developed according to the common approach for multi-threaded programs. 

Build and Run example:
```
g++ example.cpp -o unix_example -pthread
./unix_example
pkill unix_example (from other terminal)
```