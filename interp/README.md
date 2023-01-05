# 인터프리터 컴파일
	gcc -g y.tab.c lex.yy.c interp.c lib.c -o interp.out

#  어셈블리어 인터프리트
	./interp.out ../a.asm
