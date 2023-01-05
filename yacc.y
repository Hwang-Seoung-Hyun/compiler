%token AUTO_SYM BREAK_SYM CASE_SYM CONST_SYM CONTINUE_SYM DEFAULT_SYM DO_SYM ELSE_SYM ENUM_SYM EXTERN_SYM FOR_SYM IF_SYM 
	REG_SYM RETURN_SYM SIZEOF_SYM STATIC_SYM STRUCT_SYM SWITCH_SYM TYPEDEF_SYM UNION_SYM WHILE_SYM PLUSPLUS MINUSMINUS 
	ARROW LSS QUESTION XOR GTR LEQ GEQ EQL NEQ AMPAMP BARBAR DOTDOTDOT LP RP LB RB LR RR COLON PERIOD 
	COMMA EXCL STAR SLASH PERCENT GOTO_SYM BAR RSFT LSFT TILDE AMP SEMICOLON PLUS MINUS ASSIGN 
	INTEGER_CONSTANT FLOAT_CONSTANT IDENTIFIER TYPE_IDENTIFIER STRING_LITERAL CHARACTER_CONSTANT EMPTY
%start program
%{
#define YYSTYPE_IS_DECLARED 1
typedef long YYSTYPE;
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include"type.h"
int yylex();
void yyerror();


A_ID* makeIdentifier(char*);
void checkForwordReference();
A_ID* setFunctionDeclaratorSpecifier(A_ID* const, A_SPECIFIER* const);
A_SPECIFIER* makeSpecifier(A_TYPE* const, S_KIND);
A_ID* setDeclaratorListSpecifier(A_ID* const,const A_SPECIFIER* const);
A_SPECIFIER* updateSpecifier(A_SPECIFIER* const, A_TYPE* const, S_KIND); 
A_ID* linkDeclaratorList(A_ID* const,A_ID* const);
//struct
A_TYPE* setTypeStructOrEnumIdentifier(T_KIND, char*, ID_KIND);
A_TYPE* setTypeField(A_TYPE* const,A_ID* const);
A_TYPE* getTypeOfStructOrEnumIdentifier(T_KIND, char*, ID_KIND);
A_ID* setStructDeclaratorListSpecifier(A_ID* const, const A_TYPE* const);
A_TYPE* makeType(T_KIND);
//enum
A_ID* setDeclaratorKind(A_ID* const, ID_KIND);
A_ID* setDeclaratorElementType(A_ID* const, A_TYPE* const);
A_TYPE* setTypeElementType(A_TYPE* const, A_TYPE* const);
//parameter
A_ID* makeDummyIdentifier();
A_ID* setParameterDeclaratorSpecifier(A_ID* const, A_SPECIFIER* const);
A_ID* setDeclaratorType(A_ID* const,A_TYPE* const);
void initialize();
void initIdArrayIndex();
A_ID* findPrevCurrentLevelId(A_ID* const);

//node
A_NODE* makeNode(NODE_NAME, A_NODE* const, A_NODE* const, A_NODE* const);
A_TYPE* setTypeExpr(A_TYPE* const, const A_NODE* const);
A_NODE* makeNodeList(NODE_NAME,A_NODE* const, A_NODE* const);
A_TYPE* setTypeNameSpecifier(A_TYPE* const, A_SPECIFIER* const);
A_ID* setFunctionDeclaratorBody(A_ID* const,A_NODE* const);
A_ID* setDeclaratorInit(A_ID* const, A_NODE* const);
A_ID* getIdentifierDeclared(const char* const);

extern int current_level;
extern  A_ID* current_id;
extern A_NODE* root;
extern A_TYPE* int_type;
extern char* identifier[];
extern A_TYPE* typeIdentifier;
extern int idArrayIndex_lex;
extern int idArrayIndex;
extern char* charConst;
extern char* stringLiteral;
extern int* intConst;
extern float* floatConst;
%}
%%
program : translation_unit {checkForwordReference(); root=makeNode(N_PROGRAM,0,(A_NODE*)$1,0); }
translation_unit : external_declaration {$$=$1; initIdArrayIndex();}
	| translation_unit external_declaration{$$=(long int)linkDeclaratorList((A_ID*)$1,(A_ID*)$2); initIdArrayIndex();}
external_declaration : function_definition{$$=$1;}
	| declaration{$$=$1;}

function_definition : declaration_specifiers declarator {$$=(long int)setFunctionDeclaratorSpecifier((A_ID*)$2,(A_SPECIFIER*)$1); } compound_statement {$$=(long int)setFunctionDeclaratorBody((A_ID*)$3,(A_NODE*)$4); initIdArrayIndex();}
	| declarator {$$=(long int)setFunctionDeclaratorSpecifier((A_ID*)$1,makeSpecifier(int_type,0));} compound_statement 
		{$$=(long int)setFunctionDeclaratorBody((A_ID*)$2,(A_NODE *)$3);}
declaration : declaration_specifiers init_declarator_list SEMICOLON {$$=(long int)setDeclaratorListSpecifier((A_ID*)$2,(A_SPECIFIER*)$1); initIdArrayIndex();}
declaration_specifiers : type_specifier {$$=(long int)makeSpecifier((A_TYPE*)$1,0);}
	| storage_class_specifier {$$=(long int)makeSpecifier((A_TYPE*)0,$1);}
	| type_qualifier
	| type_specifier declaration_specifiers {$$=(long int)updateSpecifier((A_SPECIFIER*)$2,(A_TYPE*)$1,0);}
	| storage_class_specifier declaration_specifiers {$$=(long int)updateSpecifier((A_SPECIFIER*)$2,0,$1);}
	| type_qualifier declaration_specifiers
storage_class_specifier : AUTO_SYM {$$=S_AUTO;}
			| STATIC_SYM {$$=S_STATIC;}
			| TYPEDEF_SYM {$$=S_TYPEDEF;}
			| REG_SYM  {$$=S_REGISTER;}
			| EXTERN_SYM
type_qualifier : CONST_SYM 
init_declarator_list : init_declarator {$$=$1;}
	| init_declarator_list COMMA init_declarator {$$=(long int)linkDeclaratorList((A_ID*)$1,(A_ID*)$3);}
init_declarator : declarator {$$=$1;}
	| declarator ASSIGN initializer {$$=(long int)setDeclaratorInit((A_ID*)$1,(A_NODE *)$3);}
type_specifier : struct_specifier {$$=$1;}
	| enum_specifier {$$=$1;}
	| TYPE_IDENTIFIER {$$=(long int)typeIdentifier;}
struct_specifier : struct_or_union IDENTIFIER {$$=(long int)setTypeStructOrEnumIdentifier($1,(identifier[idArrayIndex++]),ID_STRUCT);} 
		LR{$$=(long int)current_id; current_level++;} struct_declaration_list RR 
		{$$=(long int)setTypeField((A_TYPE*)$3,(A_ID*)$6);  checkForwordReference(); current_level--; current_id=(A_ID*)$5;}
	| struct_or_union {$$=(long int)makeType($1);} LR {$$=(long int)current_id; current_level++;} struct_declaration_list RR 
		{$$=(long int)setTypeField((A_TYPE*)$2,(A_ID*)$5); checkForwordReference(); current_level--; current_id=(A_ID*)$4;}
	| struct_or_union IDENTIFIER {$$=(long int)getTypeOfStructOrEnumIdentifier($1,identifier[idArrayIndex++],ID_STRUCT);}
struct_or_union : STRUCT_SYM {$$=T_STRUCT;}	 
	| UNION_SYM {$$=T_UNION;}
struct_declaration_list : struct_declaration {$$=$1;}
	| struct_declaration_list struct_declaration{$$=(long int)linkDeclaratorList((A_ID*)$1,(A_ID*)$2);}
struct_declaration : type_specifier struct_declarator_list SEMICOLON {$$=(long int)setStructDeclaratorListSpecifier((A_ID*)$2,(A_TYPE*)$1);} 
struct_declarator_list : struct_declarator {$$=$1;}
	| struct_declarator_list COMMA struct_declarator {$$=(long int)linkDeclaratorList((A_ID*)$1,(A_ID*)$3);}
struct_declarator : declarator {$$=$1;}
enum_specifier : ENUM_SYM IDENTIFIER {$$=(long int)setTypeStructOrEnumIdentifier(T_ENUM,identifier[idArrayIndex++],ID_ENUM);} 
		LR enumerator_list RR {$$=(long int)setTypeField((A_TYPE*)$3,(A_ID*)$5);}
	| ENUM_SYM {$$=(long int)makeType(T_ENUM);} LR enumerator_list RR {$$=(long int)setTypeField((A_TYPE*)$2,(A_ID*)$4);}
	| ENUM_SYM IDENTIFIER {$$=(long int)getTypeOfStructOrEnumIdentifier(T_ENUM,identifier[idArrayIndex++],ID_ENUM);}
enumerator_list : enumerator {$$=$1;}
	| enumerator_list COMMA enumerator {$$=(long int)linkDeclaratorList((A_ID*)$1,(A_ID*)$3);}

enumerator : IDENTIFIER {$$=(long int)setDeclaratorKind(makeIdentifier(identifier[idArrayIndex++]),ID_ENUM_LITERAL);}
	| IDENTIFIER {$$=(long int)setDeclaratorKind(makeIdentifier(identifier[idArrayIndex++]),ID_ENUM_LITERAL);} ASSIGN expression{$$=(long int)setDeclaratorInit((A_ID*)$2,(A_NODE*)$4);}
declarator : pointer direct_declarator {$$=(long int)setDeclaratorElementType((A_ID*)$2,(A_TYPE*)$1);}
	| direct_declarator {$$=$1;}
pointer: STAR {$$=(long int)makeType(T_POINTER);}
	| STAR pointer {$$=(long int)setTypeElementType((A_TYPE*)$2,makeType(T_POINTER));}
	| STAR type_qualifier {$$=(long int)makeType(T_POINTER);}
	| STAR type_qualifier pointer {$$=(long int)setTypeElementType((A_TYPE*)$3,makeType(T_POINTER));}
direct_declarator : IDENTIFIER {$$=(long int)makeIdentifier(identifier[idArrayIndex++]);}
	| LP declarator RP {$$=$2;}
	| direct_declarator LB constant_expression_opt RB {$$=(long int)setDeclaratorElementType((A_ID*)$1,setTypeExpr(makeType(T_ARRAY),(A_NODE*)$3));} 
	| direct_declarator LP {$$=(long int)current_id; current_level++;} parameter_type_list_opt RP {$$=(long int)setDeclaratorElementType((A_ID*)$1,setTypeField(makeType(T_FUNC),(A_ID*)$4)); checkForwordReference(); current_level--; current_id=(A_ID*)$3;} 
constant_expression_opt : /* empty */ {$$=0;} 
	| constant_expression {$$=$1;}
parameter_type_list_opt : /* empty */ {$$=0;} 
	| parameter_type_list {$$=$1;} 
parameter_type_list : parameter_list {$$=$1;} 
	| parameter_list COMMA DOTDOTDOT {$$=(long int)linkDeclaratorList((A_ID*)$1,setDeclaratorKind(makeDummyIdentifier(),ID_PARM));} 
parameter_list : parameter_declaration {$$=$1;}
	| parameter_list COMMA parameter_declaration {$$=(long int)linkDeclaratorList((A_ID*)$1,(A_ID*)$3);}
parameter_declaration : declaration_specifiers declarator {$$=(long int)setParameterDeclaratorSpecifier((A_ID*)$2,(A_SPECIFIER*)$1);}
	| declaration_specifiers abstract_declarator_opt {$$=(long int)setParameterDeclaratorSpecifier(setDeclaratorType(makeDummyIdentifier(),(A_TYPE*)$2),(A_SPECIFIER*)$1);}
abstract_declarator_opt : /* empty */ {$$=0;}
	| abstract_declarator {$$=$1;}
abstract_declarator : pointer {$$=$1;}
	| direct_abstract_declarator {$$=$1;}
	| pointer direct_abstract_declarator {$$=(long int)setTypeElementType((A_TYPE*)$2,(A_TYPE*)$1);}
direct_abstract_declarator : LP abstract_declarator RP {$$=$2;}
	| LB constant_expression_opt RB {$$=(long int)setTypeExpr(makeType(T_ARRAY),(A_NODE*)$2);}
	| LP parameter_type_list_opt RP {$$=(long int)setTypeField(makeType(T_FUNC),(A_ID*)$2);}
	| direct_abstract_declarator LB constant_expression_opt RB
		{$$=(long int)setTypeElementType((A_TYPE*)$1,setTypeExpr(makeType(T_ARRAY),(A_NODE*)$3));}
	| direct_abstract_declarator LP parameter_type_list_opt RP
		{$$=(long int)setTypeElementType((A_TYPE*)$1,setTypeField(makeType(T_FUNC),(A_ID*)$3));}
initializer : expression {$$=(long int)makeNode(N_INIT_LIST_ONE,0,(A_NODE*)$1,0);}
	| LR initializer_list RR {$$=$2;}
	| LR initializer_list COMMA RR {$$=$2;}
initializer_list : initializer {$$=(long int)makeNode(N_INIT_LIST,(A_NODE*)$1,0,makeNode(N_INIT_LIST_NIL,0,0,0));}
	| initializer_list COMMA initializer {$$=(long int)makeNodeList(N_INIT_LIST,(A_NODE*)$1,(A_NODE*)$3);}
statement : labeled_statement 	{$$=$1;}
	| compound_statement 	{$$=$1;}
	| expression_statement 	{$$=$1;}
	| selection_statement 	{$$=$1;}
	| iteration_statement 	{$$=$1;}
	| jump_statement 	{$$=$1;}
labeled_statement : CASE_SYM constant_expression COLON statement {$$=(long int)makeNode(N_STMT_LABEL_CASE,(A_NODE*)$2,0,(A_NODE*)$4);} 
	| DEFAULT_SYM COLON statement {$$=(long int)makeNode(N_STMT_LABEL_DEFAULT,(A_NODE*)0,(A_NODE*)$3,(A_NODE*)0);}
compound_statement : LR { $$=(long int)current_id; current_level++;}declaration_list_opt statement_list_opt RR {$$=(long int)makeNode(N_STMT_COMPOUND,(A_NODE*)$3,(A_NODE*)0,(A_NODE*)$4); checkForwordReference();   current_level--; current_id=(A_ID*)$2;}
declaration_list_opt : /* empty */ {$$=0;}
	| declaration_list {$$=$1;}
declaration_list : declaration {$$=$1;}
	| declaration_list declaration {$$=(long int)linkDeclaratorList((A_ID*)$1,(A_ID*)$2);}
statement_list_opt : /* empty */ {$$=(long int)makeNode(N_STMT_LIST_NIL,0,0,0);}
	| statement_list {$$=$1;}
statement_list : statement /* empty */ {$$=(long int)makeNode(N_STMT_LIST,(A_NODE*)$1,0,makeNode(N_STMT_LIST_NIL,0,0,0)); initIdArrayIndex();}
	| statement_list statement {$$=(long int)makeNodeList(N_STMT_LIST,(A_NODE*)$1,(A_NODE*)$2); initIdArrayIndex();}
expression_statement : SEMICOLON {$$=(long int)makeNode(N_STMT_EMPTY,0,0,0);}
	| expression SEMICOLON	{$$=(long int)makeNode(N_STMT_EXPRESSION,0,(A_NODE*)$1,0);}
selection_statement : IF_SYM LP expression RP statement {$$=(long int)makeNode(N_STMT_IF,(A_NODE*)$3,0,(A_NODE*)$5);}
	| IF_SYM LP expression RP statement ELSE_SYM statement {$$=(long int)makeNode(N_STMT_IF_ELSE,(A_NODE*)$3,(A_NODE*)$5,(A_NODE*)$7);}
	| SWITCH_SYM LP expression RP statement {$$=(long int)makeNode(N_STMT_SWITCH,(A_NODE*)$3,0,(A_NODE*)$5);}
iteration_statement : WHILE_SYM LP expression RP statement {$$=(long int)makeNode(N_STMT_WHILE,(A_NODE*)$3,0,(A_NODE*)$5);}
	| DO_SYM statement WHILE_SYM LP expression RP SEMICOLON {$$=(long int)makeNode(N_STMT_DO,(A_NODE*)$2,0,(A_NODE*)$5);}
	| FOR_SYM LP for_expression RP statement {$$=(long int)makeNode(N_STMT_FOR,(A_NODE*)$3,0,(A_NODE*)$5);}
for_expression: expression_opt SEMICOLON expression_opt SEMICOLON expression_opt {$$=(long int)makeNode(N_FOR_EXP,(A_NODE*)$1,(A_NODE*)$3,(A_NODE*)$5);}
expression_opt : /* empty */ {$$=0;}
	| expression {$$=$1;}
jump_statement : RETURN_SYM expression_opt SEMICOLON {$$=(long int)makeNode(N_STMT_RETURN,0,(A_NODE*)$2,0);}
	| CONTINUE_SYM SEMICOLON {$$=(long int)makeNode(N_STMT_CONTINUE,0,0,0);}
	| BREAK_SYM SEMICOLON {$$=(long int)makeNode(N_STMT_BREAK,0,0,0);}
primary_expression : IDENTIFIER {$$=(long int)makeNode(N_EXP_IDENT,0,(A_NODE*)getIdentifierDeclared(identifier[idArrayIndex++]),0);}
	| INTEGER_CONSTANT {$$=(long int)makeNode(N_EXP_INT_CONST,0,(A_NODE*)intConst,0);}
	| FLOAT_CONSTANT {$$=(long int)makeNode(N_EXP_FLOAT_CONST,0,(A_NODE*)floatConst,0);}
	| CHARACTER_CONSTANT {$$=(long int)makeNode(N_EXP_CHAR_CONST,0,(A_NODE*)charConst,0);}
	| STRING_LITERAL {$$=(long int)makeNode(N_EXP_STRING_LITERAL,0,(A_NODE*)stringLiteral,0);}
	| LP expression RP {$$=$2;}
postfix_expression : primary_expression {$$=$1;}
	| postfix_expression LB expression RB {$$=(long int)makeNode(N_EXP_ARRAY,(A_NODE*)$1,0,(A_NODE*)$3);}
	| postfix_expression LP arg_expression_list_opt RP {$$=(long int)makeNode(N_EXP_FUNCTION_CALL,(A_NODE*)$1,0,(A_NODE*)$3);}
	| postfix_expression PERIOD IDENTIFIER {$$=(long int)makeNode(N_EXP_STRUCT,(A_NODE*)$1,0,(A_NODE*)identifier[idArrayIndex++]);}
	| postfix_expression ARROW IDENTIFIER {$$=(long int)makeNode(N_EXP_ARROW,(A_NODE*)$1,0,(A_NODE*)identifier[idArrayIndex++]);}
	| postfix_expression PLUSPLUS {$$=(long int)makeNode(N_EXP_POST_INC,0,(A_NODE*)$1,0);}
	| postfix_expression MINUSMINUS {$$=(long int)makeNode(N_EXP_POST_DEC,0,(A_NODE*)$1,0);}
arg_expression_list_opt : /* empty */ {$$=(long int)makeNode(N_ARG_LIST_NIL,0,0,0);}
	| arg_expression_list{$$=$1;}
arg_expression_list : expression{$$=(long int)makeNode(N_ARG_LIST,(A_NODE*)$1,0,makeNode(N_ARG_LIST_NIL,0,0,0));}
	| arg_expression_list COMMA expression {$$=(long int)makeNodeList(N_ARG_LIST,(A_NODE*)$1,(A_NODE*)$3);}
unary_expression : postfix_expression {$$=$1;}
	| PLUSPLUS unary_expression {$$=(long int)makeNode(N_EXP_PRE_INC,0,(A_NODE*)$2,0);}
	| MINUSMINUS unary_expression {$$=(long int)makeNode(N_EXP_PRE_DEC,0,(A_NODE*)$2,0);}
	| AMP cast_expression {$$=(long int)makeNode(N_EXP_AMP,0,(A_NODE*)$2,0);}
	| STAR cast_expression {$$=(long int)makeNode(N_EXP_STAR,0,(A_NODE*)$2,0);}
	| EXCL cast_expression {$$=(long int)makeNode(N_EXP_NOT,0,(A_NODE*)$2,0);}
	| MINUS cast_expression {$$=(long int)makeNode(N_EXP_MINUS,0,(A_NODE*)$2,0);}
	| PLUS cast_expression {$$=$2;}
	| SIZEOF_SYM unary_expression {$$=(long int)makeNode(N_EXP_SIZE_EXP,0,(A_NODE*)$2,0);}
	| SIZEOF_SYM LP type_name RP {$$=(long int)makeNode(N_EXP_SIZE_TYPE,0,(A_NODE*)$3,0);}
cast_expression : unary_expression {$$=$1;}
	| LP type_name RP cast_expression {$$=(long int)makeNode(N_EXP_CAST,(A_NODE*)$2,0,(A_NODE*)$4);}
type_name : declaration_specifiers {$$=(long int)setTypeNameSpecifier(NULL,(A_SPECIFIER*)$1);}
	| declaration_specifiers abstract_declarator {$$=(long int)setTypeNameSpecifier((A_TYPE*)$2,(A_SPECIFIER*)$1);}
multiplicative_expression : cast_expression {$$=$1;}
	| multiplicative_expression STAR cast_expression {$$=(long int)makeNode(N_EXP_MUL,(A_NODE*)$1,0,(A_NODE*)$3);} 
	| multiplicative_expression SLASH cast_expression {$$=(long int)makeNode(N_EXP_DIV,(A_NODE*)$1,0,(A_NODE*)$3);}
	| multiplicative_expression PERCENT cast_expression {$$=(long int)makeNode(N_EXP_MOD,(A_NODE*)$1,0,(A_NODE*)$3);}
additive_expression : multiplicative_expression {$$=$1;}
	| additive_expression PLUS multiplicative_expression {$$=(long int)makeNode(N_EXP_ADD,(A_NODE*)$1,0,(A_NODE*)$3);}
	| additive_expression MINUS multiplicative_expression {$$=(long int)makeNode(N_EXP_SUB,(A_NODE*)$1,0,(A_NODE*)$3);}
shift_expression : additive_expression {$$=$1;}
	| shift_expression LSFT additive_expression 
	| shift_expression RSFT additive_expression 
relational_expression : shift_expression {$$=$1;}
	| relational_expression LSS shift_expression {$$=(long int)makeNode(N_EXP_LSS,(A_NODE*)$1,0,(A_NODE*)$3);}
	| relational_expression GTR shift_expression  {$$=(long int)makeNode(N_EXP_GTR,(A_NODE*)$1,0,(A_NODE*)$3);}
	| relational_expression LEQ shift_expression  {$$=(long int)makeNode(N_EXP_LEQ,(A_NODE*)$1,0,(A_NODE*)$3);}
	| relational_expression GEQ shift_expression  {$$=(long int)makeNode(N_EXP_GEQ,(A_NODE*)$1,0,(A_NODE*)$3);}
equality_expression : relational_expression {$$=$1;}
	| equality_expression EQL relational_expression  {$$=(long int)makeNode(N_EXP_EQL,(A_NODE*)$1,0,(A_NODE*)$3);}
	| equality_expression NEQ relational_expression  {$$=(long int)makeNode(N_EXP_NEQ,(A_NODE*)$1,0,(A_NODE*)$3);}
logical_AND_expression : equality_expression {$$=$1;}
	| logical_AND_expression AMPAMP equality_expression {$$=(long int)makeNode(N_EXP_AND,(A_NODE*)$1,0,(A_NODE*)$3);}
logical_OR_expression : logical_AND_expression {$$=$1;}
	| logical_OR_expression BARBAR logical_AND_expression {$$=(long int)makeNode(N_EXP_OR,(A_NODE*)$1,0,(A_NODE*)$3);}
constant_expression : logical_OR_expression {$$=$1;}
expression : constant_expression {$$=$1;}
	| unary_expression ASSIGN expression {$$=(long int)makeNode(N_EXP_ASSIGN,(A_NODE*)$1,0,(A_NODE*)$3);}
