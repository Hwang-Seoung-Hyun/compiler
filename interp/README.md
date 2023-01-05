# compile interpreter
	gcc -g y.tab.c lex.yy.c interp.c lib.c -o interp.out

# interprete asm
	./interp.out ../a.asm
