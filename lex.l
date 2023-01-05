digit	[0-9]
letter	[a-zA-Z_]
delim	[\t]
line	[\n]
ws	{delim}+

%{
#define MAX_ID_ARRAY_LENGTH 50
#define YYSTYPE_IS_DECLARED 1
typedef int YYSTYPE;
#include<stdio.h>
#include<string.h>
#include"type.h"
#include "yacc.tab.h"
extern int line_no;
int checkIdentifier();
A_ID* findPrevIdTypeUseName_null(char*);
int str2i(const char* const);
float str2f(const char* const);
char* identifier[MAX_ID_ARRAY_LENGTH];
char* stringLiteral;
char* charConst;
int* intConst;
float* floatConst;
A_TYPE* typeIdentifier;
extern int current_level;
extern A_ID* current_id;
int idArrayIndex_lex=0;
extern int idArrayIndex;
%}

%%
" "	{;}
{ws} 	{;}
{line} 	{++line_no;}
auto 	{ return(AUTO_SYM);}
break	{ return(BREAK_SYM);}
case	{ return(CASE_SYM);}
const	{ return(CONST_SYM);}
continue 	{ return(CONTINUE_SYM);}
default	{ return(DEFAULT_SYM);}
do	{ return(DO_SYM);}
else	{ return(ELSE_SYM);}
enum	{ return(ENUM_SYM);}
extern	{ return(EXTERN_SYM);}
for	{  return(FOR_SYM);}
if	{ return(IF_SYM);}
register	{ return(REG_SYM);}
return	{ return(RETURN_SYM);}
sizeof	{ return(SIZEOF_SYM);}
static	{ return(STATIC_SYM);}
struct	{ return(STRUCT_SYM);}
switch	{ return(SWITCH_SYM);}
typedef	{ return(TYPEDEF_SYM);}
union	{ return(UNION_SYM);}
while	{ return(WHILE_SYM);}
"++"	{ return(PLUSPLUS); }
"--"	{ return(MINUSMINUS); }
"->"	{ return(ARROW); }
"<"	{ return(LSS); }
">"	{ return(GTR); }
"<="	{ return(LEQ); }
">="	{ return(GEQ); }
"=="	{ return(EQL); }
"!="	{ return(NEQ); }
"&&"	{ return(AMPAMP); }
"||"	{ return(BARBAR); }
"..."	{ return(DOTDOTDOT); }
"("	{ return(LP); }
")"	{ return(RP); }
"["	{ return(LB); }
"]"	{ return(RB); }
"{"	{ return(LR); }
"}"	{ return(RR); }
":"	{ return(COLON); }
"."	{ return(PERIOD); }
","	{ return(COMMA); }
"!"	{ return(EXCL); }
"*"	{ return(STAR); }
"/"	{ return(SLASH); }
"%"	{ return(PERCENT); }
"&"	{ return(AMP); }
";"	{ return(SEMICOLON); }
"+"	{ return(PLUS); }
"-"	{ return(MINUS); }
"="	{ return(ASSIGN); } 
"~"	{ return (TILDE); }
{digit}+	{ intConst=malloc(sizeof(int)); *intConst=str2i(yytext); return(INTEGER_CONSTANT);}
{digit}+\.{digit}+	{ floatConst=malloc(sizeof(float)); *floatConst=str2f(yytext); return(FLOAT_CONSTANT);}
{letter}({letter}|{digit})*	{ return(checkIdentifier(yytext));}
\"([^"\n]|\\["\n])*\"	{ stringLiteral=malloc(sizeof(yytext)); strcpy(stringLiteral,yytext); return(STRING_LITERAL);}
\'([^'\n]|\'\')\'	{ charConst=malloc(sizeof(yytext));  *charConst=*(yytext) /*strcpy(charConst,yytext)*/; return(CHARACTER_CONSTANT);}
%%

int checkIdentifier(char *s)
{

	/*
	strcpy(identifier,s);
	char *table[]={"int","float","char","void"};
	int same=0;
	for(int k=0;k<3;k++)
	{
		if(*(s+k)==*(table[0]+k))
		{
			++same;
		}
	}
	if(same==3)
	{
		 return(TYPE_IDENTIFIER);
	}

	for(int i=1;i<4;i++)
	{
		same=0;
		for(int k=0;k<4;k++)
		{
			if(*(s+k)==*(table[i]+k))
			{
				++same;
			}
		}
		if(same==4)
		{
			return(TYPE_IDENTIFIER);
		}
	}
	return(IDENTIFIER);
	*/
	identifier[idArrayIndex_lex]=malloc(sizeof(char)*strlen(s));

	strcpy(identifier[idArrayIndex_lex],s);
	A_ID* preDefType = findPrevIdTypeUseName_null(s);

	if(preDefType)
	{
		free(identifier[idArrayIndex_lex]);
		typeIdentifier=preDefType->type;
		return(TYPE_IDENTIFIER);
	}
	else
	{
		++idArrayIndex_lex;
		return(IDENTIFIER);
	}
		
}
//find symbol table, which name is s and kind is ID_TYPE
A_ID* findPrevIdTypeUseName_null(char* const s)
{
	A_ID* prev=current_id;
	while(prev)
	{
		char* prevName_null = prev->name;
		if(prevName_null && strcmp(prevName_null,s)==0 && prev->kind==ID_TYPE)
		{
			return prev;
		}
		prev=prev->prev;
	}
	return NULL;
}


int str2i(const char* const s)
{
	int intConst=0;
	for(int i=0;*(s+i)!='\0';i++)
	{
		intConst*=10;
		intConst+=*(s+i)-'0';
	}
	return intConst;
}
float str2f(const char* const s)
{
	float floatConst=0.0f;
	int i=0;
	for(i=0;*(s+i)!='.';i++)
	{
		floatConst*=10.0f;
		floatConst+=(float)(*(s+i)-'0');
	}
	++i;
	float dotSum=0.0f;
	for(;*(s+i)!='\0';i++)
	{
		dotSum+=(float)(*(s+i)-'0');
		dotSum/=10.0f;
	}
	floatConst+=dotSum;
	return floatConst;
}

