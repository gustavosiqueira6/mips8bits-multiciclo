simulador: main.c Back.c Controle.c ULA.c Reg.c PC.c bin.c Memoria.c Assembly.c Regs_interm.c Head.c
	gcc main.c Back.c Controle.c ULA.c Reg.c PC.c bin.c Memoria.c Assembly.c Regs_interm.c Head.c -o simulador

clean:
	rm -f simulador
