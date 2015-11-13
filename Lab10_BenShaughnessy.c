//Ben Shaughnessy
// CS 350, Fall 2014
// Lab 7: SDC Simulator
//
// Illinois Institute of Technology, (c) 2014, James Sasaki
#include <stdio.h>
#include <stdlib.h> // For error exit()
// CPU Declarations -- a CPU is a structure with fields for the
// different parts of the CPU.
//
typedef short int Word; // type that represents a word of SDC memory
typedef unsigned short int Address; // type that represents an SDC address

#define MEMLEN 65536
#define NREG 8

typedef struct
{
    Word mem[MEMLEN];
    Word reg[NREG]; // Note: "register" is a reserved word
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
// Main program: Initialize the cpu, read initial memory values,
// and execute the read-in program starting at location 00.
//
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
    printf("pc : x%04x IR : x%04x cc : %x RUNNING : %d", (*cpu).pc, (*cpu).ir, (*cpu).cc, (*cpu).running);
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
            printf("x%04x: x%04x %x\n",i,(*cpu).mem[i], (*cpu).mem[i]);
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
    printf("R%d x%04x %x ", i, (*cpu).reg[i], (*cpu).reg[i]);
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
        if (location >= MEMLEN || location < 0){
            printf("invalid location in memory\n");
        }
        else if (value >= MEMLEN || location < 0){
            printf("invalid value to store in memory\n");
        }
        else {
            //(*cpu).mem[location] = value;
            printf("set mem[x%04hx] to x%04hx\n", location, value);
        }
    }
    else if(cmd_char == 'r') {
        if (location <= 0 || location > 7){
            printf("invalid register value\n");
        }
        else if (value >= MEMLEN || location < 0){
            printf("invalid value to store in memory\n");
        }
        else {
            //(*cpu).mem[location] = value;
            printf("setting r%d to x%04hx\n", location, value);
        }
    }
    else if(cmd_char == 'j') {
        if(location >= MEMLEN || location < 0) {
            printf("invalid memory locations\n");
        } else {
            (*cpu).pc = location;
            printf("jumped to x%04x\n", (*cpu).pc);
        }
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
void one_instruction_cycle(CPU *cpu)
{
    /*
        Word mem[MEMLEN];
        Word reg[NREG]; // Note: "register" is a reserved word
        Address pc;
        int cc; // Program Counter
        int running; // running = 1 iff CPU is executing instructions
        Word ir; // Instruction Register
        int instr_sign;
        int opcode;
        int reg_R;
        int addr_MM;
    */


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

    int instr = (*cpu).mem[(*cpu).pc];
    printf("%d", instr)

    // Get instruction and increment pc
    //
    /*int instr_loc = cpu -> pc; // Instruction's location (pc before increment)
    cpu -> ir = cpu -> mem[cpu -> pc++];
    // Decode instruction into opcode, reg_R, addr_MM, and instruction sign
    // *** STUB ****
    int opcode,reg_R,addr_MM,instr_sign;
    int posir = abs((*cpu).ir);
    int temp;
    opcode = posir / 1000;
    reg_R = (posir /100) % 10;
    addr_MM = posir % 100;
    if((*cpu).ir > 0) instr_sign = 1;
    else	instr_sign = -1;

    // Echo instruction
    //printf("opcode: %s, reg_R:%s, addr_MM: %s, instr_sign:%s, posir: %s, instr_loc: %s\n", opcode, reg_R, addr_MM, instr_sign, posir, instr_loc);
    printf("executing instruction  at location: x%04x with value: x%04x   command not yet supported\n", (*cpu).pc, (*cpu).mem[(*cpu).pc]);
    */
}
// Execute the halt instruction (make CPU stop running)
//
void exec_HLT(CPU *cpu)
{
    printf("HALT\nHalting\n");
    (cpu -> running) = 0;
}
