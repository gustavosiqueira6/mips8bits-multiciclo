#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Head.h"

#ifdef _WIN32
    #include <windows.h>
    #include <commdlg.h>
#endif


void selecionar_arquivo(char *caminho)
{
#ifdef _WIN32
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile   = caminho;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile    = 260;
    ofn.lpstrFilter = "Arquivos MEM\0*.mem\0Todos\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn))
        printf("Arquivo selecionado: %s\n", caminho);
#else
    printf("Digite o caminho do arquivo (.mem): ");
    scanf("%s", caminho);
    strcat(caminho, ".mem");
#endif
}


static void print_estado(int estado)
{
    const char *nomes[] = {

        "0 - Busca (fetch)",
        "1 - Decodificacao + leitura Rs/Rt",
        "2 - Calculo de endereco (lw/sw/addi)",
        "3 - Acesso a memoria: leitura (lw)",
        "4 - Escrita no registrador Rt (lw)",
        "5 - Acesso a memoria: escrita (sw)",
        "6 - Escrita no registrador Rt (addi)",
        "7 - Execucao tipo R",
        "8 - Escrita no registrador Rd (tipo R)",
        "9 - Termino beq",
        "10 - Termino jump"
    };

    if (estado >= 0 && estado <= 10)
    {

        printf("\n[ESTADO %s]\n", nomes[estado]);

    }

    else
    {

        printf("\n[ESTADO INVALIDO: %d]\n", estado);

    }

}

int executa_ciclo(instro*mem,signed char reg[8],int Sinais[16],int *estado,unsigned char *PC,unsigned short *RI,signed char *RDM,signed char *regA,signed char *regB,signed char *ULASaida,int *ULAop,int *overflow,int *zero,int *n_ciclo)
{

    (*n_ciclo)++;

    unsigned char opcode = (*RI >> 12) & 0xF;

    unsigned char rs     = (*RI >> 9)  & 0x7;

    unsigned char rt     = (*RI >> 6)  & 0x7;

    unsigned char rd     = (*RI >> 3)  & 0x7;

    unsigned char imm6   = *RI & 0x3F;

    unsigned char funct  = *RI & 0x7;

    unsigned char addr   = *RI & 0xFF;

    signed char immx = 0;

    Estender(imm6, &immx);

    print_estado(*estado);

    Decodifica_estado(*estado, opcode, funct, Sinais);

    int instrucao_concluida = 0;

    switch (*estado)
    {

        case 0:

            *RI = ler_unificada(mem, *PC);

            printf("  RI = Mem[%d] = ", *PC);

            print_bin(*RI)
            ;
            printf(" (0x%04X)\n", *RI);

            *PC = busca(*PC);

            printf("  PC atualizado para %d\n", *PC);

            *estado = 1;

            break;


        case 1:

            opcode = (*RI >> 12) & 0xF;

            rs     = (*RI >> 9)  & 0x7;

            rt     = (*RI >> 6)  & 0x7;

            rd     = (*RI >> 3)  & 0x7;

            imm6   = *RI & 0x3F;

            funct  = *RI & 0x7;

            addr   = *RI & 0xFF;

            Estender(imm6, &immx);

            read(reg, rs, rt, regA, regB);

            printf("  A = Reg[%d] = %d  |  B = Reg[%d] = %d\n", rs, *regA, rt, *regB);


            *ULASaida = (signed char)(*PC) + immx;

            printf("  ULASaida (end. branch) = %d + %d = %d\n", *PC, immx, *ULASaida);

            *estado = proximo_estado(1, opcode);

            printf("  Opcode 0x%X -> proximo estado: %d\n", opcode, *estado);

            break;

        case 2:

            *ULASaida = (signed char)ula(0, *regA, immx, overflow, zero);

            printf("  ULASaida = %d + %d = %d\n", *regA, immx, *ULASaida);

            *estado = proximo_estado(2, opcode);

            break;


        case 3:

            *RDM = (signed char)ler_unificada(mem, (unsigned char)*ULASaida+128);

            printf("  RDM = Mem[%d] = %d\n", (unsigned char)*ULASaida, *RDM);

            *estado = 4;

            break;


        case 4:

            esc(reg, rt, *RDM, 1);

            printf("  Reg[%d] <- %d (lw concluido)\n", rt, *RDM);

            *estado = 0;

            instrucao_concluida = 1;

            break;


        case 5:

            Store(mem, *ULASaida, *regB);

            printf("  Mem[%d] <- %d (sw concluido)\n", (unsigned char)*ULASaida, *regB);

            *estado = 0;

            instrucao_concluida = 1;

            break;


        case 6:

            esc(reg, rt, *ULASaida, 1);

            printf("  Reg[%d] <- %d (addi concluido)\n", rt, *ULASaida);

            *estado = 0;

            instrucao_concluida = 1;

            break;


        case 7:

            *ULAop = funct;

            tipo(funct);

            *ULASaida = (signed char)ula(*ULAop, *regA, *regB, overflow, zero);

            printf("  ULASaida = %d op %d = %d (funct=%d)\n",*regA, *regB, *ULASaida, funct);

            *estado = 8;

            break;


        case 8:

            esc(reg, rd, *ULASaida, 1);

            printf("  Reg[%d] <- %d (tipo R concluido)\n", rd, *ULASaida);

            if (*overflow)
            {

                printf("  ** OVERFLOW **\n");

            }


            *estado = 0;

            instrucao_concluida = 1;

            break;

        case 9:

            ula(2 , *regA, *regB, overflow, zero);

            if (*zero)
            {

                *PC = (unsigned char)*ULASaida;

                printf("  beq: branch tomado -> PC = %d\n", *PC);

            }

            else
            {

                printf("  beq: branch NAO tomado, PC = %d\n", *PC);

            }

            *estado = 0;

            instrucao_concluida = 1;

            break;


        case 10:

            *PC = jump(addr);

            printf("  jump -> PC = %d\n", *PC);

            *estado = 0;

            instrucao_concluida = 1;

            break;

        default:

            printf("  ESTADO INVALIDO %d — resetando para 0\n", *estado);

            *estado = 0;

            break;

    }

    return instrucao_concluida;
}


int main(void)
{

    instro mem_unificada = {0};

    signed char reg[8];

    iniat(reg);

    int Sinais[16] = {0};

    unsigned short RI       = 0;

    signed char    RDM      = 0;

    signed char    regA     = 0;

    signed char    regB     = 0;

    signed char    ULASaida = 0;


    int estado = 0;


    int ULAop    = 0;

    int overflow = 0;

    int zero     = 0;

    int n_ciclo  = 0;

    int n_instr  = 0;


    unsigned char PC = 0;


    Snapshot pilha[500];

    int sp = -1;

    char caminho[260];

    int op = 99;

    do {

        printf("\n");
        printf(" 1  - Carregar memoria unificada (.mem)\n");
        printf(" 2  - sem uso ainda...)\n");
        printf(" 3  - Imprimir memoria(s) (instrucoes e dados)\n");
        printf(" 4  - Imprimir banco de registradores\n");
        printf(" 5  - Imprimir simulador\n");
        printf(" 6  - Salvar programa (.asm)\n");
        printf(" 8  - Executar programa(run)\n");
        printf(" 9  - Executar ciclo de clock (step)\n");
        printf(" 10 - Voltar um ciclo (back)\n");
        printf(" 0  - Sair\n");
        printf("Sua escolha: ");
        scanf("%d", &op);

        switch (op)
        {

            case 1:

                caminho[0] = '\0';

                selecionar_arquivo(caminho);

                if (carregar_unificado(&mem_unificada, caminho) != 0)
                {

                    printf("\nErro ao carregar memoria unificada.\n");

                }


                else
                {

                    printf("\nMemoria unificada carregada com sucesso.\n");

                    PC      = 0;

                    estado  = 0;

                    n_ciclo = 0;

                    n_instr = 0;

                    sp = -1;

                    iniat(reg);
                }

                break;


            case 3:

                print_mem_unificada(&mem_unificada);

                break;


            case 4:

                print_regs(reg);

                break;

            case 5:

                printf("\n=== ESTADO DO SIMULADOR ===\n");

                printf("PC: %d (", PC);

                print_bin8(PC);

                printf(")\n");

                printf("Estado FSM: %d\n", estado);

                printf("Clocks: %d\n", n_ciclo);

                printf("Instrucoes concluidas: %d\n", n_instr);

                printf("RI: "); print_bin(RI); printf(" (0x%04X)\n", RI);

                printf("RDM: %d\n", RDM);

                printf("RegA: %d\n", regA);

                printf("RegB: %d\n", regB);

                printf("ULASaida: %d\n", ULASaida);

                print_regs(reg);

                print_mem_unificada(&mem_unificada);

                break;


            case 6:

                print_program(&mem_unificada, 128);

                save_program_asm(&mem_unificada, 128, "programa.asm");

                break;


            case 8:

                printf("\nExecutando programa...\n");

                PC      = 0;

                estado  = 0;

                n_ciclo = 0;

                n_instr = 0;

                sp = -1;

                iniat(reg);


                while (PC < 128)
                {

                    push_multi(pilha, &sp, reg, Sinais, ULAop, overflow,(signed char)ULASaida, RDM, regA, regB,RI, PC, estado, n_ciclo, n_instr);

                    int concluiu = executa_ciclo(&mem_unificada, reg, Sinais,&estado, &PC,&RI, &RDM, &regA, &regB, &ULASaida,&ULAop, &overflow, &zero, &n_ciclo);

                    if (concluiu)
                    {
                        n_instr++;
                        printf("\n--- Instrucao %d concluida (ciclo %d) ---\n",
                               n_instr, n_ciclo);
                        printf("Registradores: ");
                        for (int i = 0; i < 8; i++)
                            printf("$R%d=%d ", i, reg[i]);
                        printf("\n");

                    }


                    if (n_ciclo > 10000)
                    {

                        printf("\n[AVISO] Limite de 10000 ciclos atingido.\n");

                        break;

                    }

                }

                printf("\n=== FIM DO PROGRAMA ===\n");

                printf("Total de ciclos de clock: %d\n", n_ciclo);

                printf("Total de instrucoes: %d\n", n_instr);

                printf("Estado final do PC: %d\n", PC);

                break;


            case 9:

                //tira snapshoot
                push_multi(pilha, &sp, reg, Sinais, ULAop, overflow,(signed char)ULASaida, RDM, regA, regB,RI, PC, estado, n_ciclo, n_instr);

                printf("\n=== STEP: ciclo %d | PC=%d | estado=%d ===\n",n_ciclo + 1, PC, estado);

                {
                    int concluiu = executa_ciclo(&mem_unificada, reg, Sinais,&estado, &PC,&RI, &RDM, &regA, &regB, &ULASaida,&ULAop, &overflow, &zero, &n_ciclo);

                    if (concluiu)
                    {

                        n_instr++;

                        printf("\n  -> Instrucao %d CONCLUIDA neste ciclo.\n", n_instr);

                    }

                    else
                    {

                        printf("\n  -> Proximo estado: %d\n", estado);

                    }

                }

                printf("\nRegistradores apos o ciclo:\n");

                for (int i = 0; i < 8; i++)
                {

                    printf("  $R%d = %d\n", i, reg[i]);

                }


                printf("PC = %d | Estado = %d\n", PC, estado);

                break;

            case 10:

                pop_multi(pilha, &sp, reg, Sinais, &ULAop, &overflow,&ULASaida, &RDM, &regA, &regB,&RI, &PC, &estado, &n_ciclo, &n_instr);

                printf("  Voltou para: ciclo=%d | PC=%d | estado=%d\n",n_ciclo, PC, estado);

                break;

            case 0:

                printf("\n saindo da aplicação \n");

                break;

            default:

                printf("\n opção invalida porfavor escolha uma das opções fornecidas pelo menu\n");

                break;

        }

    } while (op != 0);


    return 0;

}
