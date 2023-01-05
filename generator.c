#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include"yacc.tab.h"
#include"type.h"
#define LIT_MAX 8092

void gen_program(const A_NODE* const);
void gen_literal_table();
void gen_declaration_list(const A_ID* const);
void gen_declaration(const A_ID* const);
void gen_expression(const A_NODE* const);
void gen_expression_left(const A_NODE* const);
int gen_arg_expression(const A_NODE* const);
void gen_statement(const A_NODE* const,const int,const int);
void gen_statement_list(const A_NODE* const,const int,const int);

void gen_code_i(OPCODE, const int, const int);
void gen_code_f(OPCODE, const int, const float);
void gen_code_s(OPCODE, const int, const char* const);
void gen_code_l(OPCODE, const int, const int);
int get_label();
void gen_label_number(const int);
void gen_label_name(const char* const);
void gen_error(const int, const int, const char* const);

char* opcode_name[] =
{
	"OP_NULL", "LOD", "LDX", "LDXB", "LDA", "LITI",
	"STO", "STOB", "STX", "STXB",
	"SUBI", "SUBF", "DIVI", "DIVF", "ADDI", "ADDF", "OFFSET", "MULI", "MULF", "MOD",
	"LSSI", "LSSF", "GTRI", "GTRF", "LEQI", "LEQF", "GEQI", "GEQF", "NEQI", "NEQF", "EQLI", "EQLF",
	"NOT", "OR", "AND", "CVTI", "CVTF",
	"JPC", "JPCR", "JMP", "JPT", "JPTR", "INT", "INCI", "INCF", "DECI", "DECF", "SUP", "CAL", "ADDR",
	"RET", "MINUSI", "MINUSF", "LDI", "LDIB", "POP"
};
int label_no = 0;
int gen_err=0;
FILE* fout;

//extern function declaration
int syntex_analize(A_NODE* const);
int semantic_analysis();
A_NODE* makeNode(NODE_NAME, A_NODE* const, A_NODE* const, A_NODE* const);
A_TYPE* makeType(T_KIND);
A_TYPE* setTypeElementType(A_TYPE* const, A_TYPE* const);
bool isArrayType(const A_TYPE* const);
bool isFloatType(const A_TYPE* const);
bool isPointerOrArrayType(const A_TYPE* const);
bool isPointerType(const A_TYPE* const);
extern A_TYPE* char_type, *int_type, *void_type, *string_type, *float_type;
extern A_NODE* root;
extern A_LITERAL literal_table[LIT_MAX];
extern int literal_no;
void generateCode();
int main()
{
	if ((fout = fopen("a.asm", "w")) == NULL)
    	{
        	printf("file open error : \"a.asm\"\n");
		fclose(fout);
        	exit(-1);
    	}
	int correctSyntex = syntex_analize(root);
       
	if (correctSyntex == -1)
        {
           return 0;
        }
	printf("\nlet's sementic_analize!\n");   
	int correctSemantic = semantic_analysis();
	if ( correctSemantic == -1)
	{
		return 0;
	}
	generateCode();	
	return 0;
}

void generateCode()
{
	printf("\nlet's generate code!\n");
	gen_program(root);
	gen_literal_table();
}


/// <summary>
/// program 시작시 asm을 만든다.
/// 1. grobal activation record 생성
/// 2. main() 함수 호출
/// 3. 리턴
/// </summary>
/// <param name="node"></param>
void gen_program(const A_NODE* const node)
{
	switch (node->name)
	{
	case N_PROGRAM:
		gen_code_i(INT, 0, node->value);
		gen_code_s(SUP, 0, "main");
		gen_code_i(RET, 0, 0);
		gen_declaration_list((A_ID*)node->clink);
		break;
	default:
		gen_error(100, node->line, NULL);

	}

}
void gen_literal_table()
{
	for(int i=1;i<=literal_no;i++)
	{
		fprintf(fout,".literal %5d ",literal_table[i].addr);
		if(literal_table[i].type==int_type)
		{
			fprintf(fout,"%d \n",literal_table[i].value.i);
		}
		else if(literal_table[i].type==float_type)
                {
                        fprintf(fout,"%f \n",literal_table[i].value.f);
                }
		else if(literal_table[i].type==char_type)
                {
                        fprintf(fout,"%d \n",literal_table[i].value.c);
                }
		else if(literal_table[i].type==string_type)
                {
                        fprintf(fout,"%s \n",literal_table[i].value.s);
                }
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="id"></param>
void gen_declaration_list(const A_ID* const id_null)
{
	if (id_null)
	{
		gen_declaration(id_null);
		gen_declaration_list(id_null->link);
	}
	return;
}

/// <summary>
/// 
/// </summary>
/// <param name="id"></param>
void gen_declaration(const A_ID* const id)
{
	switch (id->kind)
	{
	case ID_FUNC:
		if(id->type->expr)
		{
			gen_label_name(id->name);
			gen_code_i(INT, 0, id->type->local_var_size);
			gen_statement(id->type->expr, 0, 0);
			gen_code_i(RET, 0, 0);
		}
		break;
	}
}

/// <summary>
/// expression의 asm을 만든다. 단 치환연산자의 좌항은 만들지 않는다.
/// </summary>
/// <param name="node"></param>
void gen_expression(const A_NODE* const node)
{
	A_ID* id;
	A_TYPE* type;
	int i;
	switch (node->name)
	{
	case  N_EXP_IDENT:
		id = (A_ID*)node->clink;
		type = id->type;
		switch (id->kind)
		{
		case ID_VAR:
		case ID_PARM:
			switch (type->kind)
			{
			case T_ENUM:
			case T_POINTER:
				gen_code_i(LOD, id->level, id->address);
				break;
			case T_ARRAY:
				if (id->kind == ID_VAR)
				{
					gen_code_i(LDA, id->level, id->address);
				}
				else
				{
					gen_code_i(LOD, id->level, id->address);
				}
				break;
			default:
				gen_error(11,id->line,id->name);
				break;
			}
			break;
		case ID_ENUM_LITERAL:
			gen_code_i(LITI, 0, *((int*)id->init));
			break;
		case ID_FUNC:
			gen_code_s(ADDR,0,id->name);
			break;
		default:
			gen_error(11,node->line,NULL);
			break;
		}
		break;
	case N_EXP_CHAR_CONST:
		gen_code_i(LITI, 0, *((char*)node->clink));
		break;
	case N_EXP_INT_CONST:
		gen_code_i(LITI, 0, *((int*)node->clink));
		break;
	case N_EXP_FLOAT_CONST:
		i = *((int*)node->clink);
		if(literal_table[i].type!=node->type)
		{
			if(literal_table[i].type==float_type)
			{
				int temp=(int)literal_table[i].value.f;
				literal_table[i].value.i=temp;
			}
			else 
			{
				float temp=(float)literal_table[i].value.i;
                                literal_table[i].value.f=temp;
			}


			literal_table[i].type=node->type;
		}
		gen_code_i(LOD, 0, literal_table[i].addr);
		
		break;
	case N_EXP_STRING_LITERAL:
		i = *((int*)node->clink);
		gen_code_i(LDA, 0, literal_table[i].addr);
		break;
		////////////////////////
		///postfix_expression///
		////////////////////////
	case N_EXP_ARRAY:
		// expr1 [ expr2 ]
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (node->type->size > 1)
		{
			gen_code_i(LITI, 0, node->type->size);
			gen_code_i(MULI, 0, 0);
		}
		gen_code_i(OFFSET, 0, 0);
		if (!isArrayType(node->type))
		{
			i = node->type->size;
			if (i == 1)
			{
				gen_code_i(LDIB, 0, 0);
			}
			else
			{
				gen_code_i(LDI, 0, 0);
			}
		}
		break;
	case N_EXP_STRUCT:
		//expr1 . id
		gen_expression_left(node->llink);
		id = (A_ID*)node->rlink;
		if (id->address > 0)
		{
			gen_code_i(LITI, 0, id->address);
			gen_code_i(OFFSET, 0, 0);
		}
		if (!isArrayType(node->type))
		{
			i = node->type->size;
			if (i == 1)
			{
				gen_code_i(LDIB, 0, 0);
			}
			else
			{
				gen_code_i(LDI, 0, 0);
			}
		}
		break;
	case N_EXP_ARROW:
		//expr1 -> id
		gen_expression(node->llink);
		id = (A_ID*)node->rlink; 
		if (id->address > 0)
		{
			gen_code_i(LITI, 0, id->address); gen_code_i(OFFSET, 0, 0);
		}
		if (!isArrayType(node->type))
		{
			i = node->type->size;
			if (i == 1)
			{
				gen_code_i(LDIB, 0, 0);
			}
			else
			{
				gen_code_i(LDI, 0, 0);
			}
		}
		break;
	case N_EXP_FUNCTION_CALL:
		//expr1 ( arg_list_opt )
		type = node->llink->type;
		i = type->element_type->element_type->size;
		if (i % 4)
		{
			i = i / 4 * 4 + 4;
		}
		if (node->rlink)
		{
			gen_code_i(INT, 0, 12 + i);
			gen_arg_expression(node->rlink);
			gen_code_i(POP, 0, node->rlink->value / 4 + 3);
		}

		else gen_code_i(INT, 0, i);
		gen_expression(node->llink);
		gen_code_i(CAL, 0, 0);
		break;
	case N_EXP_POST_INC:
		gen_expression(node->clink);
		gen_expression_left(node->clink);
		type = node->type;
		if (node->type->size == 1)
		{
			gen_code_i(LDXB, 0, 0);
		}
		else
		{
			gen_code_i(LDX, 0, 0);
		}
		if (isPointerOrArrayType(node->type))
		{
			gen_code_i(LITI, 0, node->type->element_type->size);
			gen_code_i(ADDI, 0, 0);
		}
		else if (isFloatType(node->type))
		{
			gen_code_i(INCF, 0, 0);
		}
		else
		{
			gen_code_i(INCI, 0, 0);
		}
		if (node->type->size == 1)
		{
			gen_code_i(STOB, 0, 0);
		}
		else
		{
			gen_code_i(STO, 0, 0);
		}
		break;
	case N_EXP_POST_DEC:
		gen_expression(node->clink);
		gen_expression_left(node->clink);
		type = node->type;
		if (node->type->size == 1)
		{
			gen_code_i(LDXB, 0, 0);
		}
		else
		{
			gen_code_i(LDX, 0, 0);
		}
		if (isPointerOrArrayType(node->type))
		{
			gen_code_i(LITI, 0, node->type->element_type->size);
			gen_code_i(SUBI, 0, 0);
		}
		else if (isFloatType(node->type))
		{
			gen_code_i(DECF, 0, 0);
		}
		else
		{
			gen_code_i(DECI, 0, 0);
		}
		if (node->type->size == 1)
		{
			gen_code_i(STOB, 0, 0);
		}
		else
		{
			gen_code_i(STO, 0, 0);
		}
		break;
		//////////////////////
		///unary expression///
		/// //////////////////
	case N_EXP_CAST:
		//( typename ) expr1
		gen_expression(node->rlink);

		if(node->type!=node->rlink->type)
		{
			if(isFloatType(node->type))
			{
				gen_code_i(CVTF,0,0);

			}
		 	if(isFloatType(node->rlink->type))
                        {
                                gen_code_i(CVTI,0,0);
                        }

		}

		break;
	case N_EXP_SIZE_TYPE:
		gen_code_i(LITI,0,*((int*)node->clink));
		break;
	case N_EXP_SIZE_EXP:
		gen_code_i(LITI,0,*((int*)node->clink));
		break;
	case N_EXP_PLUS:
		gen_expression(node->clink);
		break;
	case N_EXP_MINUS:
		gen_expression(node->clink);
		if (isFloatType(node->type))
		{
			gen_code_i(MINUSF, 0, 0);
		}
		else
		{
			gen_code_i(MINUSI, 0, 0);
		}

		break;
	case N_EXP_NOT:
		gen_expression(node->clink);
		gen_code_i(NOT, 0, 0);
		break;
	case N_EXP_AMP:
		gen_expression_left(node->clink);

		break;
	case N_EXP_STAR:
		gen_expression(node->clink);
		gen_code_i(LDI, 0, 0);
		break;
	case N_EXP_PRE_INC:
		gen_expression_left(node->clink);
		type = node->type;
		if (node->type->size == 1)
		{
			gen_code_i(LDXB, 0, 0);
		}
		else
		{
			gen_code_i(LDX, 0, 0);
		}
		if (isPointerOrArrayType(node->type))
		{
			gen_code_i(LITI, 0, node->type->element_type->size);
			gen_code_i(ADDI, 0, 0);
		}
		else if (isFloatType(node->type))
		{
			gen_code_i(INCF, 0, 0);
		}
		else
		{
			gen_code_i(INCI, 0, 0);
		}
		if (node->type->size == 1)
		{
			gen_code_i(STXB, 0, 0);
		}
		else
		{
			gen_code_i(STX, 0, 0);
		}
		break;
	case N_EXP_PRE_DEC:
		gen_expression_left(node->clink);
		type = node->type;
		if (node->type->size == 1)
		{
			gen_code_i(LDXB, 0, 0);
		}
		else
		{
			gen_code_i(LDX, 0, 0);
		}
		if (isPointerOrArrayType(node->type))
		{
			gen_code_i(LITI, 0, node->type->element_type->size);
			gen_code_i(SUBI, 0, 0);
		}
		else if (isFloatType(node->type))
		{
			gen_code_i(DECF, 0, 0);
		}
		else
		{
			gen_code_i(DECI, 0, 0);
		}
		if (node->type->size == 1)
		{
			gen_code_i(STXB, 0, 0);
		}
		else
		{
			gen_code_i(STX, 0, 0);
		}
		break;
		///////////////////////////////
		///multiplicative expression///
		///////////////////////////////
	case N_EXP_MUL:
		//expr1 * expr2
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(MULF, 0, 0);
		}
		else
		{
			gen_code_i(MULI, 0, 0);
		}
		break;
	case N_EXP_DIV:
		//expr1 / expr2
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(DIVF, 0, 0);
		}
		else
		{
			gen_code_i(DIVI, 0, 0);
		}


		break;
	case N_EXP_MOD:
		// expr1 % expr2
		gen_expression(node->llink);
		gen_expression(node->rlink);


		gen_code_i(MOD, 0, 0);

		break;
		/////////////////////////
		///additive expression///
		/////////////////////////
	case N_EXP_ADD:
		//expr1 + expr2
		gen_expression(node->llink);
		if( isPointerType(node->rlink->type))
		{
			int size=node->llink->type->size;
			gen_code_i(LITI,0,size);
			gen_code_i(MULI,0,0);
		}
		gen_expression(node->rlink);
		if( isPointerType(node->llink->type))
                {
                        int size=node->rlink->type->size;
                        gen_code_i(LITI,0,size);
			gen_code_i(MULI,0,0);
                }

		if (isFloatType(node->type))
		{
			gen_code_i(ADDF, 0, 0);
		}
		else
		{
			gen_code_i(ADDI, 0, 0);
		}

		break;
	case N_EXP_SUB:
		//expr1 - expr2
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(SUBF, 0, 0);
		}
		else
		{
			gen_code_i(SUBI, 0, 0);
		}
		break;
		///////////////////////////
		///relational expression///
		///////////////////////////
	case N_EXP_LSS:
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(LSSF, 0, 0);
		}
		else
		{
			gen_code_i(LSSI, 0, 0);
		}
		break;
	case N_EXP_GTR:
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(GTRF, 0, 0);
		}
		else
		{
			gen_code_i(GTRI, 0, 0);
		}
		break;
	case N_EXP_LEQ:
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(LEQF, 0, 0);
		}
		else
		{
			gen_code_i(LEQI, 0, 0);
		}
		break;
	case N_EXP_GEQ:
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(GEQF, 0, 0);
		}
		else
		{
			gen_code_i(GEQI, 0, 0);
		}
		break;
		/////////////////////////
		///equality expression///
		/////////////////////////
	case N_EXP_EQL:
		//expr1 == expr2
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(EQLF, 0, 0);
		}
		else
		{
			gen_code_i(EQLI, 0, 0);
		}
		break;
		break;
	case N_EXP_NEQ:
		//expr1 != expr2
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (isFloatType(node->type))
		{
			gen_code_i(NEQF, 0, 0);
		}
		else
		{
			gen_code_i(NEQI, 0, 0);
		}
		break;
		////////////////////////////
		///logical AND expression///
		////////////////////////////
	case N_EXP_AND:
		//expr1 && expr2
		gen_expression(node->llink);
		gen_expression(node->rlink);

		gen_code_i(AND, 0, 0);

		break;
	case N_EXP_OR:
		//expr1 || expr2
		gen_expression(node->llink);
		gen_expression(node->rlink);

		gen_code_i(OR, 0, 0);
		break;
		////////////////
		///expression///
		////////////////
	case N_EXP_ASSIGN:
		//expr1 = expr2
		gen_expression_left(node->llink);
		gen_expression(node->rlink);
		if (node->type->size == 1)
		{
			gen_code_i(STXB, 0, 0);
		}
		else
		{
			gen_code_i(STX, 0, 0);
		}
		break;
	}
}

/// <summary>
/// 치환연산 수식의 좌항 수식에 대한 asm을 출력한다.
/// </summary>
/// <param name="node"></param>
void gen_expression_left(const A_NODE* const node)
{
	A_ID* id;
	A_TYPE* type;
	int i;
	switch (node->name)
	{
	case  N_EXP_IDENT:
		id = (A_ID*)node->clink;
		type = id->type;
		switch (id->kind)
		{
		case ID_VAR:
			gen_code_i(LDA, id->level, id->address);
			break;
		case ID_PARM:
			switch (type->kind)
			{
			case T_ARRAY:
				gen_code_i(LOD, id->level, id->address);
				break;
			default:
				gen_code_i(LDA, id->level, id->address);
				break;
			}
		}
		break;

		////////////////////////
		///postfix_expression///
		////////////////////////
	case N_EXP_ARRAY:
		// expr1 [ expr2 ]
		gen_expression(node->llink);
		gen_expression(node->rlink);
		if (node->type->size > 1)
		{
			gen_code_i(LITI, 0, node->type->size);
			gen_code_i(MULI, 0, 0);
		}
		gen_code_i(OFFSET, 0, 0);
		break;
	case N_EXP_STRUCT:
		//expr1 . id
		gen_expression_left(node->llink);
		id=(A_ID*)node->rlink;
		if(id->address>0)
		{
			gen_code_i(LITI,0,id->address);
			gen_code_i(OFFSET,0,0);
		}

                break;
	case N_EXP_ARROW:
		//expr1 -> id
		gen_expression(node->llink);
		id=(A_ID*)node->rlink;
		if(id->address>0)
                {
                        gen_code_i(LITI,0,id->address);
                        gen_code_i(OFFSET,0,0);
                }
                break;
	case N_EXP_STAR:
		//* expr
		gen_expression(node->clink);
		break;

	}
}
int gen_arg_expression(const A_NODE* const node)
{
	switch (node->name)
	{
	case N_ARG_LIST:
		gen_expression(node->llink);
		gen_arg_expression(node->rlink);
		break;
	case N_ARG_LIST_NIL:
		break;
	default:
		gen_error(100, node->line, NULL);
	}
}

void gen_statement(const A_NODE* const node, const int cont_label, const int break_label)
{
	A_NODE* n;
	A_ID* id; 
	A_TYPE* type;
	int i, l1, l2, l3;
	switch (node->name)
	{
	case N_STMT_COMPOUND:
		if (node->llink)
		{
			gen_declaration_list((A_ID*)node->llink);
		}
		if (node->rlink)
		{
			gen_statement_list(node->rlink, cont_label, break_label);
		}
		break;
	case N_STMT_EMPTY:
		break;
	case N_STMT_EXPRESSION:
		gen_expression(node->clink);
		i = node->clink->type->size;
		if (i)
		{
			gen_code_i(POP, 0, i % 4 ? i / 4 + 1 : i / 4);
		}
		break;
	case N_STMT_IF:
		gen_expression(node->llink);
		gen_code_l(JPC, 0, l1 = get_label());
		gen_statement(node->rlink, cont_label, break_label);
		gen_label_number(l1);
		break;
	case N_STMT_IF_ELSE:
		gen_expression(node->llink);
		gen_code_l(JPC, 0, l1 = get_label());
		gen_statement(node->clink, cont_label, break_label);
		gen_code_l(JMP, 0, l2 = get_label());
		gen_label_number(l1);
		gen_statement(node->rlink, cont_label, break_label);
		gen_label_number(l2);
		break;
	case N_STMT_WHILE:
		l3 = get_label();
		gen_label_number(l1 = get_label());
		gen_expression(node->llink);
		gen_code_l(JPC, 0, l2 = get_label());
		gen_statement(node->rlink, l3, l2);
		gen_label_number(l3); gen_code_l(JMP, 0, l1);
		gen_label_number(l2);
		break;
	case N_STMT_DO:
		l3 = get_label();
		l2 = get_label();
		gen_label_number(l1 = get_label());
		gen_statement(node->llink, l2, l3);
		gen_label_number(l2);
		gen_expression(node->rlink);
		gen_code_l(JPT, 0, l1);
		gen_label_number(l3);
		break;
	case N_STMT_FOR:
		n = node->llink;
		l3 = get_label();
		if (n->llink)
		{
			gen_expression(n->llink);
			i = n->llink->type->size;
			if (i)
			{
				gen_code_i(POP, 0, i % 4 ? i / 4 + 1 : i / 4);
			}
		}
		gen_label_number(l1 = get_label());
		l2 = get_label();
		if (n->clink)
		{
			gen_expression(n->clink); gen_code_l(JPC, 0, l2);
		}
		gen_statement(node->rlink, l3, l2);
		gen_label_number(l3);
		if (n->rlink)
		{
			gen_expression(n->rlink);
			i = n->rlink->type->size;
			if (i)
			{
				gen_code_i(POP, 0, i % 4 ? i / 4 + 1 : i / 4);
			}
		}
		gen_code_l(JMP, 0, l1);
		gen_label_number(l2);
		break;
	case N_STMT_RETURN:
		if (node->clink)
		{
			gen_code_i(LDA, 1, -(node->clink->type->size));
			gen_expression(node->clink);
			gen_code_i(STO, 0, 0);
		}
		gen_code_i(RET, 0, 0);
		break;

	case	N_STMT_CONTINUE:
		gen_code_i(JMP, 0, cont_label);
		break;

	case	N_STMT_BREAK:
		gen_code_i(JMP, 0, break_label);
		break;

	}
}
void gen_statement_list(const A_NODE* const node, const int cont_label, const int break_label)
{
	switch (node->name)
	{
	case N_STMT_LIST:
		gen_statement(node->llink, cont_label, break_label);
		gen_statement_list(node->rlink, cont_label, break_label);
		break;
	case N_STMT_LIST_NIL:
		break;
	default:
		gen_error(100, node->line, NULL);
	}
}



void gen_code_i(OPCODE opcode, const int l, const int a)
{
	fprintf(fout, "\t%9s  %d, %d\n", opcode_name[opcode], l, a);
}
void gen_code_f(OPCODE opcode, const int l, const float f)
{
	fprintf(fout, "\t%9s  %d, %f\n", opcode_name[opcode], l, f);
}
void gen_code_s(OPCODE opcode, const int l, const char* const s)
{
	fprintf(fout, "\t%9s  %d, %s\n", opcode_name[opcode], l, s);
}
void gen_code_l(OPCODE opcode, const int l, const int label)
{
	fprintf(fout, "\t%9s  %d, L%d\n", opcode_name[opcode], l, label);
}
int get_label()
{
	++label_no;
	return label_no;
}
void gen_label_number(const int i)
{
	fprintf(fout, "L%d:\n", i);
}
void gen_label_name(const char* const s)
{
	fprintf(fout, "%s:\n", s);
}
void gen_error(const int i, const int ll, const char* const s)
{
	printf("generate error line %d:", ll);
	switch (i)
	{
	case 11:
		printf("illegal identifier in expression\n");
		break;
	case 12:
		printf("illegal l-value expression\n");
		break;
	case 13:
		printf("identifier %s not l-value expression \n",s);
		break;
	case 22:
		printf("no destination for continue statement\n");
		break;
	case 23:
		printf("no destination for break statement\n");
		break;
	case 100:
		printf("fatal compiler error during node generation\n");
		break;
	}
	

}


