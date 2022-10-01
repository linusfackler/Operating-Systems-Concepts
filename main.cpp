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
int const PIPE_ERROR        = -1;
int const CHILD_PID         =  0;
int const user_program      = 0;
int const system_program    = 1000;
int const max_line_size     = 1000;
int memory[2000];

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
    FILE *file = fopen(argv[1], "r");
    if (!file)
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
        int current_program = user_program;
        char input = [max_line_size];
        int size = sizeof(input);

        bool change_address = false;

        while(fgets(input, max_line_size, file) != NULL)
        {
            char *temp;
            int i = 0;

            if (input[0] == '.')
            {
                change_address = true;
                i++;
            }
            
            while (isdigit(input[i]))
            {
                temp[i] = input[i];
                i++;
            }

            if (change_address)
            {
                for (int j = 0; j < sizeof(temp) - 1; j++)
                    temp[j] = temp[j + 1];
                current_program = atoi(temp);
            }

            else if (isdigit(input[0]))
            {
                memory[current_program] = atoi(temp);
                current_program++;
            }
        }
        // finished reading file

        char instruction;
        while (true)
        {
            read(pipeMemory[0], )
        }

        
    }
    
}