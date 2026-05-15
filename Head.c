#ifndef HEAD_H
#define HEAD_H

/* ==========================================================================
   HEAD.H � Mini MIPS 8 bits Multiciclo
   ========================================================================== */

/* --------------------------------------------------------------------------
   Struct de mem�ria unificada
   [0-127]   = instru��es (16 bits cada posi��o)
   [128-255] = dados      (8 bits usados, armazenados em unsigned short)
   -------------------------------------------------------------------------- */
typedef struct {
    unsigned short instc[256];
    int n;   /* n�mero de instru��es carregadas */
} instro;

/* --------------------------------------------------------------------------
   �ndices dos sinais de controle no vetor Sinais[16]
   -------------------------------------------------------------------------- */
typedef enum {
    REG_DST      = 0,   /* 1: destino = rd (tipo R); 0: destino = rt (tipo I)  */
    JUMP         = 1,   /* 1: instru��o � jump (n�o mais usado diretamente)     */
    MEM_READ     = 2,   /* 1: habilita leitura da mem�ria                      */
    MEM_WRITE    = 3,   /* 1: habilita escrita na mem�ria                      */
    BRANCH       = 4,   /* 1: instru��o � beq                                  */
    ALU_SRC      = 5,   /* 1: 2� entrada da ULA = imediato; 0: = regB          */
    MEM_TO_REG   = 6,   /* 1: dado escrito no reg vem da mem�ria; 0: da ULA    */
    REG_WRITE    = 7,   /* 1: habilita escrita no banco de registradores        */
    IouD         = 8,   /* 1: endere�o da mem = ULASaida; 0: = PC              */
    IR_ESC       = 9,   /* 1: habilita escrita no Registrador de Instru��o      */
    PC_ESC       = 10,  /* 1: habilita escrita no PC                           */
    PC_FONTE0    = 11,  /* bit 0 da fonte do PC                                */
    PC_FONTE1    = 12,  /* bit 1 da fonte do PC                                */
    ULA_FONTE_A  = 13,  /* 1: entrada A da ULA = regA; 0: = PC                 */
    ULA_FONTE_B0 = 14,  /* bit 0 da fonte B da ULA                             */
    ULA_FONTE_B1 = 15   /* bit 1 da fonte B da ULA                             */
                        /* ULAFonteB: 00=regB 01=const1 10=imm_ext 11=reservado*/
} SinalControle;

/* --------------------------------------------------------------------------
   Snapshot � estado completo do simulador em um ciclo de clock
   Usado pelo back (desfazer um ciclo)
   -------------------------------------------------------------------------- */
typedef struct {
    signed char    reg[8];
    int            Sinais[16];
    int            ULAop;
    int            overflow;
    signed char    ULASaida;
    signed char    RDM;
    signed char    regA;
    signed char    regB;
    unsigned short RI;
    unsigned char  PC;
    int            estado;
    int            n_ciclo;
    int            n_instr;
} Snapshot;

/* ==========================================================================
   ASSINATURAS DAS FUN��ES
   ========================================================================== */

/* --- banco_registradores.c --- */
void        print_regs(signed char reg[8]);
int         iniat(signed char reg[8]);
int         read(signed char reg[8], signed char rs, signed char rt,
                 signed char *outA, signed char *outB);
int         Rdest(int Sinais[16], signed char rd, signed char rt);
int         esc(signed char reg[8], int dest, signed char valor, int RegWrite);

/* --- ula.c --- */
int         ulamx(int Sinais[16], signed char A, signed char B, signed char immx);
int         ula(int ULAop, signed char A, signed char B,
                int *overflow, int *zero);
void        Estender(unsigned char imm, signed char *immx);
int         wrmux(int Sinais[16], int result);
void        tipo(int ULAop);
void        Tipo2(unsigned char opcode, int *ULAop);

/* --- controle.c --- */
void        Decodifica_estado(int estado, unsigned char opcode,
                              unsigned char funct, int Sinais[16]);
int         proximo_estado(int estado_atual, unsigned char opcode);

/* --- memoria.c (substitui memoria_inst.c + memoria_dados.c) --- */
int         carregar_unificado(instro *mem, const char *nome_arquivo);
unsigned short ler_unificada(instro *mem, unsigned char endereco);
void        print_mem_unificada(instro *mem);
int         Store(instro *mem, signed char endereco, signed char valor);
void        save_mem_dat(instro *mem, const char *nome_arquivo);

/* --- assembly.c --- */
void        print_asm(unsigned short instr);
void        print_program(instro *mem, int tamanho);
void        save_program_asm(instro *mem, int tamanho, const char *nome_arquivo);

/* --- pc.c --- */
unsigned char busca(unsigned char PC);
unsigned char jump(unsigned char addr);
unsigned char branch(unsigned char PC, unsigned char imm);

/* --- back.c --- */
void        push_multi(Snapshot *pilha, int *sp,
                       signed char reg[], int Sinais[16],
                       int ULAop, int overflow,
                       signed char ULASaida, signed char RDM,
                       signed char regA, signed char regB,
                       unsigned short RI, unsigned char PC,
                       int estado, int n_ciclo, int n_instr, instro *mem);

void        pop_multi(Snapshot *pilha, int *sp,
                      signed char reg[], int Sinais[16],
                      int *ULAop, int *overflow,
                      signed char *ULASaida, signed char *RDM,
                      signed char *regA, signed char *regB,
                      unsigned short *RI, unsigned char *PC,
                      int *estado, int *n_ciclo, int *n_instr, instro *mem);

/* --- main.c (helpers internos expostos para impress�o) --- */
void        print_bin(unsigned short x);
void        print_bin8(unsigned char x);

#endif /* HEAD_H */
