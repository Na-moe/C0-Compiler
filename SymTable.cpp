#include <iostream>
#include "SymTable.h"

CommonSym::CommonSym(string name1, IdKind kind1, IdType type1) {
    name = name1;
    kind = kind1;
    type = type1;
}
VarSym::VarSym(string name1, IdType type1, int offset1, int asize1) : CommonSym(name1, VAR, type1) {
    offset = offset1;
    size1 = asize1;
}
ConstSym::ConstSym(string name1, IdType type1, int value1) : CommonSym(name1, CONST, type1) {
    value = value1;
}
FuncSym::FuncSym(string name1, IdType type1, bool ret1) : CommonSym(name1, FUNC, type1) {
    ret = ret1;
}

vector<FuncSym> funcTable;
vector<VarSym> tmpParas;
vector<CommonSym*> localIdTable;
vector<CommonSym*> globalIdTable;

void insertId(CommonSym* sym) {
    localIdTable.push_back(sym);
}
void insertFunc(FuncSym sym) {
    funcTable.push_back(sym);
}
void insertParasInFunc() {
    funcTable.back().paras = move(tmpParas);
}
void insertTmpParas(VarSym sym, bool isValue) {
    tmpParas.push_back(sym);
    if (!isValue) {
        localIdTable.push_back(new VarSym(sym.name, sym.type, sym.offset));
    }
}
void setGlobalIdTable() {
    globalIdTable = move(localIdTable);
}
void clrLocalIdTable() {
    localIdTable.clear();
}
void clrTmpParas() {
    tmpParas.clear();
}

IdType getIdType(string name) {
    for (auto & i : localIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            return i->type;
        }
    }
    for (auto & i : globalIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            return i->type;
        }
    }
    return VOID;
}

IdKind getIdKind(string name) {
    for (auto & i : localIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            return i->kind;
        }
    }
    for (auto & i : globalIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            return i->kind;
        }
    }
    return FUNC;
}

IRItem* getIRItem(string name) {
    IRItemKind kind;
    IRItemType type;
    int value;
    int size1;
    for (auto & i : localIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            type = i->type==INT?Int:Char;
            if (i->kind == CONST) {
                kind = Const;
                value = ((ConstSym *)i)->value;
                size1 = 0;
            } else {
                kind = LocVar;
                value = ((VarSym *)i)->offset;
                size1 = ((VarSym *)i)->size1;
            }
            return new IRItem(name, kind, type, value, size1);
        }
    }
    for (auto & i : globalIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            type = i->type==INT?Int:Char;
            if (i->kind == CONST) {
                kind = Const;
                value = ((ConstSym *)i)->value;
                size1 = 0;
            } else {
                kind = GlbVar;
                value = ((VarSym *)i)->offset;
                size1 = ((VarSym *)i)->size1;
            }
        }
    }
    return new IRItem(name, kind, type, value, size1);
}

IdType getFuncType(string name) {
    for (auto & i : funcTable) {
        if (strToLower(i.name) == strToLower(name)) {
            return i.type;
        }
    }
    return VOID;
}

bool isRetFunc(string name) {
    for (auto & i : funcTable) {
        if (strToLower(i.name) == strToLower(name)) {
            return i.ret;
        }
    }
    return false;
}

bool checkDupDefId(string name) {
    for (auto & i : localIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            return true;
        }
    }
    return false;
}

bool checkDupDefFuncId(string name) {
    for (auto & i : globalIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            return true;
        }
    }
    for (auto & i : funcTable) {
        if (strToLower(i.name) == strToLower(name)) {
            return true;
        }
    }
    return false;
}

bool checkUndefId(string name) {
    for (auto & i : localIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            return false;
        }
    }
    for (auto & i : globalIdTable) {
        if (strToLower(i->name) == strToLower(name)) {
            return false;
        }
    }
    return true;
}

bool checkUndefFuncId(string name) {
    for (auto & i : funcTable) {
        if (strToLower(i.name) == strToLower(name)) {
            return false;
        }
    }
    return true;
}

int checkUnmatchedPara(string name, vector<IdType> localParaTable) {
    int i;
    for (i = 0; i < funcTable.size(); ++i) {
        if (strToLower(funcTable[i].name) == strToLower(name)) {
            break;
        }
    }
    if (funcTable[i].paras.size() != localParaTable.size()) {
        return 1;
    }
    for (int j = 0; j < funcTable[i].paras.size(); ++j) {
        if (funcTable[i].paras[j].type != localParaTable[j]) {
            return 2;
        }
    }
    return 0;
}