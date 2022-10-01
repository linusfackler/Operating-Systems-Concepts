#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

int pipeMemory[2], pipeCPU[2];
int const READ      = 0;
int const WRITE     = 1;
int const PIPE_ERROR    = -1;
int const CHILD_PID     =  0;

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cout << "I need 2 arguments to work. Try again." << endl;
        cout << "Input file & timer value" << endl;
        exit(EXIT_FAILURE);
    }

    if (pipe(pipeMemory) == PIPE_ERROR || pipe(pipeCPU) == PIPE_ERROR)
    {
        cout << "Pipe failed. Try again." << endl;
        exit(EXIT_FAILURE);
    }

    int timer = (int)argv[2];
    FILE *input = fopen(argv[1], "r");
    if (!input)
    {
        cout << "File does not exist. Try again." << endl;
        exit(EXIT_FAILURE);
    }

    int pid = fork();

    if (pid < 0>)
    {
        cout << "Invalid process. Try again." << endl;
        exit(EXIT_FAILURE);
    }

    else if (pid == 0)
    {
        int memory[2000];
        int user_program = 0;
        int system_program = 1000;

        char *line;
    }
    
}