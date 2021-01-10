#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <vector>
#include <string>
#include "Stuff.h"
#include "IR.h"

using namespace std;

enum IdKind {
    CONST, VAR, FUNC
};
enum IdType {
    VOID, CHAR, INT
};

class CommonSym {
public:
    string name;
    IdKind kind;
    IdType type;
    CommonSym(string name1, IdKind kind1, IdType type1);
};
class VarSym : public CommonSym {
public:
    int dim;
    int offset;
    int size1;
    VarSym(string name1, IdType type1, int offset1, int asize1=0);
};
class ConstSym : public CommonSym {
public:
    int value;
    ConstSym(string name1, IdType type1, int value1);
};
class FuncSym : public CommonSym {
public:
    bool ret;
    vector<VarSym> paras;
    FuncSym(string name1, IdType type1, bool ret1);
};

void insertId(CommonSym* sym);
void insertFunc(FuncSym sym);
void insertParasInFunc();
void insertTmpParas(VarSym sym, bool isValue);
void setGlobalIdTable();
void clrLocalIdTable();
void clrTmpParas();

IdType getIdType(string name);
IdKind getIdKind(string name);
IRItem* getIRItem(string name);
IdType getFuncType(string name);

bool isRetFunc(string name);

bool checkDupDefId(string name);
bool checkDupDefFuncId(string name);
bool checkUndefId(string name);
bool checkUndefFuncId(string name);
int checkUnmatchedPara(string name, vector<IdType>);

#endif
