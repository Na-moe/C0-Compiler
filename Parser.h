#ifndef __PARSER_H__
#define __PARSER_H__

#include "Lexer.h"
#include "SymTable.h"
#include "IR.h"

void parse();
void program();
void constDeclaration();
void constDefination();
void integer();
int unsignedInteger();
void preUnsignedInteger();
void varDeclaration();
void varDefination();
void varDefWithoutInit();
void varDefWithInit();
int constant();
void funcWithRet();
void funcWithoutRet();
bool declarationHeader();
void paraList();
void compoundStatement();
void statements();
void statement();
void loopStatement();
void condition();
int expression();
void genExpIR(IROp op);
int term();
int factor();
void stepLength();
void ifStatement();
int funcStatement();
vector<IdType> valueParaList();
void assignStatement();
void genAssignIR(IROp op);
void scanfStatement();
void printfStatement();
void parseStr();
void switchStatement();
void switchList(string switchNo);
void switchSubStatement(string switchNo, int switchCaseNo);
void switchDefault();
void returnStatement();
void mainFunc();

#endif