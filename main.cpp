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
int const READ      = 0;
int const WRITE     = 1;
int const PIPE_ERROR        = -1;
int const CHILD_PID         =  0;
int const user_program      = 0;
int const system_program    = 1000;
int const max_line_size     = 1000;

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("I need 2 arguments to work. Try again.\n");
        printf("Input file & timer value\n");
        exit(EXIT_FAILURE);
    }

    if (pipe(pipeMemory) == PIPE_ERROR || pipe(pipeCPU) == PIPE_ERROR)
    {
        printf("Pipe failed. Try again.\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        printf("File does not exist. Try again.\n");
        exit(EXIT_FAILURE);
    }


    int pid = fork();

    if (pid < 0)
    {
        printf("Invalid process. Try again.");
        exit(EXIT_FAILURE);
    }

    // -----------------------------------------------------
    // -------------------- M E M O R Y --------------------
    // -----------------------------------------------------
    else if (pid == 0)
    {
        //printf("In MEMORY now!!!\n\n\n");
        int memory[2000];
        close(pipeMemory[1]);
        close(pipeCPU[0]);
        int current_program = user_program;
        char input [max_line_size];
        int size = sizeof(input);

        bool change_address = false;

        while(fgets(input, max_line_size, file) != NULL)
        {
            //printf("should be reading in now...\n");
            char temp[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
            int i = 0;
            //printf(input[i] + "\n");
            //cout << "i: " << i << endl << endl;
            change_address = false;

            if (input[0] == '.')
            {
                change_address = true;
                i++;
            }
            
            while (isdigit(input[i]))
            {
                //cout << "input[i]: " << input[i] << endl;
                temp[i] = input[i];
                //cout << "temp[i]: " << temp[i] << endl;
                i++;
            }

            //cout << "get past the second if" << endl;

            if (change_address)
            {
                //cout << "about to change address. size of temp is: " << sizeof(temp) << endl << endl;
                for (int j = 0; j < 5; j++)
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
        int address;
        int value;
        close(pipeMemory[1]);
        close(pipeCPU[0]);

        while (true)
        {
            read(pipeMemory[0], &instruction, sizeof(char));

            if (instruction == 'r')
            {
                read(pipeMemory[0], &address, sizeof(int));
                value = memory[address];
                write(pipeCPU[1], &value, sizeof(int));
            }
            else if (instruction == 'w')
            {
                read(pipeMemory[0], &address, sizeof(int));
                read(pipeMemory[0], &value, sizeof(int));
                memory[address] = value;
            }
            else if (instruction == 'e')
                break;
        }
    }
    
    // -----------------------------------------------------
    // ----------------------- C P U -----------------------
    // -----------------------------------------------------
    else
    {
        int timer = atoi(argv[2]);
        srand(time(0));         // set timer seed to time(0) (num of seconds since Jan 1, 1970)

        int user_stack   = 999;     // end of user memory (0 - 999)
        int system_stack = 1999;    // end of system memory (1000 - 1999)
        int mode = 0;               // 0 -> user mode | 1 -> kernel mode

        int PC = 0, SP = user_stack, IR = 0, AC = 0, X, Y;
        int tempPC, tempSP;
        char instruction;

        int value, address;
        bool ex = false;    // writes exits when true

        close(pipeMemory[0]);
        close(pipeCPU[1]);

        while(true)
        {
            //timer interrupt
            if ((mode == 0) && (PC == timer))   // checks if in user mode
            {
                mode = 1;           // switches to kernel mode
                timer += timer;
                tempPC = PC;
                tempSP = SP;
                SP = system_stack;
                PC = 1000;
                
                // push user sp into system stack
                instruction = 'w';
                write(pipeMemory[1], &instruction, sizeof(char));
                write(pipeMemory[1], &SP, sizeof(int));
                write(pipeMemory[1], &tempSP, sizeof(int));
                SP--;

                // push user pc into system stack
                write(pipeMemory[1], &instruction, sizeof(char));
                write(pipeMemory[1], &SP, sizeof(int));
                write(pipeMemory[1], &tempPC, sizeof(int));

                //continue;
            }

            instruction = 'r';
            write(pipeMemory[1], &instruction, sizeof(char));
            write(pipeMemory[1], &PC, sizeof(int));
            PC++;
            read(pipeCPU[0], &IR, sizeof(int));
            // reading instruction from memory into Instruction Register
            
            //instruction = 'r';
            //cout << "IR: " << IR << endl;





            // ---------------------------------------------------------------------------------------
            // ------------------- IR ------------
            // ---------------------------------------------------------------------------------------
            switch (IR)     // Instruction set
            {
                case 1:     // Load the value into the AC
                {
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;
                    read(pipeCPU[0], &value, sizeof(int));
                    AC = value;
                    //printf("AC after we set it = %d\n", AC);
                    break;
                }

                case 2:     // Load the value at the address into the AC
                {
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;
                    read(pipeCPU[0], &address, sizeof(int));
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &address, sizeof(int));
                    read(pipeCPU[0], &value, sizeof(int));
                    AC = value;
                    break;
                }

                case 3:     // Load the value from the address found
                {           // in the given address into the AC
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;
                    read(pipeCPU[0], &address, sizeof(int));
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &address, sizeof(int));

                    read(pipeCPU[0], &address, sizeof(int));
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &address, sizeof(int));
                    read(pipeCPU[0], &value, sizeof(int));
                    AC = value;
                    break;
                }
                
                case 4:     // Load the value at (address + X) into the AC
                {
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;

                    read(pipeCPU[0], &address, sizeof(int));
                    //cout << "Case 4: address BEFORE: " << address << endl;
                    address += X;
                    //cout << "Case 4: address AFTER: " << address << endl;
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &address, sizeof(int));
                    read(pipeCPU[0], &value, sizeof(int));
                    AC = value;
                    break;
                }  

                case 5:     // Load the value at (address + Y) into the AC
                {
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;

                    read(pipeCPU[0], &address, sizeof(int));
                    address += Y;
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &address, sizeof(int));
                    read(pipeCPU[0], &value, sizeof(int));
                    AC = value;
                    break;
                }

                case 6:     // Load from (SP + X) into the AC
                {
                    instruction = 'r';
                    address = SP + X;
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &address, sizeof(int));
                    read(pipeCPU[0], &value, sizeof(int));
                    AC = value;
                    break;
                }

                case 7:     // Store the value in the AC into the address
                {
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;
                    read(pipeCPU[0], &address, sizeof(int));
                    instruction = 'w';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &address, sizeof(int));
                    write(pipeMemory[1], &AC, sizeof(int));
                    break;
                }

                case 8:     // Gets a random int from 1 to 100 into the AC
                {
                    AC = rand() % 100 + 1;
                    printf("AC = %d\n", AC);
                    break;
                }

                case 9:     // If port = 1, writes AC as an int to the screen
                {           // If port = 2, write AC as a char to the screen
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;
                    read(pipeCPU[0], &value, sizeof(int));
                    
                    if (value == 1)
                        printf("%d",AC);
                        
                    else if (value == 2)
                        printf("%c",AC);
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
                    //printf("X before we set it = %d\n", X);
                    X = AC;
                    //printf("X after we set it = %d\n", X);
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
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;

                    read(pipeCPU[0], &address, sizeof(int));
                    PC = address;
                    break;
                }

                case 21:    // Jump to the address only if the
                {           // value in the AC is zero
                    if (AC == 0)
                    {
                        instruction = 'r';
                        write(pipeMemory[1], &instruction, sizeof(char));
                        write(pipeMemory[1], &PC, sizeof(int));
                    }
                    PC++;

                    if (AC == 0)
                    {
                        read(pipeCPU[0], &address, sizeof(int));
                        PC = address;
                    }
                    break;
                }

                case 22:    // Jump to the address only if the
                {           // value in the AC is not zero
                    if (AC != 0)
                    {
                        instruction = 'r';
                        write(pipeMemory[1], &instruction, sizeof(char));
                        write(pipeMemory[1], &PC, sizeof(int));
                    }
                    PC++;
                    if (AC != 0)
                    {
                        read(pipeCPU[0], &address, sizeof(int));
                        PC = address;
                    }
                    break;
                }

                case 23:    // Push return address onto stack
                {           // jump to the address
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC++;
                    read(pipeCPU[0], &address, sizeof(int));
                    SP--;

                    instruction = 'w';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &SP, sizeof(int));
                    write(pipeMemory[1], &PC, sizeof(int));
                    PC = address;
                    break;
                }

                case 24:    // Pop return address from stack, jump to address
                {
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &SP, sizeof(int));
                    read(pipeCPU[0], &address, sizeof(int));
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
                    instruction = 'w';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &SP, sizeof(int));
                    write(pipeMemory[1], &AC, sizeof(int));
                    break;
                }

                case 28:    // Pop from stack into AC
                {
                    instruction = 'r';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &SP, sizeof(int));
                    read(pipeCPU[0], &AC, sizeof(int));
                    SP++;
                    break;
                }

                case 29:    // Perform system call
                {
                    // similar to interrupt
                    mode = 1;   // enter kernel mode
                    tempSP = SP;    // save SP and PC
                    tempPC = PC;
                    SP = system_stack;
                    PC = 1500;

                    instruction = 'w';
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &SP, sizeof(int));
                    write(pipeMemory[1], &tempSP, sizeof(int));
                    SP--;
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &SP, sizeof(int));
                    write(pipeMemory[1], &tempPC, sizeof(int));
                    break;
                }

                case 30:    // Return from system call
                {
                    instruction = 'r';
                    mode = 0;   // enter user mode
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &SP, sizeof(int));
                    read(pipeCPU[0], &tempPC, sizeof(int));
                    SP++;
                    write(pipeMemory[1], &instruction, sizeof(char));
                    write(pipeMemory[1], &SP, sizeof(int));
                    read(pipeCPU[0], &tempSP, sizeof(int));
                    SP++;

                    PC = tempPC;
                    SP = tempSP;    // restore saved values
                    break;
                }

                case 50:    // End execution
                {
                    instruction = 'e';  // exit
                    write(pipeMemory[1], &instruction, sizeof(char));
                    ex = true;
                    break;
                }
            }
            if (ex)
                break;

            // ---------------------------------------------------------------------------------------
            // -------------------------------------IR------------------------------------------------
            // ---------------------------------------------------------------------------------------
        }
    }
}