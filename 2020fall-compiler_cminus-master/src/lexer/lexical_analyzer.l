%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "lexical_analyzer.h"

int lines;
int pos_start;
int pos_end;

/*****************声明和选项设置  end*****************/

%}
 

%%

 /******************TODO*********************/
 /****请在此补全所有flex的模式与动作  start******/
 //STUDENT TO DO
"+" { return ADD;}
"-" { return SUB;}
"*" { return MUL;}
"/" { return DIV;}
"<" { return LT;}
"<=" { return LTE;}
">" { return GT;}
">=" { return GTE;}
"==" { return EQ;}
"!=" { return NEQ;}
"=" { return ASSIN;}
";" { return SEMICOLON;}
"," { return COMMA;}
"(" { return LPARENTHESE;}
")" { return RPARENTHESE;}
"[" { return LBRACKET;}
"]" { return RBRACKET;}
"{" { return LBRACE;}
"}" { return RBRACE;}
"else" { return ELSE;}
"if" { return IF;}
"int" { return INT;}
"float" { return FLOAT;}
"return" { return RETURN;}
"void" { return VOID;}
"while" { return WHILE;}
[a-zA-Z]+ { return IDENTIFIER;}
[0-9]+ { return INTEGER;}
([0-9]+\.)|([0-9]*\.[0-9]+) { return FLOATPOINT;}
"[]" { return ARRAY;}
[a-zA-Z] { return LETTER;}
"\n" { return EOL;}
"/*"([^*]|(\*)*[^\*/])*(\*)*"*/" { return COMMENT;}
" "|"\t" { return BLANK;}
"." { return ERROR;}


 /****请在此补全所有flex的模式与动作  end******/
%%
/****************C代码 start*************/

/// \brief analysize a *.cminus file
///
/// \param input_file, 需要分析的文件路径
/// \param token stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

void analyzer(char* input_file, Token_Node* token_stream){
    lines = 1;
    pos_start = 1;
    pos_end = 1;
    if(!(yyin = fopen(input_file,"r"))){
        printf("[ERR] No input file\n");
        exit(1);
    }
    printf("[START]: Read from: %s\n", input_file);

    int token,i,comlen;
    int index = 0;

    while(token = yylex()){
        pos_start = pos_end;
        switch(token){
            case COMMENT:
                //STUDENT TO DO
                comlen = strlen(yytext);
                for(i = 0;i < comlen;i++){
                    if(yytext[i] == '\n'){  
                        lines ++;
                        pos_end = 0;
                    }
                    pos_end ++;
                }
                break;
            case BLANK:
                //STUDENT TO DO
                pos_end ++;
                break;
            case EOL:
                //STUDENT TO DO
                lines ++;
                pos_end = 1;
                break;
            case ERROR:
                printf("[ERR]: unable to analysize %s at %d line, from %d to %d\n", yytext, lines, pos_start, pos_end);
            default :
                if (token == ERROR){
                    sprintf(token_stream[index].text, "[ERR]: unable to analysize %s at %d line, from %d to %d", yytext, lines, pos_start, pos_end);
                } else {
                    strcpy(token_stream[index].text, yytext);
                }
                pos_end += strlen(yytext);
                token_stream[index].token = token;
                token_stream[index].lines = lines;
                token_stream[index].pos_start = pos_start;
                token_stream[index].pos_end = pos_end;
                index++;
                if (index >= MAX_NUM_TOKEN_NODE){
                    printf("%s has too many tokens (> %d)", input_file, MAX_NUM_TOKEN_NODE);
                    exit(1);
                }
        }
    }
    printf("[END]: Analysis completed.\n");
    return;
}



/****************C代码 end*************/
