#include <iostream>
#include "ErrorHandler.h"

using namespace std;

extern ofstream ferr;

void errOutput(char type, int lineNo) {
    ferr << string().append(to_string(lineNo)).append(" ").append(1, type) << endl;
}

void errSym(int lineNo) {
    errOutput('a', lineNo);
}

bool errDupDefId(string name, int lineNo) {
    if (checkDupDefId(name)) {
        errOutput('b', lineNo);
        return true;
    }
    return false;
}

bool errDupDefFuncId(string name, int lineNo) {
    if (checkDupDefFuncId(name)) {
        errOutput('b', lineNo);
        return true;
    }
    return false;
}

bool errUndefId(string name, int lineNo) {
    if (checkUndefId(name)) {
        errOutput('c', lineNo);
        return true;
    }
    return false;
}

bool errUndefFuncId(string name, int lineNo) {
    if (checkUndefFuncId(name)) {
        errOutput('c', lineNo);
        return true;
    }
    return false;
}

bool errUnmatchedPara(string name, vector<IdType> localParaTable, int lineNo) {
    int errType = checkUnmatchedPara(name, localParaTable);
    switch(errType) {
        case 1: {
            errOutput('d', lineNo);
            return true;
        }
        case 2: {
            errOutput('e', lineNo);
            return true;
        }
        default: return false;
    }
}

void errIllegalTypeInCond(int lineNo) {
    errOutput('f', lineNo);
}

void errVoidFuncRet(int lineNo) {
    errOutput('g', lineNo);
}

void errRetFuncUnmatch(int lineNo) {
    errOutput('h', lineNo);
}

void errArrayIndexNotInt(int lineNo) {
    errOutput('i', lineNo);
}

void errChangeConst(string name, int lineNo) {
    if (getIdKind(name) == CONST) {
        errOutput('j', lineNo);
    }
}

void errLackSemicolon(int lineNo) {
    errOutput('k', lineNo);
}

void errLackRParent(int lineNo) {
    errOutput('l', lineNo);
}

void errLackRBrack(int lineNo) {
    errOutput('m', lineNo);
}

void errUnmatchedCountInArrayInit(int lineNo) {
    errOutput('n', lineNo);
}

void errUnmatchedConstType(int lineNo) {
    errOutput('o', lineNo);
}

void errLackDefault(int lineNo) {
    errOutput('p', lineNo);
}