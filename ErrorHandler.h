#ifndef __ERRORHANDLER_H__
#define __ERRORHANDLER_H__

#include <fstream>
#include <iostream>
#include "SymTable.h"

using namespace std;

void errSym(int lineNo);
bool errDupDefId(string name, int lineNo);
bool errDupDefFuncId(string name, int lineNo);
bool errUndefId(string name, int lineNo);
bool errUndefFuncId(string name, int lineNo);
bool errUnmatchedPara(string name, vector<IdType> localParaTable, int lineNo);
void errIllegalTypeInCond(int lineNo);
void errVoidFuncRet(int lineNo);
void errRetFuncUnmatch(int lineNo);
void errArrayIndexNotInt(int lineNo);
void errChangeConst(string name, int lineNo);
void errLackSemicolon(int lineNo);
void errLackRParent(int lineNo);
void errLackRBrack(int lineNo);
void errUnmatchedCountInArrayInit(int lineNo);
void errUnmatchedConstType(int lineNo);
void errLackDefault(int lineNo);

#endif
