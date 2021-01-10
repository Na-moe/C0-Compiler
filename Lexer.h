#ifndef __LEXER_H__
#define __LEXER_H__

#include <iostream>
#include <map>
#include <list>
#include <queue>
#include "ErrorHandler.h"
#include "Stuff.h"

using namespace std;

enum tCode {
    IDENFR = 1,
    INTCON, CHARCON, STRCON,
    CONSTTK, INTTK, CHARTK,
    VOIDTK, MAINTK,
    IFTK, ELSETK,
    SWITCHTK, CASETK, DEFAULTTK,
    WHILETK, FORTK,
    SCANFTK, PRINTFTK,
    RETURNTK,
    PLUS, MINU, MULT, DIV,
    LSS, LEQ, GRE, GEQ, EQL, NEQ,
    COLON, ASSIGN, SEMICN, COMMA,
    LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
};

struct Symbol {
    tCode type;
    string token;
    int lineNo;
};

extern list<string> outputList;
extern queue<Symbol> symQueue;
extern Symbol lastSym;

Symbol preGetSym(ifstream &fin);
Symbol getSym(ifstream &fin);
void pushSym(Symbol sym);

#endif