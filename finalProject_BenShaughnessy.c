//Ben Shaughnessy
// CS 350, Fall 2014
// Final Project
//
// Illinois Institute of Technology, (c) 2014, James Sasaki
#include <stdio.h>
#include <stdlib.h> // For error exit()
#include <string.h>
#include <errno.h>
// CPU Declarations -- a CPU is a structure with fields for the
// different parts of the CPU.
//
typedef short int Word; // type that represents a word of SDC memory
typedef unsigned short int Address; // type that represents an SDC address

#define MEMLEN 65536
#define NREG 8

typedef struct
{
    //Word mem[MEMLEN];
    //Word reg[NREG]; // Note: "register" is a reserved word
    int mem[MEMLEN];
    int reg[NREG]; // Note: "register" is a reserved word
    Address pc;
    int cc; // Program Counter
    int running; // running = 1 iff CPU is executing instructions
    Word ir; // Instruction Register
    int instr_sign;
    int opcode;
    int reg_R;
    int addr_MM;
    //Address cache0;
    //short int cache1;
} CPU;
// Prototypes [note the functions are also declared in this order]
//
int main(int argc, char *argv[]);
void initialize_control_unit(CPU *cpu);
void initialize_memory(int argc, char *argv[], CPU *cpu);
FILE *get_datafile(int argc, char *argv[]);
void dump_control_unit(CPU *cpu);
void dump_memory(CPU *cpu);
void dump_registers(CPU *cpu);
int read_execute_command(CPU *cpu);
int execute_command(char cmd_char, CPU *cpu, char leadChar, short unsigned int location, short unsigned int value);
void help_message(void);
void many_instruction_cycles(int nbr_cycles, CPU *cpu);
void one_instruction_cycle(CPU *cpu);
void exec_HLT(CPU *cpu);

//functions for commands
void addHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void trapHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void leaHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void jmpHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void stiHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void notHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void strHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void ldrHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void andHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void jsrHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void stHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void ldHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);
void branchHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr);

// Main program: Initialize the cpu, read initial memory values,
// and execute the read-in program starting at location 00.
//


ssize_t getdelim(char **linep, size_t *n, int delim, FILE *fp){
    int ch;
    size_t i = 0;
    if(!linep || !n || !fp){
        errno = EINVAL;
        return -1;
    }
    if(*linep == NULL){
        if(NULL==(*linep = malloc(*n=128))){
            *n = 0;
            return -1;
        }
    }
    while((ch = fgetc(fp)) != EOF){
        if(i == *n - 1){
            char *temp = realloc(*linep, *n += 128);
            if(!temp){
                free(*linep);
                return -1;
            }
            *linep = temp;
        }
        (*linep)[i++] = ch;
        if(ch == delim)
            break;
    }
    (*linep)[i] = '\0';
    return !i && ch == EOF ? -1 : i;
}
ssize_t getline(char **linep, size_t *n, FILE *fp){
    return getdelim(linep, n, '\n', fp);
}


int main(int argc, char *argv[])
{
    printf("LC3 Simulator: CS 350 Lab 8 Ben Shaughnessy\n");
    CPU cpu_value, *cpu = &cpu_value;
    initialize_control_unit(cpu);
    initialize_memory(argc, argv, cpu);
    dump_control_unit(cpu);
    dump_memory(cpu);
    char *prompt = "> ";
    printf("\nBeginning execution; type h for help\n%s", prompt);
    int done = read_execute_command(cpu);
    while (!done) {
        printf("%s", prompt);
        done = read_execute_command(cpu);
    }
    return 0;
}
// Initialize the control registers (pc, ir, running flag) and
// the general-purpose registers
//
void initialize_control_unit(CPU *cpu)
{
    (*cpu).pc = 0;
    (*cpu).ir = 0;
    (*cpu).cc = 0;
    (*cpu).running = 1;
    int i;
    for (i = 0; i < NREG; i++) {
        cpu->reg[i] = 0;
    }
}
// Read and dump initial values for memory
//
void initialize_memory(int argc, char *argv[], CPU *cpu)
{
    FILE *datafile = get_datafile(argc, argv);
    // Will read the next line (words_read = 1 if it started
    // with a memory value). Will set memory location loc to
    // value_read
    //
    int value_read, words_read, loc = 0, done = 0;
    // Each getline automatically reallocates buffer and
    // updates buffer_len so that we can read in the whole line
    // of input. bytes_read is 0 at end-of-file. Note we must
    // free the buffer once we're done with this file.
    //
    // See linux command man 3 getline for details.
    //
    char *buffer = NULL;
    size_t buffer_len = 0, bytes_read = 0;
    int i, initial = 1;
    // Read in first and succeeding memory values. Stop when we
    // hit a sentinel value, fill up memory, or hit end-of-file.
    //
    for (i = 0; i < MEMLEN; i++)
    (*cpu).mem[i] = 0;
    bytes_read = getline(&buffer, &buffer_len, datafile);
    while (bytes_read != -1 && !done){
        // If the line of input begins with an integer, treat
        // it as the memory value to read in. Ignore junk
        // after the number and ignore blank lines and lines
        // that don't begin with a number.
        //
        words_read = sscanf(buffer, "%x", &value_read);
        if(words_read == 1)
        {
            if (initial && value_read < MEMLEN && value_read > 0)
            {
                loc = value_read;
                initial = 0;
                (*cpu).pc = value_read;
            }
            else if(value_read < 0 || value_read >= MEMLEN)
            {
                printf("sentinel %d found at location %d\n", value_read, loc);
                break;
            }
            else if(loc >= MEMLEN)
            {
            loc = 0;
                (*cpu).mem[loc++] = value_read;
            }
            else
            {
                (*cpu).mem[loc++] = value_read;
            }
        }
    // *** STUB *** set memory value at current location to
    // value_read and increment location. Exceptions: If
    // loc is out of range, complain and quit the loop. If
    // value_read is outside -9999...9999, then it's a
    // sentinel and we should say so and quit the loop.
    // Get next line and continue the loop
    //
        bytes_read = getline(&buffer, &buffer_len, datafile);
        }
        free(buffer); // return buffer to OS
        // Initialize rest of memory
        //
}
// Get the data file to initialize memory with. If it was
// specified on the command line as argv[1], use that file
// otherwise use default.sdc. If file opening fails, complain
// and terminate program execution with an error.
// See linux command man 3 exit for details.
//
FILE *get_datafile(int argc, char *argv[])
{
    char *default_datafile_name = "program.hex";
    char *datafile_name;

    if(!argv[1])
        datafile_name = default_datafile_name;
    else
        datafile_name = argv[1];
    printf("Loading %s\n\n", datafile_name);
    FILE *datafile = fopen(datafile_name, "r");

    if(!datafile)
    {
    printf("File failed to open \n");
    exit(EXIT_FAILURE);
    // exit(EXIT_FAILURE); to quit the entire program
    return datafile;
    }
}
// dump_control_unit(CPU *cpu): Print out the control and
// general-purpose registers
//
void dump_control_unit(CPU *cpu)
{
    printf("CONTROL UNIT:\n");
    printf("pc : x%04x IR : x%04x cc : %d RUNNING : %d", (*cpu).pc, (*cpu).ir, cpu->cc, (*cpu).running);
    dump_registers(cpu);
}
// dump_memory(CPU *cpu): Print memory values in a table, ten per
// row, with a space between each group of five columns and with
// a header column of memory locations.
//
void dump_memory(CPU *cpu)
{
    int loc = 0;
    int row, col, i;
    printf("\n\nMEMORY (addresses x0000 - xFFFF)\n");
    // *** ****
    // for (row = 0; row < 100; row +=10)
    // {
    // printf("%02d:",row);
    // for(col=row; col < row +10; col++)
    // {
    // if(col%10 == 5) printf(" ");
    // printf("%6d",(*cpu).mem[col]);
    // }
    // printf("\n");
    // }
    for (i = 0; i < MEMLEN; i++){
        // while((*cpu).mem[i] == 0) i++;
        if ((*cpu).mem[i] != 0){
            printf("x%04x: x%04x %x\n",i,abs((*cpu).mem[i]), abs((*cpu).mem[i]));
        }
    }
}
// dump_registers(CPU *cpu): Print register values in two rows of
// five.
//
void dump_registers(CPU *cpu)
{
    // *** ****
    int i;
    for (i = 0; i < NREG; i++)
    {
    if(i%4 == 0) printf("\n");
    printf("R%d x%04x %x ", i, abs((*cpu).reg[i]), abs((*cpu).reg[i]));
    }
}
// Read a simulator command from the keyboard ("h", "?", "d", number,
// or empty line) and execute it. Return true if we hit end-of-input
// or execute_command told us to quit. Otherwise return false.
//
int read_execute_command(CPU *cpu)
{
    // Buffer for the command line from the keyboard, plus its size
    //
    char *cmd_buffer = NULL;
    size_t cmd_buffer_len = 0, bytes_read = 0;
    // Values read using sscanf of command line
    //
    int nbr_cycles;
    char cmd_char;
    char leadChar;
    short unsigned int location;
    short unsigned int value;
    size_t words_read; // number of items read by sscanf call
    int done = 0; // Should simulator stop?
    bytes_read = getline(&cmd_buffer, &cmd_buffer_len, stdin);
    if (bytes_read == -1) {
        done = 1; // Hit end of file
    }

    words_read = sscanf(cmd_buffer, "%d", &nbr_cycles);
    // *** **** If we found a number, do that many

    if (words_read == 1)
    many_instruction_cycles(nbr_cycles, cpu);

    else
    {
        words_read = sscanf(cmd_buffer, "%c %c%hx x%hx", &cmd_char, &leadChar, &location, &value);

        if(cmd_char == 'j' && leadChar=='x' && words_read != 3) {
            printf("the jump command needs a location. 'j x1234'\n");
        } else if(cmd_char == 'm' && leadChar == 'x' && words_read != 4) {
            printf("the memory command needs a location and a value. example:  'm x3005 x00FF\n");
        } else if(cmd_char == 'r' && leadChar == 'r' && words_read != 4) {
            printf("the r command needs a register and a value. Example: 'r r5 x00F3\n");
        } else {
            done = execute_command(cmd_char, cpu, leadChar, location, value);
        }
        // instruction cycles. Otherwise sscanf for a character
        // and call execute_command with it. (Note the character
        // might be '\n'.)
        free(cmd_buffer);
        return done;
    }
}

void mCommand(CPU *cpu, short unsigned int location, short unsigned int value) {
        if (location >= MEMLEN || location < 0){
            printf("invalid location in memory\n");
        }
        else if (value >= MEMLEN || location < 0){
            printf("invalid value to store in memory\n");
        }
        else {
            cpu -> mem[location] = abs(value);
            printf("set mem[x%04hx] to x%04hx (%d) mem[x%04x] = %04x\n", location, value, value, location, cpu->mem[location]);
        }
}
void jCommand(CPU *cpu, short unsigned int location, short unsigned int value) {
        if(location >= MEMLEN || location < 0) {
            printf("invalid memory locations\n");
        } else {
            (*cpu).pc = location;
            printf("jumped to x%04x\n", (*cpu).pc);
        }
}
void rCommand(CPU *cpu, short unsigned int location, short unsigned int value) {
        if (location <= 0 || location > 7){
            printf("invalid register value: %d\n", location);
        }
        else if (value >= MEMLEN || value < 0){
            printf("invalid value to store in memory\n");
        }
        else {
            (*cpu).reg[location] = value;
            printf("setting r%d to x%04hx\n", location, value);
        }
}
// Execute a nonnumeric command; complain if it's not 'h', '?', 'd', 'q' or '\n'
// Return true for the q command, false otherwise
//
int execute_command(char cmd_char, CPU *cpu, char leadChar, short unsigned int location, short unsigned int value)
{
    if (cmd_char == '?' || cmd_char == 'h')
    {
        help_message();
    }
    else if(cmd_char=='q')
    {
        printf("Quit \n");
        return 1;
    }
    else if(cmd_char =='d')
    {
        dump_control_unit(cpu);
        dump_memory(cpu);
    }
    else if (cmd_char == '\n')
    {
        one_instruction_cycle(cpu);
    }
    else if(cmd_char == 'm') {
            mCommand(cpu,location,value);
    }
    else if(cmd_char - 'r' == 0) {
        rCommand(cpu,location,value);
    }
    else if(cmd_char == 'j') {
        jCommand(cpu,location,value);
    }
    else
        printf("%c not a valid command \n",cmd_char);
    return 0;
}
// Print standard message for simulator help command ('h' or '?')
//
void help_message(void)
{
    printf("h or ? for help (prints this message)\n");
    printf("q to quit\n");
    printf("d to dump the control unit and mammary\n");
    printf("\nNot Suppoerted yet:\n");
    printf("j xNNNN to jump to new location\n");
    printf("m xNNNN xMMMM to assign memory location xNNNN = value xMMMM\n");
    printf("An integer >0 to execute that many instruction cycles\n");
    printf("Or just return, which executes one instruction cycle\n");
}
// Execute a number of instruction cycles. Exceptions: If the
// number of cycles is <= 0, complain and return; if the CPU is
// not running, say so and return; if the number of cycles is
// insanely large, complain and substitute a saner limit.
//
// If, as we execute the many cycles, the CPU stops running,
// then return.
//
void many_instruction_cycles(int nbr_cycles, CPU *cpu) {
    if(nbr_cycles <= 0) {
        printf("Number of instruction cycles > 0\n");
        return;
    }
    if((*cpu).running == 0)
    {
        printf("CPU has been halted\n");
        return;
    }
    if(nbr_cycles > 100)
    {
        printf("%d is too large for cycle; doing %d \n",nbr_cycles,MEMLEN);
    }
    int count;
    printf("executing %d cycles\n", nbr_cycles);
    for(count = 0; count < nbr_cycles; count++) {
        if((*cpu).running == 0) break;
        one_instruction_cycle(cpu);
    }
    return;
}
// Execute one instruction cycle
//
//* temporary commented

//this method is used to take in an array of charachtars (string) containing the 16bit binary code
//for an instruction, then take out a select group of bits and return it in the form of an int.
int getSelectIntOld(char* str, int start, int numb) {
    int opcodeInt = 0;

    int i =0x0;
    for(i = start; i < numb + start; i++) {
        opcodeInt = opcodeInt + str[i] - '0';
        opcodeInt = opcodeInt * 10;
        //printf("opcode: %d strloc: %c\n", opcodeInt, str[i]);
    }
    //printf("\n");
    opcodeInt = opcodeInt/10;

    //printf("the new opcode int maybe? %d\n", opcodeInt);

    return opcodeInt;
}

//This function takes the 7 chars starting with str[start]
//enter 0 in isSigned to get an unsigned int out, and 1 to make it signed
//if it is signed then the first char (str[start]) is the sign bit.
//if the number that comes out is negative, the function will return the base ten negative of
//the positive of the binary. so since 0xD is -3, th function will returnn -11;
int getSelectInt(char* str, int start, int numb, int isSigned) {
    if(isSigned == 0) {
        int opcodeInt = 0;

        int i = 0;
        for(i = start; i < numb + start; i++) {
            opcodeInt = opcodeInt + str[i] - '0';
            opcodeInt = opcodeInt * 10;
        }
        opcodeInt = opcodeInt/10;

        return opcodeInt;
    } else {
        //check the first number
        if(str[start] - '0' == 0) {
            //positive
            int opcodeInt = 0;

            //printing the bitset in question
            printf("\n");

            int i = 0;
            for(i = start + 1; i < numb + start; i++) {
                opcodeInt = opcodeInt + str[i] - '0';
                opcodeInt = opcodeInt * 10;
            }
            opcodeInt = opcodeInt/10;

            return opcodeInt;
        } else if (str[start] - '0' == 1) {
            //is a negative number

            //starting off by looping through until we can subtract 1
            int i = 0;
            int endloc = 0;
            for(i = start + numb - 1; i >= start; i--) {
                int currentBit = str[i] - '0';
                if(currentBit == 1) {
                    //a bit can be subtracted at str[i] by swapping it with a '0';
                    str[i] = '0';
                    endloc = i;
                    i = start - 1;
                }
            }

            //carrying the 1
            for(i = endloc + 1; i < start + numb; i++) {
                str[i] = '1'; //carrying the one over, but since we are subtracting by 1, we change
                              //all bit to the right to a 1 since they had to have been 0
            }

            //find the decimal value the same way except invert the char at every step.
            int opcodeInt = 0;
            for(i = start + 1; i < numb + start; i++) {
                opcodeInt = opcodeInt + 1 - (str[i] - '0');
                opcodeInt = opcodeInt * 10;
            }
            opcodeInt = opcodeInt/10;

            return -opcodeInt;
        } else {
            printf("error in bitstring");
            return 0;
        }
        return 0;
            //if 1, flip the rest
            //if 0, continue as normal
    }

}

//helper function for taking the power
int power(int a, int b) {
    int i = 0;
    int total = 1;
    for(i = 0; i < b; i++) {
    total = total * a;
    }

    return total;
}

//converts an into to an int of 1's and 0's representing the binary of that decimal input
int binaryToDecimal(int n) /* Function to convert binary to decimal.*/
{
    int decimal=0, i=0, rem;
    while (n!=0){
        rem = n%10;
        n/=10;
        decimal += rem*power(2,i);
        ++i;
    }
    return decimal;
}

//gets a bitstring give the hex ir as input
char* getBitstringFromInstr(unsigned short int instr) {
    //this is what the code below is going to convert into. It is set to default
    //as a 16 bit string full of zeros to act as a safety default.
    char* binaryNumber = (char*) malloc(17*sizeof(char));//create dynamic char sequence
    memset(binaryNumber, '0', 16); //assign the default value with String copy

    //array for converting hex values to strings of binary
    char *convert[16];
    convert[0] = "0000";
    convert[1] = "0001";
    convert[2] = "0010";
    convert[3] = "0011";
    convert[4] = "0100";
    convert[5] = "0101";
    convert[6] = "0110";
    convert[7] = "0111";
    convert[8] = "1000";
    convert[9] = "1001";
    convert[10] = "1010";
    convert[11] = "1011";
    convert[12] = "1100";
    convert[13] = "1101";
    convert[14] = "1110";
    convert[15] = "1111";

    int i = 0; //for looping through the hex word
    for(i = 0; i < 4; i++) {
        int mask = 0xf << (12 - (4 * i));
        int a = instr & mask;
        a = a >> (12 - (4 * i));

        //this gets the binary replacement for the hex char
        char *newstring = convert[a];

        //replacing the part of the main string with the substring
        int n = 0;
        for(n = 0; n < 4; n++) {
            binaryNumber[4*i + n] = newstring[n];
        }
    }

    return binaryNumber;
}


//code for dealing with Adding
void addHanding(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {
//getting the indicator bit
        int indicatorBit = getSelectInt(binaryNumber, 10, 1, 0);

        //get Dst and Src1 registers
        int dst = getSelectInt(binaryNumber, 4, 3, 0);
        int src1 = getSelectInt(binaryNumber, 7, 3, 0);

        if(indicatorBit == 0) {
            //Add with Registers
            int src2 = getSelectInt(binaryNumber, 13, 3, 0);

            //doing the operation
            cpu->reg[dst] = cpu->reg[binaryToDecimal(src1)] + cpu->reg[binaryToDecimal(src2)];

            if(cpu->reg[dst] < 0) {
                cpu->cc = -1;
            } else if(cpu->reg[dst] > 0) {
                cpu->cc = 1;
            } else {
                cpu->cc = 0;
            }

            //printing out message to user:
            printf("at x%04x: ir x%04x; ADD R%d R%d R%d; %d <- %d + %d = %d;\n",instr_loc, instr, binaryToDecimal(dst),
                                                            binaryToDecimal(src1),
                                                            binaryToDecimal(src2),
                                                            binaryToDecimal(dst),
                                                            cpu->reg[binaryToDecimal(src1)],
                                                            cpu->reg[binaryToDecimal(src2)],
                                                            cpu->reg[binaryToDecimal(dst)]);

        } else if (indicatorBit == 1) {
            //Add Immediate
            int immed5 = getSelectInt(binaryNumber, 11, 5, 1);

            //doing the operation
            cpu->reg[dst] = cpu->reg[binaryToDecimal(src1)] + binaryToDecimal(immed5);


            if(cpu->reg[dst] < 0) {
                cpu->cc = -1;
            } else if(cpu->reg[dst] > 0) {
                cpu->cc = 1;
            } else {
                cpu->cc = 0;
            }

            printf("at x%04x: ir x%04x; ADD R%d R%d %d; R%d <- %d + %d = %d;\n",instr_loc, instr, binaryToDecimal(dst),
                                                            binaryToDecimal(src1),
                                                            binaryToDecimal(immed5),
                                                            binaryToDecimal(dst),
                                                            cpu->reg[binaryToDecimal(src1)],
                                                            binaryToDecimal(immed5),
                                                            cpu->reg[binaryToDecimal(dst)]);
        } else {
            //if this triggers then it could not read a 1 or 0 at the indicator bit
            printf("there was an unexpected error with adding!\n");
        }
}

void trapHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {
        int trapvec = getSelectInt(binaryNumber, 8, 8, 0);

        int hextrapvec = binaryToDecimal(trapvec);
        if(hextrapvec == 0x20) {
            //GETC

            //saving the program counter
            cpu->reg[7] = cpu->pc;

            //getting a char from the user and printing it out
            char inputChar = getchar();
            cpu->reg[0] = inputChar;

            //message
            printf("\nat x%04x: ir x%04x; TRAP TrapVec8: x%02x GETC\n",instr_loc, instr, binaryToDecimal(trapvec));
        } else if (hextrapvec == 0x21) {
            //OUT

            //saving the program counter
            cpu->reg[7] = cpu->pc;

            //getting the char at the rightmost bits of r0 and printing it out
            char a = cpu->reg[0] & 0x00ff;
            printf("%c", a);

            //message
            printf("\nat x%04x: ir x%04x; TRAP TrapVec8: x%02x OUT. R0: x%04x\n", instr_loc, instr, binaryToDecimal(trapvec), cpu->reg[0]);
        } else if (hextrapvec == 0x22) {
            //PUTS

            //saving the program counter
            cpu->reg[7] = cpu->pc;

            //loops through memory starting at the location of R0 and prints out the char in the rightmost 8 bits until it hits the null char
            int readLocation = cpu->reg[0];
            char printChar = cpu->mem[readLocation] & 0x00ff;
            while(printChar != 0) {
                printf("%c", printChar);
                readLocation = readLocation + 1;
                printChar = cpu->mem[readLocation] & 0x00ff;
            }

            //message
            printf("\nat x%04x: ir x%04x; TRAP TrapVec8: x%02x PUTS\n",instr_loc, instr,  binaryToDecimal(trapvec));
        } else if (hextrapvec == 0x23) {
            //IN

            //saving the program counter
            cpu->reg[7] = cpu->pc;

            //prompt user, getting a char from the user and printing it out
            printf("Enter a character:");
            char inputChar = getchar();
            cpu->reg[0] = inputChar;

            printf("\nat x%04x: ir x%04x; TRAP TrapVec8: x%02x IN\n",instr_loc, instr,  binaryToDecimal(trapvec));
        } else if (hextrapvec == 0x25) {
            //HALT
            cpu->cc = 1;

            exec_HLT(cpu); //halts the program
            printf("\nat x%04x: ir x%04x; TRAP TrapVec8: x%02x HALT\n",instr_loc, instr,  binaryToDecimal(trapvec));
        } else {
            printf("\nat x%04x: ir x%04x; TRAP TrapVec8: x%02x Unknown Trap Vector\n",instr_loc, instr,  binaryToDecimal(trapvec));
        }
}

void leaHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {
        //getting the src register and pcoffset
        int dst = binaryToDecimal(getSelectInt(binaryNumber, 4, 3, 0));
        int pcoffset9 = binaryToDecimal(getSelectInt(binaryNumber, 7, 9, 1));

        cpu-> reg[dst] = cpu->pc + pcoffset9;

        if(cpu->reg[dst] < 0) {
            cpu->cc = -1;
        } else if(cpu->reg[dst] > 0) {
            cpu->cc = 1;
        } else {
            cpu->cc = 0;
        }


        //printing out the message
        printf("at x%04x: ir x%04x; LEA R%d <- PC + %d = %04x\n",instr_loc, instr, dst, pcoffset9, cpu->reg[dst]);

}

void jmpHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {

        //getting the dst reg, src reg, and the extra 6 1's
        int base = binaryToDecimal(getSelectInt(binaryNumber, 7, 3, 0));
        int extra = binaryToDecimal(getSelectInt(binaryNumber, 10, 6, 0));

        cpu->pc = cpu->reg[base];

        //printing out the message
        printf("at x%04x: ir x%04x; JMP PC <- R%d ; PC <- %04x\n", instr_loc, instr,base, cpu->pc);

}

void stiHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {
        //STI
        //getting the src register and pcoffset
        int src = getSelectInt(binaryNumber, 4, 3, 0);
        int pcoffset9 = getSelectInt(binaryNumber, 7, 9, 1);

        cpu->mem[cpu->mem[cpu->pc + pcoffset9]] = cpu->reg[src];
        //printing out the message
        //printf("at x%04x: %04x; STI SRC: %d pcoffset9: %d\n",instr_loc, instr, binaryToDecimal(src), binaryToDecimal(pcoffset9));
        printf("at x%04x: ir x%04x; STI M[M[PC+%d] <- R%d ; M[M[%04x]] = M[%04x] <-%04x\n",instr_loc, instr, pcoffset9, src, cpu->pc + pcoffset9, cpu->mem[cpu->pc + pcoffset9], cpu->reg[src] = cpu->reg[src]);

}

void notHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {
        //getting the dst reg, src reg, and the extra 6 1's
        int dst = binaryToDecimal(getSelectInt(binaryNumber, 4, 3, 0));
        int src1 = binaryToDecimal(getSelectInt(binaryNumber, 7, 3, 0));
        int extra = binaryToDecimal(getSelectInt(binaryNumber, 10, 6, 1));

        unsigned short int a= cpu->reg[src1];
        cpu->reg[dst] = (~a) & 0xffff; //mask to get rid of the extra bits outside 2 bytes

        if(cpu->reg[dst] < 0) {
            cpu->cc = -1;
        } else if(cpu->reg[dst] > 0) {
            cpu->cc = 1;
        } else {
            cpu->cc = 0;
        }

        //printing out the message
        printf("at x%04x: ir x%04x; NOT R%d <- NOT R%d; =  R%d <- NOT %04x; R%d = %04x\n",instr_loc, instr, dst, src1, dst, cpu->reg[src1], dst, cpu->reg[dst]);

}

void strHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {
        //dst reg, base reg, and offset6 for LDR with a helper function
        int src = binaryToDecimal(getSelectInt(binaryNumber, 4, 3, 0));
        int base = binaryToDecimal(getSelectInt(binaryNumber, 7, 3, 0));
        int offset = binaryToDecimal(getSelectInt(binaryNumber, 10, 6, 1));

        cpu->mem[cpu->reg[base] + offset] = cpu->reg[src];

        printf("\nat x%04x: ir x%04x;  STR  mem[R%d + offset:%d] <-R%d = %04x\n",instr_loc, instr, base, offset, src, cpu->reg[src]);

}


void ldrHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {

        //dst reg, base reg, and offset6 for LDR with a helper function
        int dst = binaryToDecimal(getSelectInt(binaryNumber, 4, 3, 0));
        int base = binaryToDecimal(getSelectInt(binaryNumber, 7, 3, 0));
        int offset = binaryToDecimal(getSelectInt(binaryNumber, 10, 6, 1));

        cpu->reg[dst] = cpu->mem[cpu->reg[base] + offset];

        if(cpu->reg[dst] < 0) {
            cpu->cc = -1;
        } else if(cpu->reg[dst] > 0) {
            cpu->cc = 1;
        } else {
            cpu->cc = 0;
        }

        printf("\nat x%04x: ir x%04x; LDR R%d <- mem[R%d + offset:%d] = %04x\n",instr_loc, instr, dst, base, offset, cpu->reg[dst]);


}

void andHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {

        //getting the selector, dst reg, and src1 reg from the bitstring
        int indicator = binaryToDecimal(getSelectInt(binaryNumber, 10, 1, 0));
        int dst = binaryToDecimal(getSelectInt(binaryNumber, 4, 3, 0));
        int src1 = binaryToDecimal(getSelectInt(binaryNumber, 7, 3, 0));

        //if indicator == 0 then program will use and with two src registers
        if(indicator == 0) {
            int src2 = binaryToDecimal(getSelectInt(binaryNumber, 13, 3, 0));

            //doing the operation
            cpu->reg[dst] = cpu->reg[src1] & cpu->reg[src2];


            if(cpu->reg[dst] < 0) {
                cpu->cc = -1;
            } else if(cpu->reg[dst] > 0) {
                cpu->cc = 1;
            } else {
                cpu->cc = 0;
            }

            printf("at x%04x: ir x%04x; AND R%d <- R%d & R%d; R%d <- %d & %d = %d\n",instr_loc, instr, dst, src1, src2, dst, cpu->reg[src1], cpu->reg[src2], cpu->reg[dst]);
        //if indicator == 1 then program will use immediate AND
        } else if (indicator == 1) {
            //Add Immediate
            int immed5 = binaryToDecimal(getSelectInt(binaryNumber, 11, 5, 1));

            //doing the operation
            cpu->reg[dst] = cpu->reg[src1] & immed5;

            if(cpu->reg[dst] < 0) {
                cpu->cc = -1;
            } else if(cpu->reg[dst] > 0) {
                cpu->cc = 1;
            } else {
                cpu->cc = 0;
            }

            printf("at x%04x: ir x%04x; AND R%d <- R%d & immed5; R%d <- %d & %d = %d\n",instr_loc, instr, dst, src1, dst, cpu->reg[src1], immed5, cpu->reg[dst]);
        } else {
            printf("Unknown problem with AND");
        }

}

void jsrHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {

        //getting the indicator bit for jsr
        //a 1 indicates that it uses PCoffset, 0 indicates a base reg for jumping to subroutine
        int indicator = getSelectInt(binaryNumber, 4, 1, 0);

        //if 1 then jsr will use pcoffset
        if(indicator == 1) {
            //getting the pcoffset for jsr
            int pcoffset = getSelectInt(binaryNumber, 5, 11, 1);

            //printing out a message
            printf("at x%04x: ir x%04x; JSR pcoffset: %d\n", binaryToDecimal(pcoffset));
        //if not, then it jsr will use a base reg
        } else if(indicator == 0) {
            //getting the base reg for jsr
            int basereg = getSelectInt(binaryNumber, 7, 3, 0);
            printf("at x%04x: ir x%04x; JSR Base Reg: %d\n",instr_loc, instr, binaryToDecimal(basereg));
        //indicator bit is not a zero or a 1, and we will have big problems if this is true
        } else {
            printf("Error with JSR! bad things have happened\n");
        }

}

void stHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {

        //getting the src register and pcoffset
        int src = getSelectInt(binaryNumber, 4, 3, 0);
        int pcoffset = getSelectInt(binaryNumber, 7, 9, 1);
        int src_dec = binaryToDecimal(src); //same as above but in decimal form
        int pcoffset_dec = binaryToDecimal(pcoffset); //in decimal form

        //executing instruction
         cpu->mem[cpu->pc + pcoffset_dec] = cpu->reg[src_dec];

        //printing out the message
        printf("at x%04x: ir x%04x; ST  mem[pc + x%04x] <- R%d; mem[x%04x] <- %d = %d\n",instr_loc, instr, pcoffset_dec,
                                                                src_dec,
                                                                (cpu->pc + pcoffset_dec),
                                                                cpu->reg[src_dec],
                                                                cpu->mem[cpu->pc + pcoffset_dec]);

}

void ldHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {

        //getting the dst register and pcoffset
        int dst = getSelectInt(binaryNumber, 4, 3, 0);
        int pcoffset = getSelectInt(binaryNumber, 7, 9, 1);
        int dst_dec = binaryToDecimal(dst); //same as above but in decimal form
        int pcoffset_dec = binaryToDecimal(pcoffset); //in decimal form

        //executing the command
        cpu->reg[dst_dec] = cpu->mem[cpu->pc + pcoffset_dec];


        if(cpu->reg[dst] < 0) {
            cpu->cc = -1;
        } else if(cpu->reg[dst] > 0) {
            cpu->cc = 1;
        } else {
            cpu->cc = 0;
        }

        //printing out the message
        printf("at x%04x: ir x%04x; LD R%d <- mem[pc + %d]\n",instr_loc, instr, dst_dec, pcoffset_dec);

}

void branchHandling(char *binaryNumber, CPU *cpu, int instr_loc, unsigned short int instr) {
        int nBit = binaryToDecimal(getSelectInt(binaryNumber, 4, 1, 0));
        int zBit = binaryToDecimal(getSelectInt(binaryNumber, 5, 1, 0));
        int pBit = binaryToDecimal(getSelectInt(binaryNumber, 6, 1, 0));
        int offset9 = binaryToDecimal(getSelectInt(binaryNumber, 7, 9, 1));

        int i = 2;
        char Mnemonic[] = "BR\0\0\0\0";
        if(nBit != 0) {Mnemonic[i] = 'N'; i++;}
        if(zBit != 0) {Mnemonic[i] = 'Z'; i++;}
        if(pBit != 0) {Mnemonic[i] = 'P'; i++;}

        if((nBit != 0 && cpu->cc == -1) || (zBit != 0 && cpu->cc == 0) || (pBit != 0 && cpu->cc == 1)) {
            cpu->pc = cpu->pc + offset9;
            printf("at x%04x: %04x; %s pc = pc + %d. CC: cpu->cc: %d\n",instr_loc, instr, Mnemonic, offset9, cpu->cc);
        } else {
            printf("at x%04x: ir x%04x; NOP not branching\n",instr_loc, instr);
        }
}



void one_instruction_cycle(CPU *cpu)
{
    // If the CPU isn't running, say so and return.
    // If the pc is out of range, complain and stop running the CPU.
    if((*cpu).running == 0)
    {
    printf("CPU has been Halted\n");
    return;
    }
    if((*cpu).pc >= MEMLEN)
    {
    printf("pc is out of range.\n");
    cpu->running = 0;
    return;
    }
    //16 bit hex instruction code.
    int instr_loc = cpu -> pc; // Instruction's location (pc before increment)
    cpu -> ir = cpu -> mem[cpu -> pc++];

    //getting the instruction
    unsigned short int instr = cpu->ir;

    //gets the a the bitstring for the instruction to execute in string format
    char *binaryNumber = getBitstringFromInstr(instr);

    //getting the opcode of the bitstring with a function I made.
    int opcodeInt = getSelectInt(binaryNumber, 0, 4, 0);

    //dothing the proper command based on the opcode
    if(opcodeInt == 0000) {
        branchHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt == 1) { //Instruction ADD
        addHanding(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt == 10) {
        ldHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt == 11) {
        stHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt == 100) {
        jsrHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt ==  101) {
        andHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt ==  110) {
        ldrHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt ==  111) {
        strHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt ==  1000) {
        printf("RTI - Unsupported Command");
        exec_HLT(cpu);
    } else if (opcodeInt ==  1001) {
        notHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt ==  1010) { //LDI
        stiHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt ==  1011) {    } else if (opcodeInt ==  1100) {
        jmpHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt ==  1110) {
        leaHandling(binaryNumber, cpu, instr_loc, instr);
    } else if (opcodeInt == 1111) {
        trapHandling(binaryNumber, cpu, instr_loc, instr);
    }
    else {
        printf("at x%04x: ir x%04x; Error, unknown opcode\n",instr_loc, instr);

    }
}

// Execute the halt instruction (make CPU stop running)
//
void exec_HLT(CPU *cpu)
{
    printf("HALT\nHalting\n");
    (cpu -> running) = 0;
}
