//
// Project:     Exploring Multiple Processes and IPC
// Name:        Linus Fackler
// Class:       CS4348.005 Operating Systems Concepts
// Instructor:  Greg Ozbirn
//
// This project will simulate a simple computer system
// consisting of a CPU and Memory.
// The CPU and Memory will be simluated by separate processes
// that communicate.
//

#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>

int pipeMemory[2], pipeCPU[2];
int const READ                  = 0;
int const WRITE                 = 1;
int const PIPE_ERROR            = -1;
int const MEMORY_PID            = 0;
int const user_program          = 0;
int const system_program        = 1000;
int const max_line_size         = 1000;
int const USER_MODE             = 0;
int const KERNEL_MODE           = 1;
char const READ_INSTRUCTION     = 'r';
char const WRITE_INSTRUCTION    = 'w';
char const EXIT_INSTRUCTION     = 'e';

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("I need 2 arguments to work. Try again.\n");
        printf("Input file & timer value\n");
        exit(EXIT_FAILURE);
        // program needs 3 arguments. execution file, input file, timer value
    }

    if (pipe(pipeMemory) == PIPE_ERROR || pipe(pipeCPU) == PIPE_ERROR)
    {
        printf("Pipe failed. Try again.\n");
        exit(EXIT_FAILURE);
        // Piping failed
    }

    FILE *file = fopen(argv[1], "r");   // opening file
    if (!file)
    {
        printf("File does not exist. Try again.\n");
        exit(EXIT_FAILURE);
        // input file doesn't exist
    }


    int pid = fork();       // creating fork

    if (pid < 0)                        
    {
        printf("Invalid process. Try again.");
        exit(EXIT_FAILURE);
        // fork failed
    }

    // ---------------------------------------------------------------
    // ------------------------- M E M O R Y -------------------------
    // ---------------------------------------------------------------
    else if (pid == MEMORY_PID)
    {
        int memory[2000];
        close(pipeMemory[WRITE]);       // memory only reads
        close(pipeCPU[READ]);           // CPU only writes
        int current_program = user_program;
        char input [max_line_size];

        bool change_address;
        // true when . in input file -> address changes

        // reads until file reaches end
        while(fgets(input, max_line_size, file) != NULL)
        {
            char temp[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
            // temporary array of characters to store address in
            int i = 0;      // character counter
            change_address = false;

            if (input[0] == '.')
            {
                change_address = true;
                i++;
                // attempting to change address
            }
            
            while (isdigit(input[i]))
            {
                temp[i] = input[i];
                i++;
                // reads until comments start
            }

            if (change_address)
            {
                for (int j = 0; j < 5; j++)
                    temp[j] = temp[j + 1];
                current_program = atoi(temp);
                // changes address
            }

            else if (isdigit(input[0]))
            {
                memory[current_program] = atoi(temp);
                current_program++;
                // if no address change
            }
        }
        // finished reading file

        char instruction;
        int address;
        int value;

        // loops until it hits break
        // Memory listens for CPU's r/w requests
        while (true)
        {
            read(pipeMemory[READ], &instruction, 1);
            // reads instruction

            // read operation
            if (instruction == READ_INSTRUCTION)
            {
                read(pipeMemory[READ], &address, 4);
                value = memory[address];
                write(pipeCPU[WRITE], &value, 4);
            }

            // write operation
            else if (instruction == WRITE_INSTRUCTION)
            {
                read(pipeMemory[READ], &address, 4);
                read(pipeMemory[READ], &value, 4);
                memory[address] = value;
            }

            // exit operation
            else if (instruction == EXIT_INSTRUCTION)
            {
                close(pipeMemory[READ]);
                close(pipeMemory[WRITE]);
                close(pipeCPU[READ]);
                close(pipeCPU[WRITE]);
                return 0;
            }
        }
    }
    
    // ---------------------------------------------------------------
    // ---------------------------- C P U ----------------------------
    // ---------------------------------------------------------------
    else
    {
        int timer = atoi(argv[2]);
        srand(time(0));         // set timer seed to time(0)
                                // (num of seconds since Jan 1, 1970)

        int user_stack   = 999;     // end of user memory (0 - 999)
        int system_stack = 1999;    // end of system memory (1000 - 1999)
        int mode = USER_MODE;       // 0 -> user mode | 1 -> kernel mode

        int PC = 0, SP = user_stack, IR = 0, AC = 0, X, Y;
        int tempPC, tempSP;
        char instruction;

        int value, address;
        bool ex = false;    // writes exits when true

        close(pipeMemory[READ]);
        close(pipeCPU[WRITE]);

        while(true)
        {
            //timer interrupt
            if ((mode == USER_MODE) && (PC == timer))
            {
                mode = KERNEL_MODE;           // switches to kernel mode
                timer += timer;
                tempPC = PC;
                tempSP = SP;
                SP = system_stack;
                PC = 1000;
                
                // push user sp into system stack
                instruction = WRITE_INSTRUCTION;
                write(pipeMemory[WRITE], &instruction, 1);
                write(pipeMemory[WRITE], &SP, 4);
                write(pipeMemory[WRITE], &tempSP, 4);
                SP--;

                // push user pc into system stack
                write(pipeMemory[WRITE], &instruction, 1);
                write(pipeMemory[WRITE], &SP, 4);
                write(pipeMemory[WRITE], &tempPC, 4);
            }

            instruction = READ_INSTRUCTION;
            write(pipeMemory[WRITE], &instruction, 1);
            write(pipeMemory[WRITE], &PC, 4);
            PC++;
            read(pipeCPU[READ], &IR, 4);
            // reading instruction from memory into Instruction Register

            switch (IR)     // Instruction set
            {
                case 1:     // Load the value into the AC
                {
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;
                    read(pipeCPU[READ], &value, 4);
                    AC = value;
                    break;
                }

                case 2:     // Load the value at the address into the AC
                {
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;
                    // read address
                    read(pipeCPU[READ], &address, 4);
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &address, 4);
                    read(pipeCPU[READ], &value, 4);
                    AC = value;         // load address into AC
                    break;
                }

                case 3:     // Load the value from the address found
                {           // in the given address into the AC
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;
                    // read address
                    read(pipeCPU[READ], &address, 4);
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &address, 4);

                    // read address in address
                    read(pipeCPU[READ], &address, 4);
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &address, 4);
                    read(pipeCPU[READ], &value, 4);
                    AC = value;         // load address into AC
                    break;
                }
                
                case 4:     // Load the value at (address + X) into the AC
                {
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;

                    // read address
                    read(pipeCPU[READ], &address, 4);
                    address += X;       // add X to address    
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &address, 4);
                    read(pipeCPU[READ], &value, 4);
                    AC = value;         // load address into AC
                    break;
                }  

                case 5:     // Load the value at (address + Y) into the AC
                {
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;

                    // read address
                    read(pipeCPU[READ], &address, 4);
                    address += Y;       // add Y to address
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &address, 4);
                    read(pipeCPU[READ], &value, 4);
                    AC = value;         // load address into AC
                    break;
                }

                case 6:     // Load from (SP + X) into the AC
                {
                    address = SP + X;
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &address, 4);
                    read(pipeCPU[READ], &value, 4);
                    AC = value;
                    break;
                }

                case 7:     // Store the value in the AC into the address
                {
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;

                    // read address
                    read(pipeCPU[READ], &address, 4);
                    instruction = WRITE_INSTRUCTION;
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &address, 4);
                    write(pipeMemory[WRITE], &AC, 4);
                    // write value from AC to address
                    break;
                }

                case 8:     // Gets a random int from 1 to 100 into the AC
                {
                    AC = rand() % 100 + 1;
                    break;
                }

                case 9:     // If port = 1, writes AC as an int to the screen
                {           // If port = 2, write AC as a char to the screen
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;
                    read(pipeCPU[READ], &value, 4);
                    // reads port

                    if (value == 1)
                        printf("%d",AC);    // write AC as int
                        
                    else if (value == 2)
                        printf("%c",AC);    // write AC as char
                    break;
                }

                case 10:    // Add the value in X to the AC
                {
                    AC += X;
                    break;
                }
                
                case 11:    // Add the value in Y to the AC
                {
                    AC += Y;
                    break;
                }

                case 12:    // Subtract the value in X from the AC
                {
                    AC -= X;
                    break;
                }

                case 13:    // Subtract the value in Y from the AC
                {
                    AC -= Y;
                    break;
                }

                case 14:    // Copy the value in the AC to X
                {
                    X = AC;
                    break;
                }

                case 15:    // Copy the value in X to the AC
                {
                    AC = X;
                    break;
                }

                case 16:    // Copy the value in the AC to Y
                {
                    Y = AC;
                    break;
                }

                case 17:    // Copy the value in Y to the AC
                {
                    AC = Y;
                    break;
                }

                case 18:    // Copy the value in AC to the SP
                {
                    SP = AC;
                    break;
                }

                case 19:    // Copy the value in SP to the AC
                {
                    AC = SP;
                    break;
                }

                case 20:    // Jump to the address
                {
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;

                    // read address
                    read(pipeCPU[READ], &address, 4);
                    PC = address;
                    break;
                }

                case 21:    // Jump to the address only if the
                {           // value in the AC is zero
                    if (AC == 0)
                    {
                        write(pipeMemory[WRITE], &instruction, 1);
                        write(pipeMemory[WRITE], &PC, 4);
                    }
                    PC++;   // if AC != 0, PC still gets incremented

                    if (AC == 0)
                    {
                        // read address
                        read(pipeCPU[READ], &address, 4);
                        PC = address;
                    }
                    break;
                }

                case 22:    // Jump to the address only if the
                {           // value in the AC is not zero
                    if (AC != 0)
                    {
                        write(pipeMemory[WRITE], &instruction, 1);
                        write(pipeMemory[WRITE], &PC, 4);
                    }
                    PC++;   // if AC == 0, PC still gets incremented
                    if (AC != 0)
                    {
                        // read address
                        read(pipeCPU[READ], &address, 4);
                        PC = address;
                    }
                    break;
                }

                case 23:    // Push return address onto stack
                {           // jump to the address
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC++;
                    // read address
                    read(pipeCPU[READ], &address, 4);
                    SP--;

                    instruction = WRITE_INSTRUCTION;
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &SP, 4);
                    write(pipeMemory[WRITE], &PC, 4);
                    PC = address;
                    break;
                }

                case 24:    // Pop return address from stack, jump to address
                {
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &SP, 4);
                    read(pipeCPU[READ], &address, 4);
                    // reads top of system stack
                    SP++;
                    PC = address;
                    break;
                }

                case 25:    // Increment the value in X
                {
                    X++;
                    break;
                }
                
                case 26:    // Decrement the value in X
                {
                    X--;
                    break;
                }

                case 27:    // Push AC onto stack
                {
                    SP--;
                    instruction = WRITE_INSTRUCTION;
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &SP, 4);
                    write(pipeMemory[WRITE], &AC, 4);
                    // at location SP in memory
                    break;
                }

                case 28:    // Pop from stack into AC
                {
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &SP, 4);
                    // read top of system stack
                    read(pipeCPU[READ], &AC, 4);
                    // read into AC
                    SP++;
                    break;
                }

                case 29:    // Perform system call
                {
                    // interrupt mode
                    mode = KERNEL_MODE;     // enter kernel mode
                    tempSP = SP;            // save SP
                    tempPC = PC;            // save PC
                    SP = system_stack;
                    PC = 1500;

                    // CPU writes saved SP to memory at top of system stack
                    instruction = WRITE_INSTRUCTION;
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &SP, 4);
                    write(pipeMemory[WRITE], &tempSP, 4);
                    SP--;

                    // CPU writes saved PC to memory at top of system stack
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &SP, 4);
                    write(pipeMemory[WRITE], &tempPC, 4);
                    break;
                }

                case 30:    // Return from system call
                {
                    mode = USER_MODE;   // enter user mode

                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &SP, 4);
                    read(pipeCPU[READ], &tempPC, 4);
                    SP++;
                    write(pipeMemory[WRITE], &instruction, 1);
                    write(pipeMemory[WRITE], &SP, 4);
                    read(pipeCPU[READ], &tempSP, 4);
                    SP++;
                    // CPU reads top of system stack

                    PC = tempPC;
                    SP = tempSP;    // restore saved values
                    break;
                }

                case 50:    // End execution
                {
                    instruction = EXIT_INSTRUCTION;  // exit
                    write(pipeMemory[WRITE], &instruction, 1);
                    // send Exit instruction to memory
                    ex = true;
                    break;
                }
            }
            if (ex)
                break;      // exits loop
        }
    }
}