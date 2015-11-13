#include <stdio.h>
#include <stdlib.h> // For error exit()
#include <string.h>
#include <errno.h>

int power(int a, int b) {
    int i = 0;
    int total = 1;
    for(i = 0; i < b; i++) {
    total = total * a;
    }

    return total;
}
int invert2(n, l) {
    printf("%d\n", n);
    l = l - 1;
    int a;
    int i;
    int newint = 0;
    for(i = 0; i < l; i++) {
        newint *= 10;
        a = n%10;
        newint += a;
        n/=10;
        printf("%d %d\n",n, a);
        //if(n ==0) {i = l;}
    }
    return newint;
}
int convert(int n, int l) {
    int n_orig = n;
    int no_sign = n_orig%power(10, l - 1);

    int decimal=0, i=0, rem;
    while (n!=0){
        rem = n%10;
        n/=10;
        decimal += rem*power(2,i);
        ++i;
        //printf("%d %d\n", n, i);
        if(n == 1 && i == (l - 1)) {
            n = 0;
            decimal = ~decimal  - 1;
        }
    }

    return decimal;
}

int step1(int n) {
    char str[17];
    memset(str, 'x', 16);
    sprintf(str, "%d", n);
    printf("%s", str);
return n;
}



//This function takes the 7 chars starting with str[start]
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
            int i = 0;
            for(i = start; i < numb + start; i++) {
                printf("%c", str[i]);
            }
            printf("\nSign: %c\n", str[start]);


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

char* getBitstringFromInstr(unsigned short int instr) {
    //unsigned short int instr = 0x123f;

    //this is what the code below is going to convert into. It is set to default
    //as a 16 bit string full of zeros to act as a safety default.
    //char binaryNumber[] = "0000000000000000";
    //buf = (char*) malloc(17*sizeof(char));//create dynamic char sequence
    char* binaryNumber = (char*) malloc(17*sizeof(char));//create dynamic char sequence
    memset(binaryNumber, '0', 16);
    //assign the default value with String copy

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
            //printf("%d -- ", (4*i + n));
            binaryNumber[4*i + n] = newstring[n];
            //printf("%c\n", newstring[n]);
        }
    }

    //printf("don't get excited yet %s\n", binaryNumber);

    //strcpy(buf,binaryNumber);

    return binaryNumber;
}




int main(int argc, char *argv[]) {
    /*unsigned short int instr = 0x3F00;//(char) (*cpu).mem[(*cpu).pc];
    char hex[5];
    sprintf(hex, "%04x", instr);

    //char binaryNumber[] = "0000000110000000";
    char *binaryNumber = getBitstringFromInstr(instr);
    printf("binary integer: %s\n", binaryNumber);

    //getting the opcode of the bitstring with a function I made.
    int opcodeIntOld = getSelectInt(binaryNumber, 0, 4, 1);

    printf("fake %d\n\n\n", opcodeIntOld);

    char inputChar = getchar();*/

    /*int a = 0xf0f0;
    a = ~a;
    int mask = 0xffff;
    a = a&mask;*/
        int nBit = 1;
        int zBit = 1;
        int pBit = 1;

        int i = 2;
        char Mnemonic[] = "BR\0\0\0\0";
        if(nBit != 0) {Mnemonic[i] = 'N'; i++;}
        if(zBit != 0) {Mnemonic[i] = 'Z'; i++;}
        if(pBit != 0) {Mnemonic[i] = 'P'; i++;}

    printf("%s", Mnemonic);

return 0;
}
