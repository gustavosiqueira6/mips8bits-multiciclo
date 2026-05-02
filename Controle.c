#include <stdio.h>
#include <stdlib.h>
#include "Head.h"

static void limpa_sinais(int Sinais[16])
{

    for (int i = 0; i < 16; i++)
    {

        Sinais[i] = 0;

    }

}


void Decodifica_estado(int estado, unsigned char opcode, unsigned char funct, int Sinais[16])
{

    limpa_sinais(Sinais);

    switch (estado)
    {


        case 0:

            Sinais[PC_ESC] = 1;

            Sinais[IouD] = 0;

            Sinais[IR_ESC] = 1;

            Sinais[ULA_FONTE_A] = 0;

            Sinais[ULA_FONTE_B0]= 1;

            Sinais[ULA_FONTE_B1]= 0;

            Sinais[PC_FONTE0] = 0;

            Sinais[PC_FONTE1] = 0;

            Sinais[REG_DST] = 1;

            printf("  Sinais: PCEsc=1 IouD=0 IRESc=1 ULAFonteB=01 ULAFonteA=0 (soma)\n");

            break;


        case 1:

            Sinais[ULA_FONTE_A] = 0;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            Sinais[REG_DST]     = 1;

            printf("  Sinais: ULAFonteA=0 ULAFonteB=10 (soma para branch)\n");

            break;


        case 2:

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            Sinais[REG_DST]     = 0;

            printf("  Sinais: ULAFonteA=1 ULAFonteB=10 (soma para endereco)\n");

            break;

        case 3:

            Sinais[IouD] = 1;

            Sinais[MEM_READ] = 1;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            printf("  Sinais: IouD=1 MEM_READ=1\n");

            break;

        case 4:

            Sinais[REG_WRITE] = 1;

            Sinais[MEM_TO_REG] = 1;

            Sinais[REG_DST] = 0;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            printf("  Sinais: EscReg=1 MemParaReg=1 RegDst=0 (lw concluido)\n");

            break;

        case 5:

            Sinais[IouD] = 1;

            Sinais[MEM_WRITE] = 1;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            printf("  Sinais: IouD=1 EscMem=1 (sw concluido)\n");

            break;

        case 6:

            Sinais[REG_WRITE] = 1;

            Sinais[MEM_TO_REG]= 0;

            Sinais[REG_DST]= 0;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            printf("  Sinais: EscReg=1 MemParaReg=0 RegDst=0 (addi concluido)\n");

            break;


        case 7:

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 0;

            Sinais[REG_DST]= 1;

            printf("  Sinais: ULAFonteA=1 ULAFonteB=00 ULAop=funct(%d)\n", funct);

            break;


        case 8:

            Sinais[REG_WRITE] = 1;

            Sinais[MEM_TO_REG] = 0;

            Sinais[REG_DST]= 1;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 0;

            printf("  Sinais: EscReg=1 MemParaReg=0 RegDst=1 (tipo R concluido)\n");

            break;


        case 9:

            Sinais[BRANCH] = 1;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 0;

            Sinais[PC_FONTE0] = 1;

            Sinais[PC_FONTE1] = 0;


            printf("  Sinais: Branch=1 ULAFonteA=1 ULAFonteB=00 FontePC=01 (sub)\n");

            break;


        case 10:

            Sinais[PC_ESC]= 1;

            Sinais[PC_FONTE0] = 0;

            Sinais[PC_FONTE1]= 1;

            Sinais[REG_DST]= 1;

            printf("  Sinais: PCEsc=1 FontePC=10 (jump)\n");

            break;

        default:

            printf("  [CONTROLE] Estado invalido: %d\n", estado);

            break;

    }
}

int proximo_estado(int estado_atual, unsigned char opcode)
{

    switch (estado_atual)
    {

        case 0:

            return 1;

        case 1:

            if (opcode == 0xB || opcode == 0xF || opcode == 0x4)
            {

                return 2;

            }

            else if (opcode == 0x0)
            {

                return 7;

            }

            else if (opcode == 0x8)
            {

                return 9;

            }

            else if (opcode == 0x2)
            {

                return 10;

            }


            else
            {

                printf(" Opcode desconhecido 0x%X no estado 1\n", opcode);

                return 0;
            }

        case 2:

            if (opcode == 0xB)
            {

                return 3;

            }

            else if (opcode == 0xF)
            {

                return 5;

            }


            else if (opcode == 0x4)
            {

                return 6;

            }

            else
            {
                printf(" Opcode inesperado 0x%X no estado 2\n", opcode);

                return 0;

            }

        case 3:

            return 4;

        case 4:

            return 0;

        case 5:

            return 0;

        case 6:

            return 0;

        case 7:

            return 8;

        case 8:

            return 0;

        case 9:

            return 0;

        case 10:

             return 0;

        default:

            printf("  [CONTROLE] Estado invalido %d em proximo_estado()\n", estado_atual);

            return 0;

    }

}
