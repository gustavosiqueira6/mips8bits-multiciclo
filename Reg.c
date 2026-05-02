
#include <stdio.h>
#include <stdlib.h>
#include "Head.h"



void print_regs(signed char reg[8])
{

    printf("\n--- REGISTRADORES ---\n");

    for (int i = 0; i < 8; i++)
    {

        printf("  R%d = ", i);

        print_bin8((unsigned char)reg[i]);

        printf(" (%d)\n", reg[i]);

    }

    printf("---------------------\n");

}

int iniat(signed char reg[8])
{

    for (int i = 0; i < 8; i++)
    {

        reg[i] = 0;

    }

    return 0;

}

int read(signed char reg[8], signed char rs, signed char rt,signed char *outA, signed char *outB)
{

    *outA = reg[(unsigned char)rs];

    *outB = reg[(unsigned char)rt];

    return 0;

}


int Rdest(int Sinais[16], signed char rd, signed char rt)
{

    return Sinais[REG_DST] ? (int)rd : (int)rt;

}

int esc(signed char reg[8], int dest, signed char valor, int RegWrite)
{

    if (dest != 0 && RegWrite == 1)
    {

        reg[dest] = valor;

    }


    return 0;
}
