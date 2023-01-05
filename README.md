# 컴파일러 컴파일

	yacc -d yacc.y
	flex lex.l
	gcc -g yacc.tab.c lex.yy.c syntex_analizer.c semantic_analizer.c generator.c print.c sem_print.c -o compilerHW8.out -lfl

# 컴파일러 실행

	./compiler8.out test3.c
