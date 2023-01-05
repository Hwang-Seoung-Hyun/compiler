#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include"yacc.tab.h"
#include"type.h"
//#define YYSTYPE_IS_DECLARED
//typedef long int YYSTYPE;
int line_no=1;
int current_level = 0;
int idArrayIndex = 0;
A_ID* current_id=NULL;
int isError = 0;
A_TYPE* int_type;
A_TYPE* char_type;
A_TYPE* float_type;
A_TYPE* void_type;
A_TYPE* string_type;

A_NODE* root;
extern A_TYPE* typeIdentifier;
extern int idArrayIndex_lex;

A_ID* setDeclaratorElementType(A_ID* const id, const A_TYPE* const elemantType);
void syntexError(int error,const char* idName);
A_TYPE* makeType(T_KIND);
A_TYPE* setTypeElementType(A_TYPE* const, A_TYPE* const);
A_ID * setStructDeclaratorListSpecifier(A_ID* const id, const A_TYPE* const type);
bool checkProtoParameter(A_ID* const id, A_ID* const prev);
bool checkProtoReturn(const A_ID* const, const A_ID* const);
A_ID* findPrevCurrentLevelId_null(A_ID* const id);
A_ID* setDeclaratorType(A_ID* const,A_TYPE* const);
void initialize();
//node
A_NODE* makeNode(NODE_NAME, A_NODE* const, A_NODE* const, A_NODE* const);
A_TYPE* setTypeExpr(A_TYPE* const, const A_NODE* const);
A_NODE* makeNodeList(NODE_NAME,A_NODE* const, A_NODE* const);
A_TYPE* setTypeNameSpecifier(A_TYPE* const, A_SPECIFIER* const);
//hsh func
A_ID* findPrevIdUseName_null(const char* const s);
void setLinkDeclaratorSepecifier(A_ID* const, const A_SPECIFIER* const);
void setLinkStructDeclaratorType(A_ID* const id, const A_TYPE* const type);
A_ID* setDeclaratorTypeAndKind(A_ID* const id, A_TYPE* const type,const ID_KIND);



void print_ast(A_NODE *);

int syntex_analize()
{
	initialize();
        yyparse();
	if(!isError)
	{
		print_ast(root);
		//printf("correct c code\n");
		return 0;
	}
	else
	{
		return -1;
	}
}
void yyerror()
{
        printf("\nSyntex Error: %dth line\n",line_no);
        exit(1);
}

/// <summary>
/// make symbol table
/// set name, level, line, prev
/// </summary>
/// <param name="s">id name</param>
/// <returns>tabel pointer</returns>
A_ID* makeIdentifier(char* s)
{
    A_ID* id = malloc(sizeof(A_ID));
    id->name = s;
    id->line = line_no;
    id->level = current_level;
    id->prev = current_id;
    current_id = id;
    //초기화
    id->kind=ID_NULL;
    id->specifier=S_NULL;
    id->address=0;
    id->value=0;
    id->init=NULL;
    id->type=NULL;
    id->link=NULL;

    return id;
}
/// <summary>
/// current level과 같은 level, kind가 ID_STRUCT, ID_ENUM인 id->type의 field 알맹이가 있는지 검사
/// </summary>
void checkForwordReference()
{
    A_ID* id = current_id;
    while(id&&id->level>=current_level)
    {
	if(id->kind==ID_NULL)
	{
		syntexError(31, id->name);
	}
	else if ((id->kind==ID_STRUCT|id->kind==ID_ENUM)&&id->type->field == NULL)
        {
            syntexError(32, id->name);
        }
        id = id->prev;
    } 
}
/// <summary>
/// in function_definition, set id->type->...->element_type= specifier->type
/// check symentics
/// 1. redeclarated id with same level
/// 2. if predeclarated id is ID_FUNCTION, check parameter
/// 3. make parameter be able to use in compound statement
/// 4. check if id->type->kind is T_FUNC
/// </summary>
/// <param name="id">symbol table</param>
/// <param name="specifier">specifier table</param>
/// <returns>symbol table</returns>
A_ID* setFunctionDeclaratorSpecifier(A_ID* const id, A_SPECIFIER* const specifier)
{
	
    //set return type
    A_TYPE* type = id->type;
    if(specifier->type)
    {
    	setDeclaratorElementType(id, specifier->type);
    }
    else
    {
	id->type=int_type;
    }
    if(specifier->stor)
    {
	syntexError(25,NULL);
    }
    else
    {
    	id->specifier = S_NULL;
    }
    id->kind = ID_FUNC;

    //check declarator type
    if (id->type->kind != T_FUNC)
    //if(0)
    {
        syntexError(21,id->name);
        return id;
    }
    else
    {
        //check redeclaration
        A_ID* prev = findPrevCurrentLevelId_null(id);
	if(prev)
	{
        	if (prev->kind != ID_FUNC)
        	{
            		syntexError(12, id->name);
            		return id;
        	}
        	//check proto parameter
        	if(checkProtoParameter(id,prev))
        	{
            		//check return type
            		A_TYPE* idReturnType, * previdReturnType;

        	}
        	else
        	{
            		syntexError(22, id->name);
			//return id;
            		
        	}	
	}//end of "if(prev)"

	//change parameter scope
	A_ID* idField = id->type->field;
        while (idField)
	{                
		if (idField)
		{
			if(strlen(idField->name))
			{
				current_id = idField;
			}
			else if (idField->type != int_type)
                        {
                                syntexError(23, id->name);
                        }
		}
		idField = idField->link;
	}
    }//end else of "if (id->type->kind != T_FUNC)"
    return id;
    
}
/// <summary>
/// make specifier table
/// </summary>
/// <param name="type">type pointer</param>
/// <param name="kind">storage class specifier</param>
/// <returns></returns>
A_SPECIFIER * makeSpecifier(A_TYPE * const type, S_KIND kind)
{
    A_SPECIFIER* specifier = malloc(sizeof(A_SPECIFIER));
    specifier->type = type;
    specifier->stor = kind;
    specifier->line = line_no;
}
/// <summary>
/// in Declaration, set id->type->...->element_type = specifier->type
/// and set ID_KIND
/// check sementics
/// 1. redeclarated id
/// 2. 
/// </summary>
/// <param name="id"></param>
/// <param name="specifier"></param>
/// <returns></returns>
A_ID * setDeclaratorListSpecifier(A_ID * const id, const A_SPECIFIER * const specifier)
{
    A_TYPE* type;
    A_ID* linkId = id;
    while(linkId)
    {
    	setDeclaratorElementType(linkId, specifier->type);
    	linkId->specifier = specifier->stor;
    	type=linkId->type;
    	if (linkId->specifier == S_TYPEDEF)
    	{
        	linkId->kind = ID_TYPE;
    	}
    	else
    	{
        	switch (type->kind)
        	{
        	case T_FUNC:
            		linkId->kind = ID_FUNC;
            		break;

        	case T_STRUCT:
        	case T_ENUM:
        	case T_POINTER:
        	case T_UNION:
        	case T_ARRAY:
        	case T_NULL:
        	case T_VOID:
            	linkId->kind = ID_VAR;
	    	if(linkId->specifier==S_NULL)
        	{
                	linkId->specifier=S_AUTO;
        	}
            	break;
        	}//end of "switch (type->kind)"
    	}
   
    	//setLinkDeclaratorSepecifier(id, specifier);

    	//|check redeclaration
    	A_ID* prev = findPrevCurrentLevelId_null(linkId);
    	if (prev)
    	{
        	syntexError(12,linkId->name);
        	return prev;
    	}

	linkId=linkId->link;
    }//end of "while(linkId)"
    return id;
    
}
/// <summary>
/// update specifier table
/// </summary>
/// <param name="specifier">old sepecifer tabel pointer</param>
/// <param name="type_null">type table(nullable)</param>
/// <param name="kind">kind of storage_class_specifier</param>
/// <returns>updated specifier table pointer</returns>
A_SPECIFIER * updateSpecifier(A_SPECIFIER * const specifier, A_TYPE * type_null, S_KIND kind)
{
    if (type_null)
    {
        specifier->type = type_null;
    }
    if (kind)
    {
	if(specifier->stor)
	{
		syntexError(24,NULL);
	}
	else
	{
        	specifier->stor = kind;
	}
    }
    return specifier;
}
/// <summary>
/// link id1 id2, using link pointer
/// </summary>
/// <param name="id1"></param>
/// <param name="id2"></param>
/// <returns>id1 pointer</returns>
A_ID * linkDeclaratorList(A_ID * const id1, A_ID* const id2)
{
    A_ID* linkEndId=id1;
    while(linkEndId->link)
    {
	    linkEndId=linkEndId->link;
    }
    linkEndId->link=id2;
    
    return id1;
}
/// <summary>
/// make symbol table and type table for struct or enum type
/// check sementic
/// 1. redeclarated id
/// 2. if redeclarated struct id and type table have field, error
/// </summary>
/// <param name="tKind">kind of type table</param>
/// <param name="s">name of symbol table</param>
/// <param name="idKind">kind of symbol table</param>
/// <returns>struct or enum type table pointer</returns>
A_TYPE * setTypeStructOrEnumIdentifier(T_KIND tKind, char* s, ID_KIND idKind)
{
    
    
    //check redeclaration
    A_ID* prev = current_id;
    
    while (prev && prev->level >= current_level)
    {
        if (strcmp(prev->name, s) == 0)
        {
            goto FIND;
        }
        prev = prev->prev;
    }

    prev = NULL;

    FIND:
    if (prev)
    {
        if (idKind == ID_ENUM)
        {
            syntexError(12, s);
            return prev->type;
        }
        else if (idKind == ID_STRUCT && prev->type->field)//predeclaraed struct type table's field is not empty
        {
            syntexError(12, s);
            return prev->type;
        }
        else if (idKind == ID_STRUCT)//predeclaraed struct type table's field is empty
        {
            return prev->type;
        }
        else
        {
            syntexError(12, s);
            return prev->type;
        }
    }
    else
    {
        A_TYPE* type = makeType(tKind);
        A_ID* id = makeIdentifier(s);
        id->kind = idKind;
        id->type = type;
        id->line = line_no;
        return type;
    }
}
/// <summary>
/// set type table->field 
/// </summary>
/// <param name="type">type table pointer</param>
/// <param name="id">first feild symbol table</param>
/// <returns>type table pointer</returns>
A_TYPE * setTypeField(A_TYPE* const type, A_ID* const id)
{
    type->field = id;
    return type;
}
/// <summary>
/// find predeclarated symbol table's struct or enum type table
/// if exist, return prev type table
/// if not exist and type is struct, make symbol table and type table
/// </summary>
/// <param name="tKind"></param>
/// <param name="s"></param>
/// <param name="idKind"></param>
/// <returns></returns>
A_TYPE * getTypeOfStructOrEnumIdentifier(T_KIND tKind, char* s, ID_KIND idKind)
{
    //check redeclaration
    A_ID* prev = current_id;

    while (prev)
    {
        if (strcmp(prev->name, s) == 0)
        {
            goto FIND;
        }
        prev = prev->prev;
    }

    prev = NULL;
FIND:
    if (prev)
    {
        if (prev->kind == ID_ENUM && idKind == ID_ENUM)
        {
            return prev->type;
        }
        else if (prev->kind == ID_STRUCT && idKind == ID_STRUCT)
        {
            return prev->type;
        }
        else
        {
            syntexError(11, s);
            return prev->type;
        }
    }
    else
    {
	
        if (idKind == ID_ENUM)
        {
            syntexError(13, s);
            return NULL;
        }
        else if (idKind == ID_STRUCT)
        {
            A_ID* id = makeIdentifier(s);
            A_TYPE* type = makeType(tKind);
            id->kind = idKind;
            id->line = line_no;
            id->type = type;
            return type;
        }
    }
}

/// <summary>
/// in Declaration, set id->type->...->element_type = specifier->type
/// and set ID_KIND ID_FIELD
/// check sementics
/// 1. redeclarated id
/// </summary>
/// <param name="id">struct field symbol tabel</param>
/// <param name="type">struct type table</param>
/// <returns>symbol table pointer</returns>
A_ID * setStructDeclaratorListSpecifier(A_ID* const id, const A_TYPE* const type)
{
    A_ID* linkId=id;
    while(linkId)
    {

    	setDeclaratorElementType(linkId, type);
    	linkId->kind = ID_FIELD;
    	A_ID* prev = findPrevCurrentLevelId_null(linkId);
    	if (prev)
    	{
        	syntexError(12, linkId->name);
        	return prev;
    	}
    	//setLinkStructDeclaratorType(linkId,type);
	linkId=linkId->link;
    
    }
    return id;
}
/// <summary>
/// make type table 
/// </summary>
/// <param name="kind">kind of type table</param>
/// <returns>type table pointer</returns>
A_TYPE * makeType(T_KIND kind)
{
    A_TYPE* type = malloc(sizeof(A_TYPE));
    type->kind = kind;
    type->line = line_no;
    type->size=0;
    type->local_var_size = 0;
    type->element_type = NULL;
    type->field=NULL;
    type->expr=NULL;
    type->check=FALSE;
    type->prt=FALSE;


    return type;
}
A_ID* setDeclaratorKind(A_ID* const id, ID_KIND kind)
{
    id->kind = kind;
    return id;
}
A_ID* setDeclaratorElementType(A_ID* const id, const A_TYPE* const elemantType)
{
    A_TYPE* type = id->type;
    if (type)
    {
        while (type->element_type)
        {
            type = type->element_type;
        }
        type->element_type = (A_TYPE*)elemantType;
    }
    else
    {
        id->type = (A_TYPE*)elemantType;
    }
    return id;
}
A_TYPE * setTypeElementType(A_TYPE* const type, A_TYPE* const typeElement)
{
    A_TYPE* endType = type;
    if(endType)
    {
	while (endType->element_type != NULL)
        {
        	endType = endType->element_type;
	}
    }
    
    endType->element_type = typeElement;
    return type;
}
A_ID* makeDummyIdentifier()
{
    A_ID* id = malloc(sizeof(A_ID));
    id->name=" ";
    id->line = line_no;
    id->level = current_level;
    id->prev = current_id;
    current_id = id;
    return id;
}
A_ID* setParameterDeclaratorSpecifier(A_ID* const id, A_SPECIFIER* const specifier)
{
    setDeclaratorElementType(id, specifier->type);
    id->kind = ID_PARM;
    id->specifier = specifier->stor;
    //check if Dummy Identifier
    if(id->name)
    {
	A_ID* prev = findPrevCurrentLevelId_null(id);
    	if (prev)
    	{
        	syntexError(12, id->name);
    	}
    }
    
    if(specifier->stor||(id->type==void_type&&strcmp(id->name," ")))
    {
	    syntexError(14,id->name);
    }
    return id;
}
void syntexError(int error,const char* idName)
{
    isError = 1;
    printf("syntex error line %d: ", line_no);
    switch (error)
    {
    case 11:
        printf("illegal referencing struct or union identifier \"%s\"", idName);
        break;
    case 12:
        printf("redeclaration of identifier \"%s\"", idName);
        break;
        
    case 13:
        printf("undefined identifier \"%s\"", idName);
        break;

    case 14:
        printf("illegal type specifier in formal parameters");
        break;

    case 20:
        printf("illegal storage class in type specifiers");
        break;

    case 21:
        printf("illegal function declarator");
        break;

    case 22:
        printf("conflicting parameter type in prototype function \"%s\"", idName);
        break;

    case 23:
        printf("empty parameter name");
        break;

    case 24:
        printf("illegal declaration specifiers");
        break;

    case 25:
        printf("illegal function specifiers");
        break;

    case 26:
        printf("illegal or conflicting return type in prototype function \"%s\"",idName);
        break;

    case 31:
        printf("undefined type for identifier \"%s\"",idName);
        break;

    case 32:
        printf("incomplete forward reference for identifier \"%s\"",idName);
        break;

    }
    printf("\n");
}

A_ID* findPrevCurrentLevelId_null(A_ID* const id)
{
    if(strcmp(id->name," ")==0)
    {
	    return NULL;//
    }
    A_ID* prev = id;
    prev = prev->prev;
    while (prev && prev->level >= id->level)
    {
	char* prevName_null = prev->name;
        if (prevName_null && strcmp(prevName_null, id->name) == 0)
        {

            return prev;//find
        }
        prev = prev->prev;
    }
    return NULL;//not find
}

bool checkProtoParameter(A_ID* const id, A_ID* const prev)
{
    A_ID* idField = id->type->field, * prevField = prev->type->field;
    while (idField && prevField)
    {
        A_TYPE* idFieldType = idField->type, * prevFieldType = prevField->type;
        while (idFieldType && prevFieldType)
        {
            if (idFieldType->kind != prevFieldType->kind)
            {
                return false;
            }
            idFieldType = idFieldType->element_type;
            prevFieldType = prevFieldType->element_type;
        }
        if (idFieldType != NULL || prevFieldType != NULL)
        {
            return false;
        }
        idField = idField->link;
        prevField = prevField->link;
    }
    if (idField == NULL && prevField == NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool checkProtoReturn(const A_ID* const id, const A_ID* const prev)
{
    A_TYPE* idReturn = id->type->element_type;
    A_TYPE* prevReturn = id->type->element_type;

    while (idReturn && prevReturn)
    {
        if (idReturn->kind != prevReturn->kind)
        {
            syntexError(26, id->name);
            return false;
        }
    }
    return true;
}
void initialize()
{
	//predeclarated type
	A_ID* id = makeIdentifier("int");
	int_type=makeType(T_ENUM);
	int_type->size=4;
	id = setDeclaratorElementType(id,int_type);
	id->kind=ID_TYPE;
	id->type->check=TRUE;


	id= makeIdentifier("char");
	char_type=makeType(T_ENUM);
	char_type->size=4;
	id = setDeclaratorElementType(id,char_type);
	id->kind=ID_TYPE;
	id->type->check=TRUE;

	id= makeIdentifier("float");
	float_type=makeType(T_ENUM);
        float_type->size=4;
        id = setDeclaratorElementType(id,float_type);
	id->kind=ID_TYPE;
	id->type->check=TRUE;
	
	id= makeIdentifier("void");
        void_type=makeType(T_VOID);
        void_type->size=0;
        id = setDeclaratorElementType(id,void_type);
        id->kind=ID_TYPE;
	id->type->check=TRUE;


	string_type=setTypeElementType(makeType(T_POINTER),char_type);
	string_type->size=4;

	//predeclarated function
	id=setDeclaratorTypeAndKind
		( 
		  makeIdentifier("printf"),
		  setTypeField
		  ( 
		  	setTypeElementType
			(
			   	makeType(T_FUNC),void_type
			),

			linkDeclaratorList
			(
			     	setDeclaratorTypeAndKind( makeDummyIdentifier(), string_type, ID_PARM),
				setDeclaratorKind( makeDummyIdentifier(), ID_PARM)
			)
		 ),
		 ID_FUNC
		);

	id=setDeclaratorTypeAndKind
                ( makeIdentifier("scanf"),
                  setTypeField
                  (
                        setTypeElementType
                        (
                                makeType(T_FUNC),void_type
                        ),
                        linkDeclaratorList
                        (
                                setDeclaratorTypeAndKind( makeDummyIdentifier(), string_type, ID_PARM),
                                setDeclaratorKind( makeDummyIdentifier(), ID_PARM)
                        )
                 ),
                 ID_FUNC);


	id = setDeclaratorTypeAndKind
    	(
        	makeIdentifier("malloc"),
        	setTypeField
        	(
            		setTypeElementType
            		(
                		makeType(T_FUNC), setTypeElementType(makeType(T_POINTER),void_type)
            		),

                	setDeclaratorTypeAndKind(makeDummyIdentifier(), int_type, ID_PARM)
        	),
        	ID_FUNC
    	);
	id->type->element_type->size=4;


	id = setDeclaratorTypeAndKind
        (
            makeIdentifier("free"),
            setTypeField
            (
                setTypeElementType
                (
                    makeType(T_FUNC), void_type
                ),

                setDeclaratorTypeAndKind
		(
		 	makeDummyIdentifier(), 
			setTypeElementType(makeType(T_POINTER),void_type),
			ID_PARM
		)
            ),
            ID_FUNC
        );


	idArrayIndex = 0;
} 

A_ID* setDeclaratorType(A_ID* const id,A_TYPE* const type)
{
	id->type=type;
	return id;
}

//node
A_NODE* makeNode(NODE_NAME name, A_NODE* const left, A_NODE* const center, A_NODE* const right)
{
	A_NODE* node = malloc(sizeof(A_NODE));
	memset(node,0,sizeof(A_NODE*));
	node->name=name;
	node->line=line_no;
	node->llink=left;
	node->clink=center;
	node->rlink=right;
	return node;

}
A_TYPE* setTypeExpr(A_TYPE* const type, const A_NODE* const node)
{
	type->expr=(A_NODE*)node;
}
/// <summary>
/// add node in node list
/// add node between node and righteset node
/// </summary>
/// <param name="name"></param>
/// <param name="nodeList">node list pointer</param>
/// <param name="addNode">node pointer</param>
/// <returns>node list Pointer</returns>
A_NODE* makeNodeList(NODE_NAME name,A_NODE* const nodeList, A_NODE* const addNode)
{
	A_NODE* node = nodeList;
	//find rightest node (N_STMT_LIST_NIL)
	while(node->rlink->rlink!=NULL)
	{
		node = node->rlink;
	}
	//make N_STMT_LIST
	A_NODE* stmt_list_node=malloc(sizeof(A_NODE));
	stmt_list_node->name=name;
	stmt_list_node->llink=addNode;
	stmt_list_node->rlink=node->rlink;
	stmt_list_node->line=line_no;
	stmt_list_node->clink=NULL;
	stmt_list_node->value=0;
	stmt_list_node->type=NULL;
	node->rlink=stmt_list_node;
	return nodeList;
}
A_TYPE* setTypeNameSpecifier(A_TYPE* const type, A_SPECIFIER* const specifier)
{
	if(specifier->stor)
	{
		syntexError(20,NULL);
	}
	if(specifier->type==NULL)
	{
		specifier->type=int_type;
	}
	specifier->stor=S_AUTO;
	if(type)
	{
		setTypeElementType(type,specifier->type);
		return type;
	}
	else
	{
		return specifier->type;
	}
}

A_ID* setFunctionDeclaratorBody(A_ID* const id,A_NODE* const node)
{
	id->type->expr=node;
	return id;
}

A_ID* setDeclaratorInit(A_ID* const id, A_NODE* const initNode)
{
	id->init=initNode;
	return id;
}

A_ID* getIdentifierDeclared(const char* const s)
{
	A_ID* id=findPrevIdUseName_null(s);
	if(id==NULL)
	{
		syntexError(13,s);
	}
	return id;
}
A_ID* findPrevIdUseName_null(const char* const s)
{
        A_ID* prev=current_id;
        while(prev)
        {
                char* prevName_null = prev->name;
                if(prevName_null && strcmp(prevName_null,s)==0)
                {
                        return prev;
                }
                prev=prev->prev;
        }
        return NULL;
}

void setLinkDeclaratorSepecifier(A_ID* const id, const A_SPECIFIER* const specifier)
{
	A_ID* linkId_null=id->link;
	while(linkId_null)
	{
		setDeclaratorElementType( linkId_null, specifier->type);
		linkId_null->specifier = specifier->stor;
		linkId_null=linkId_null->link;
	}

}

void setLinkStructDeclaratorType(A_ID* const id, const A_TYPE* const type)
{
	A_ID* linkId_null=id->link;
        while(linkId_null)
        {
		setDeclaratorElementType( linkId_null, type);
                linkId_null=linkId_null->link;
        }
}

A_ID* setDeclaratorTypeAndKind(A_ID* const id, A_TYPE* const type,const ID_KIND kind)
{
	A_ID* result;
	result=setDeclaratorElementType(id,type);
	result=setDeclaratorKind(result,kind);
	return result;
}


void initIdArrayIndex()
{
	if(idArrayIndex_lex==idArrayIndex)
	{
		idArrayIndex_lex=0;
		idArrayIndex=0;
	}
}

