# Exploring Multiple Processes and IPC

## Language: C++

## Files:

### main.cpp
This is the main program. Since I wrote it in C++, I was using Forks and Pipes, so I only have 1 source file.

### sample5.txt
This is the file I created. It will create 3 random integers from 1-100. The 2nd will be subtracted from the 1st one and the 3rd one will add to the result.
It will print out the entire formula nicely with correct mathematical formatting.
On the next line, it will print out the course number.
Example:
```
51 - 99 + 62 = 14
CS4348
```

## How to compile and run the project
This has to be run in a UNIX/Linux environment.

```bash
g++ -std=c++11 main.cpp
./a.out <input file> <timer>
```
