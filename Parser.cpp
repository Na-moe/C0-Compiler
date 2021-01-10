#include <fstream>
#include <set>
#include "Parser.h"

#define OUTPUT(str) do {  string tmp = outputList.back();\
                            outputList.pop_back();\
                            outputList.emplace_back((str));\
                            outputList.push_back(tmp);}\
                            while (0)

using namespace std;

extern ifstream fin;
extern ofstream fout;

extern Symbol sym;

IdType funcType = VOID;
bool hasRet = false;
IRItem* retItem;
bool isMain = false;

IdType switchType = VOID;
bool isGlb = true;
int varOffset = 0;
int nowIntConstant = 0;
string nowCharConstant;
string nowStringName;//parseStr

vector<IRItem*> expStack;
IRItem* popExpStack() {
    IRItem* item = expStack.back();
    expStack.pop_back();
    return item;
}
void pushExpStack(IRItem* item) {
    expStack.push_back(item);
}

string genGlbStrName() {
    static int glbStrNo = 0;
    return "string" + to_string(glbStrNo++);
}
string genTmpVarName() {
    static int tmpVarNo = 0;
    return "tmp" + to_string(tmpVarNo++);
}
string genIfLabel() {
    static int ifNo = 0;
    return to_string(ifNo++);
}
string genWhileLabel() {
    static int whileNo = 0;
    return to_string(whileNo++);
}
string genForLabel() {
    static int forNo = 0;
    return to_string(forNo++);
}
string genSwitchLabel() {
    static int switchNo = 0;
    return to_string(switchNo++);
}

IROp branchType;
map<tCode, IROp> conditionMap = {
        {LSS, Bge},
        {LEQ, Bgt},
        {GRE, Ble},
        {GEQ, Blt},
        {EQL, Bne},
        {NEQ, Beq}
};

void parse() {
    sym = getSym(fin);
    program();
    /*for (string str: outputList) {
        fout << str << endl;
    }*/
}

void program() {
    if (sym.type == CONSTTK) {
        constDeclaration();
    }
    if (sym.type == INTTK || sym.type == CHARTK) {
        varDeclaration();
    }
    setGlobalIdTable();
    isGlb = false;
    varOffset = 0;
    while (sym.type == INTTK || sym.type == CHARTK || sym.type == VOIDTK) {
        if (sym.type != VOIDTK) {
            funcWithRet();
        } else {
            pushSym(sym);
            sym = preGetSym(fin);
            pushSym(sym);
            if (sym.type == IDENFR) {
                sym = getSym(fin);
                funcWithoutRet();
            } else {
                sym = getSym(fin);
                break;
            }
        }
        funcType = VOID;
        hasRet = false;
        varOffset = 0;
        clrLocalIdTable();
    }
    mainFunc();
    insertIRTable(new IREntry(Label, new IRItem("main_end", Const, String)));
    outputList.emplace_back("<程序>");
}

void constDeclaration() {
    if (sym.type != CONSTTK) {
        /*error*/
    }
    while (sym.type == CONSTTK) {
        sym = getSym(fin);
        constDefination();
        if (sym.type != SEMICN) {
            errLackSemicolon(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
    }
    OUTPUT("<常量说明>");
    //fout << "<常量说明>" << endl;
}

void constDefination() {
    int errLineNo;
    string name;
    IdType type;
    int value;
    if (sym.type != INTTK && sym.type != CHARTK) {
        /*error*/
    }
    if (sym.type == INTTK) {
        type = INT;
        if ((sym = getSym(fin)).type != IDENFR) {
            /*error*/
        }
        name = sym.token;
        errLineNo = sym.lineNo;
        if ((sym = getSym(fin)).type != ASSIGN) {
            /*error*/
        }
        sym = getSym(fin);
        integer();
        if (!errDupDefId(name, errLineNo)) {
            insertId(new ConstSym(name, type, nowIntConstant));
            //insertIRTable(new IREntry(ConstIntDef, new IRItem(name, Const, Int, value)));
        }
        while (sym.type == COMMA) {
            type = INT;
            if ((sym = getSym(fin)).type != IDENFR) {
                /*error*/
            }
            name = sym.token;
            errLineNo = sym.lineNo;
            if ((sym = getSym(fin)).type != ASSIGN) {
                /*error*/
            }
            sym = getSym(fin);
            integer();
            if (!errDupDefId(name, errLineNo)) {
                insertId(new ConstSym(name, type, nowIntConstant));
                //insertIRTable(new IREntry(ConstIntDef, new IRItem(name, Const, Int, value)));
            }
        }
    } else {
        type = CHAR;
        if ((sym = getSym(fin)).type != IDENFR) {
            /*error*/
        }
        name = sym.token;
        errLineNo = sym.lineNo;
        if ((sym = getSym(fin)).type != ASSIGN) {
            /*error*/
        }
        if ((sym = getSym(fin)).type != CHARCON) {
            /*error*/
        }
        value = (int) sym.token[0];
        if (!errDupDefId(name, errLineNo)) {
            insertId(new ConstSym(name, type, value));
            //insertIRTable(new IREntry(ConstCharDef, new IRItem(name, Const, Char, value)));
        }
        while ((sym = getSym(fin)).type == COMMA) {
            type = CHAR;
            if ((sym = getSym(fin)).type != IDENFR) {
                /*error*/
            }
            name = sym.token;
            errLineNo = sym.lineNo;
            if ((sym = getSym(fin)).type != ASSIGN) {
                /*error*/
            }
            if ((sym = getSym(fin)).type != CHARCON) {
                /*error*/
            }
            value = (int) sym.token[0];
            if (!errDupDefId(name, errLineNo)) {
                insertId(new ConstSym(name, type, value));
                //insertIRTable(new IREntry(ConstCharDef, new IRItem(name, Const, Char, value)));
            }
        }
    }
    OUTPUT("<常量定义>");
    //fout << "<常量定义>" << endl;
}

void integer() {
    if (sym.type == PLUS || sym.type == MINU) {
        tCode posOrNeg = sym.type;
        sym = getSym(fin);
        unsignedInteger();
        if (posOrNeg == MINU) {
            nowIntConstant = -nowIntConstant;
        }
    } else if (sym.type == INTCON) {
        unsignedInteger();
    } else {
        sym = getSym(fin);
        /*error*/
    }
    OUTPUT("<整数>");
    //fout << "<整数>" << endl;
}

int unsignedInteger() {
    int value = 18373080;
    if (sym.type != INTCON) {
        /*error*/
    } else {
        value = stoi(sym.token);
    }
    bool output = symQueue.empty();
    sym = getSym(fin);
    if (output) {
        OUTPUT("<无符号整数>");
    }
    nowIntConstant = value;
    return value;
    //fout << "<无符号整数>" << endl;
}

void preUnsignedInteger() {
    if (sym.type != INTCON) {
        /*error*/
    }
    sym = preGetSym(fin);
    OUTPUT("<无符号整数>");
}

void varDeclaration() {
    bool gotVarDec = false;
    while (sym.type == INTTK || sym.type == CHARTK) {
        pushSym(sym);
        if ((sym = preGetSym(fin)).type != IDENFR) {
            /*error*/
        }
        pushSym(sym);
        sym = preGetSym(fin);
        pushSym(sym);
        if (sym.type != LPARENT) {
            gotVarDec = true;
            sym = getSym(fin);
            varDefination();
        } else {
            sym = getSym(fin);
            if (gotVarDec) {
                string tmp0 = outputList.back();
                outputList.pop_back();
                string tmp1 = outputList.back();
                outputList.pop_back();
                string tmp2 = outputList.back();
                outputList.pop_back();
                outputList.emplace_back("<变量说明>");
                outputList.push_back(tmp2);
                outputList.push_back(tmp1);
                outputList.push_back(tmp0);
            }
            return;
        }
        if (sym.type != SEMICN) {
            errLackSemicolon(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
    }
    OUTPUT("<变量说明>");
    //fout << "<变量说明>" << endl;
}

void varDefination() {
    if (sym.type != INTTK && sym.type != CHARTK) {
        /*error*/
    }
    pushSym(sym);
    if ((sym = getSym(fin)).type != IDENFR) {
        /*error*/
    }
    pushSym(sym);
    sym = getSym(fin);
    if (sym.type == LBRACK) {
        pushSym(sym);
        sym = preGetSym(fin);
        pushSym(sym);
        preUnsignedInteger();
        if (sym.type != RBRACK) {
        } else {
            pushSym(sym);
            sym = preGetSym(fin);
        }
        if (sym.type == LBRACK) {
            pushSym(sym);
            sym = preGetSym(fin);
            pushSym(sym);
            preUnsignedInteger();
            if (sym.type != RBRACK) {
            } else {
                pushSym(sym);
                sym = preGetSym(fin);
            }
        }
    }
    if (sym.type != ASSIGN) {
        pushSym(sym);
        sym = getSym(fin);
        varDefWithoutInit();
    } else {
        pushSym(sym);
        sym = getSym(fin);
        varDefWithInit();
    }
    OUTPUT("<变量定义>");
    //fout << "<变量定义>" << endl;
}

void varDefWithoutInit() {
    int errLineNo;
    string name;
    IdType type;
    int dim = 0;
    int size = 1;
    int size1 = 0;
    if (sym.type != INTTK && sym.type != CHARTK) {
        /*error*/
    }
    type = (sym.type == INTTK) ? INT : CHAR;
    if ((sym = getSym(fin)).type != IDENFR) {
        /*error*/
    }
    name = sym.token;
    errLineNo = sym.lineNo;
    sym = getSym(fin);
    if (sym.type == LBRACK) {
        sym = getSym(fin);
        size *= unsignedInteger();
        if (sym.type != RBRACK) {
            errLackRBrack(sym.lineNo);
        } else {
            sym = getSym(fin);
        }
        dim = 1;
        if (sym.type == LBRACK) {
            sym = getSym(fin);
            size1 = unsignedInteger();
            size *= size1;
            if (sym.type != RBRACK) {
                errLackRBrack(sym.lineNo);
            } else {
                sym = getSym(fin);
            }
            dim = 2;
        }
    }
    if (!errDupDefId(name, errLineNo)) {
        varOffset += size * 4;
        insertId(new VarSym(name, type, varOffset, size1));
        insertIRTable(new IREntry(type==INT?VarIntDef:VarCharDef, new IRItem(name, isGlb?GlbVar:LocVar, type==INT?Int:Char, varOffset, size1)));
    }
    while (sym.type == COMMA) {
        dim = 0;
        size = 1;
        size1 = 0;
        if ((sym = getSym(fin)).type != IDENFR) {
            /*error*/
        }
        name = sym.token;
        errLineNo = sym.lineNo;
        sym = getSym(fin);
        if (sym.type == LBRACK) {
            sym = getSym(fin);
            size *= unsignedInteger();
            if (sym.type != RBRACK) {
                errLackRBrack(sym.lineNo);
            } else {
                sym = getSym(fin);
            }
            dim = 1;
            if (sym.type == LBRACK) {
                sym = getSym(fin);
                size1 = unsignedInteger();
                size *= size1;
                if (sym.type != RBRACK) {
                    errLackRBrack(sym.lineNo);
                } else {
                    sym = getSym(fin);
                }
                dim = 2;
            }
        }
        if (!errDupDefId(name, errLineNo)) {
            varOffset += size * 4;
            insertId(new VarSym(name, type, varOffset, size1));
            insertIRTable(new IREntry(type==INT?VarIntDef:VarCharDef, new IRItem(name, isGlb?GlbVar:LocVar, type==INT?Int:Char, varOffset, size1)));
        }
    }
    OUTPUT("<变量定义无初始化>");
    //fout << "<变量定义无初始化>" << endl;
}

void varDefWithInit() {
    int errLineNo;
    string name;
    IdType type;
    IdType varType;
    int dim = 0;
    int size = 1;
    int size1 = 0;
    if (sym.type != INTTK && sym.type != CHARTK) {
        /*error*/
    }
    type = (sym.type == INTTK) ? INT : CHAR;
    if ((sym = getSym(fin)).type != IDENFR) {
        /*error*/
    }
    name = sym.token;
    errLineNo = sym.lineNo;
    sym = getSym(fin);
    if (sym.type == ASSIGN) {
        sym = getSym(fin);
        varType = (constant() == 1) ? INT : CHAR;
        if (type != varType) {
            errUnmatchedConstType(sym.lineNo);
        }
        varOffset += size * 4;
        auto item = new IRItem(name, isGlb?GlbVar:LocVar, type==INT?Int:Char, varOffset);
        insertIRTable(new IREntry(type==INT?VarIntDef:VarCharDef, item));
        insertIRTable(new IREntry(Assign, type==INT?
                                new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant):
                                new IRItem(nowCharConstant, Const, Char, nowCharConstant[0]),
                                item));
    } else if (sym.type == LBRACK) {
        sym = getSym(fin);
        int arraySize0 = unsignedInteger();
        size *= arraySize0;
        int arraySizeInit0 = 0;
        if (sym.type != RBRACK) {
            errLackRBrack(sym.lineNo);
        } else {
            sym = getSym(fin);
        }
        dim = 1;
        if (sym.type == ASSIGN) {
            if ((sym = getSym(fin)).type != LBRACE) {
                /*error*/
            }
            sym = getSym(fin);
            varType = (constant() == 1) ? INT : CHAR;
            if (type != varType) {
                errUnmatchedConstType(sym.lineNo);
            }
            varOffset += size * 4;
            auto item = new IRItem(name, isGlb?GlbVar:LocVar, type==INT?Int:Char, varOffset);
            insertIRTable(new IREntry(type==INT?VarIntDef:VarCharDef, item));
            insertIRTable(new IREntry(ArrayStore,
                                      type==INT?
                                      new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant):
                                      new IRItem(nowCharConstant, Const, Char, nowCharConstant[0]),
                                      new IRItem(to_string(arraySizeInit0), Const, Int, arraySizeInit0),
                                      item));
            arraySizeInit0++;
            while (sym.type == COMMA) {
                sym = getSym(fin);
                varType = (constant() == 1) ? INT : CHAR;
                if (type != varType) {
                    errUnmatchedConstType(sym.lineNo);
                }
                insertIRTable(new IREntry(ArrayStore,
                                          type==INT?
                                          new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant):
                                          new IRItem(nowCharConstant, Const, Char, nowCharConstant[0]),
                                          new IRItem(to_string(arraySizeInit0), Const, Int, arraySizeInit0),
                                          item));
                arraySizeInit0++;
            }
            if (sym.type != RBRACE) {
                /*error*/
            }
            if (arraySize0 != arraySizeInit0) {
                errUnmatchedCountInArrayInit(sym.lineNo);
            }
            sym = getSym(fin);
        } else if (sym.type == LBRACK) {
            sym = getSym(fin);
            int arraySize1 = unsignedInteger();
            size1 = arraySize1;
            size *= arraySize1;
            int arraySizeInit1 = 0;
            if (sym.type != RBRACK) {
                errLackRBrack(sym.lineNo);
            } else {
                sym = getSym(fin);
            }
            dim = 2;
            if (sym.type == ASSIGN) {
                if ((sym = getSym(fin)).type != LBRACE) {
                    /*error*/
                }
                if ((sym = getSym(fin)).type != LBRACE) {
                    /*error*/
                }
                sym = getSym(fin);
                varType = (constant() == 1) ? INT : CHAR;
                if (type != varType) {
                    errUnmatchedConstType(sym.lineNo);
                }
                varOffset += size * 4;
                auto item = new IRItem(name, isGlb?GlbVar:LocVar, type==INT?Int:Char, varOffset, arraySize1);
                insertIRTable(new IREntry(type==INT?VarIntDef:VarCharDef, item));
                insertIRTable(new IREntry(ArrayStore,
                                          type==INT?
                                          new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant):
                                          new IRItem(nowCharConstant, Const, Char, nowCharConstant[0]),
                                          new IRItem(to_string(arraySizeInit0*arraySize1+arraySizeInit1), Const, Int, arraySizeInit0*arraySize1+arraySizeInit1),
                                          item));
                arraySizeInit1++;
                while (sym.type == COMMA) {
                    sym = getSym(fin);
                    varType = (constant() == 1) ? INT : CHAR;
                    if (type != varType) {
                        errUnmatchedConstType(sym.lineNo);
                    }
                    insertIRTable(new IREntry(ArrayStore,
                                              type==INT?
                                              new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant):
                                              new IRItem(nowCharConstant, Const, Char, nowCharConstant[0]),
                                              new IRItem(to_string(arraySizeInit0*arraySize1+arraySizeInit1), Const, Int, arraySizeInit0*arraySize1+arraySizeInit1),
                                              item));
                    arraySizeInit1++;
                }
                if (sym.type != RBRACE) {
                    /*error*/
                }
                if (arraySize1 != arraySizeInit1) {
                    errUnmatchedCountInArrayInit(sym.lineNo);
                }
                arraySizeInit0++;
                sym = getSym(fin);
                while (sym.type == COMMA) {
                    arraySizeInit1 = 0;
                    if ((sym = getSym(fin)).type != LBRACE) {
                        /*error*/
                    }
                    sym = getSym(fin);
                    varType = (constant() == 1) ? INT : CHAR;
                    if (type != varType) {
                        errUnmatchedConstType(sym.lineNo);
                    }
                    insertIRTable(new IREntry(ArrayStore,
                                              type==INT?
                                                new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant):
                                                new IRItem(nowCharConstant, Const, Char, nowCharConstant[0]),
                                              new IRItem(to_string(arraySizeInit0*arraySize1+arraySizeInit1), Const, Int, arraySizeInit0*arraySize1+arraySizeInit1),
                                              item));
                    arraySizeInit1++;
                    while (sym.type == COMMA) {
                        sym = getSym(fin);
                        varType = (constant() == 1) ? INT : CHAR;
                        if (type != varType) {
                            errUnmatchedConstType(sym.lineNo);
                        }
                        insertIRTable(new IREntry(ArrayStore,
                                                  type==INT?
                                                    new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant):
                                                    +new IRItem(nowCharConstant, Const, Char, nowCharConstant[0]),
                                                  new IRItem(to_string(arraySizeInit0*arraySize1+arraySizeInit1), Const, Int, arraySizeInit0*arraySize1+arraySizeInit1),
                                                  item));
                        arraySizeInit1++;
                    }
                    if (sym.type != RBRACE) {
                        /*error*/
                    }
                    if (arraySize1 != arraySizeInit1) {
                        errUnmatchedCountInArrayInit(sym.lineNo);
                    }
                    arraySizeInit0++;
                    sym = getSym(fin);
                }
                if (sym.type != RBRACE) {
                    /*error*/
                }
                if (arraySize0 != arraySizeInit0) {
                    errUnmatchedCountInArrayInit(sym.lineNo);
                }
                sym = getSym(fin);
            } else {
                /*error*/
            }
        } else {
            /*error*/
        }
    } else {
        /*error*/
    }
    if (!errDupDefId(name, errLineNo)) {
        insertId(new VarSym(name, type, varOffset, size1));
    }
    OUTPUT("<变量定义及初始化>");
    //fout << "<变量定义及初始化>" << endl;
}

int constant() {
    if (sym.type == PLUS || sym.type == MINU || sym.type == INTCON) {
        integer();
        return 1;
    } else if (sym.type == CHARCON) {
        nowCharConstant = sym.token;
        sym = getSym(fin);
        return 2;
    } else {
        /*error*/
    }
    OUTPUT("<常量>");
    return 0;
    //fout << "<常量>" << endl;
}

void funcWithRet() {
    bool dupDef = declarationHeader();
    if (sym.type != LPARENT) {
        /*error*/
    }
    sym = getSym(fin);
    paraList();
    if (!dupDef) {
        insertParasInFunc();
    } else {
        clrTmpParas();
    }
    if (sym.type != RPARENT) {
        errLackRParent(lastSym.lineNo);
    } else {
        sym = getSym(fin);
    }
    if (sym.type != LBRACE) {
        /*error*/
    }
    sym = getSym(fin);
    compoundStatement();
    if (sym.type != RBRACE) {
        /*error*/
    }
    if (!hasRet) {
        errRetFuncUnmatch(sym.lineNo);
    }
    sym = getSym(fin);
    OUTPUT("<有返回值函数定义>");
    //fout << "<有返回值函数定义>" << endl;
}

void funcWithoutRet() {
    funcType = VOID;
    if (sym.type != VOIDTK) {
        /*error*/
    }
    if ((sym = getSym(fin)).type != IDENFR) {
        /*error*/
    }
    bool dupDef = errDupDefFuncId(sym.token, sym.lineNo);
    if (!dupDef) {
        insertFunc(*new FuncSym(sym.token, VOID, false));
    }
    insertIRTable(new IREntry(FuncDef, new IRItem("func_" + strToLower(sym.token), Func, Void)));
    varOffset += 8;
    if ((sym = getSym(fin)).type != LPARENT) {
        /*error*/
    }
    sym = getSym(fin);
    paraList();
    if (!dupDef) {
        insertParasInFunc();
    } else {
        clrTmpParas();
    }
    if (sym.type != RPARENT) {
        errLackRParent(lastSym.lineNo);
    } else {
        sym = getSym(fin);
    }
    if (sym.type != LBRACE) {
        /*error*/
    }
    sym = getSym(fin);
    compoundStatement();
    if (sym.type != RBRACE) {
        /*error*/
    }
    sym = getSym(fin);
    insertIRTable(new IREntry(FuncRet, new IRItem("ret", Func, Void)));
    OUTPUT("<无返回值函数定义>");
    //fout << "<无返回值函数定义>" << endl;
}

bool declarationHeader() {
    IdType type;
    if (sym.type != INTTK && sym.type != CHARTK) {
        /*error*/
    }
    type = (sym.type == INTTK) ? INT : CHAR;
    funcType = type;
    if ((sym = getSym(fin)).type != IDENFR) {
        /*error*/
    }
    insertIRTable(new IREntry(FuncDef, new IRItem("func_" + strToLower(sym.token), Func, type==INT?Int:Char)));
    varOffset += 8;
    bool dupDef = errDupDefFuncId(sym.token, sym.lineNo);
    if (!dupDef) {
        insertFunc(*new FuncSym(sym.token, type, true));
    }
    sym = getSym(fin);
    OUTPUT("<声明头部>");
    return dupDef;
    //fout << "<声明头部>" << endl;
}

void paraList() {
    IdType type;
    if (sym.type == RPARENT) {
        OUTPUT("<参数表>");
        //fout << "<参数表>" << endl;
        return;
    } else if (sym.type == INTTK || sym.type == CHARTK) {
        type = (sym.type == INTTK) ? INT : CHAR;
        if ((sym = getSym(fin)).type != IDENFR) {
            /*error*/
        }
        if (!errDupDefId(sym.token, sym.lineNo)) {
            varOffset += 4;
            insertTmpParas(*new VarSym(sym.token, type, varOffset), false);
        }
        sym = getSym(fin);
        while (sym.type == COMMA) {
            sym = getSym(fin);
            if (sym.type != INTTK && sym.type != CHARTK) {
                /*error*/
            }
            type = (sym.type == INTTK) ? INT : CHAR;
            if ((sym = getSym(fin)).type != IDENFR) {
                /*error*/
            }
            if (!errDupDefId(sym.token, sym.lineNo)) {
                varOffset += 4;
                insertTmpParas(*new VarSym(sym.token, type, varOffset), false);
            }
            sym = getSym(fin);
        }
    } else {
        /*error*/
    }
    OUTPUT("<参数表>");
    //fout << "<参数表>" << endl;
}

void compoundStatement() {
    if (sym.type == CONSTTK) {
        constDeclaration();
    }
    if (sym.type == INTTK || sym.type == CHARTK) {
        varDeclaration();
    }
    statements();
    OUTPUT("<复合语句>");
    //fout << "<复合语句>" << endl;
}

void statements() {
    if (sym.type == RBRACE) {
        OUTPUT("<语句列>");
        //fout << "<语句列>" << endl;
        return;
    } else {
        while (sym.type == WHILETK || sym.type == FORTK || sym.type == IFTK || sym.type == SWITCHTK
        || sym.type == IDENFR || sym.type == SCANFTK || sym.type == PRINTFTK || sym.type == SEMICN || sym.type == RETURNTK || sym.type == LBRACE) {
            statement();
        }
    }
    OUTPUT("<语句列>");
    //fout << "<语句列>" << endl;
}

void statement() {
    if (sym.type == WHILETK || sym.type == FORTK) {
        loopStatement();
    } else if (sym.type == IFTK) {
        ifStatement();
    } else if (sym.type == IDENFR) {
        pushSym(sym);
        sym = preGetSym(fin);
        pushSym(sym);
        if (sym.type == LPARENT) {
            sym = getSym(fin);
            funcStatement();
            if (sym.type != SEMICN) {
                errLackSemicolon(lastSym.lineNo);
            } else {
                sym = getSym(fin);
            }
        } else if (sym.type == ASSIGN || sym.type == LBRACK) {
            sym = getSym(fin);
            assignStatement();
            if (sym.type != SEMICN) {
                errLackSemicolon(lastSym.lineNo);
            } else {
                sym = getSym(fin);
            }
        } else {
            /*error*/
        }
    } else if (sym.type == SWITCHTK) {
        switchStatement();
    } else if (sym.type == SCANFTK) {
        scanfStatement();
        if (sym.type != SEMICN) {
            errLackSemicolon(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
    } else if (sym.type == PRINTFTK) {
        printfStatement();
        if (sym.type != SEMICN) {
            errLackSemicolon(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
    } else if (sym.type == SEMICN) {
        sym = getSym(fin);
    } else if (sym.type == RETURNTK) {
        returnStatement();
        if (sym.type != SEMICN) {
            errLackSemicolon(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
    } else if (sym.type == LBRACE) {
        sym = getSym(fin);
        statements();
        if (sym.type != RBRACE) {
            /*error*/
        }
        sym = getSym(fin);
    } else {
        errLackSemicolon(lastSym.lineNo);
    }
    OUTPUT("<语句>");
    //fout << "<语句>" << endl;
}

void loopStatement() {
    if (sym.type == WHILETK) {
        string whileNo = genWhileLabel();
        string while_begin = "while_" + whileNo + "_begin";
        insertIRTable(new IREntry(Label, new IRItem(while_begin, Const, String)));
        if ((sym = getSym(fin)).type != LPARENT) {
            /*error*/
        }
        sym = getSym(fin);
        condition();
        string while_end = "while_" + whileNo + "_end";
        IRItem* item2 = popExpStack();
        IRItem* item1 = popExpStack();
        insertIRTable(new IREntry(branchType, item1, item2, new IRItem(while_end, Const, String)));
        if (sym.type != RPARENT) {
            errLackRParent(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
        statement();
        insertIRTable(new IREntry(Jump, new IRItem(while_begin, Const, String)));
        insertIRTable(new IREntry(Label, new IRItem(while_end, Const, String)));
    } else if (sym.type == FORTK) {
        if ((sym = getSym(fin)).type != LPARENT) {
            /*error*/
        }
        if ((sym = getSym(fin)).type != IDENFR) {
            /*error*/
        }
        errUndefId(sym.token, sym.lineNo);
        errChangeConst(sym.token, sym.lineNo);
        IRItem* item = getIRItem(sym.token);
        pushExpStack(item);
        if ((sym = getSym(fin)).type != ASSIGN) {
            /*error*/
        }
        sym = getSym(fin);
        expression();
        genAssignIR(Assign);
        if (sym.type != SEMICN) {
            errLackSemicolon(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
        string forNo = genForLabel();
        string for_begin = "for_" + forNo + "_begin";
        insertIRTable(new IREntry(Label, new IRItem(for_begin, Const, String)));
        condition();
        string for_end = "for_" + forNo + "_end";
        IRItem* for_item2 = popExpStack();
        IRItem* for_item1 = popExpStack();
        insertIRTable(new IREntry(branchType, for_item1, for_item2, new IRItem(for_end, Const, String)));
        if (sym.type != SEMICN) {
            errLackSemicolon(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
        IRItem* item1;
        IRItem* item2;
        IRItem* item3;
        IROp op;
        if (sym.type != IDENFR) {
            /*error*/
        }
        errUndefId(sym.token, sym.lineNo);
        errChangeConst(sym.token, sym.lineNo);
        item1 = getIRItem(sym.token);
        if ((sym = getSym(fin)).type != ASSIGN) {
            /*error*/
        }
        if ((sym = getSym(fin)).type != IDENFR) {
            /*error*/
        }
        item2 = getIRItem(sym.token);
        errUndefId(sym.token, sym.lineNo);
        sym = getSym(fin);
        if (sym.type != PLUS && sym.type != MINU) {
            /*error*/
        }
        op = sym.type==PLUS?Add:Sub;
        sym = getSym(fin);
        stepLength();
        item3 = new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant);
        if (sym.type != RPARENT) {
            errLackRParent(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
        statement();
        pushExpStack(item1);
        pushExpStack(item2);
        pushExpStack(item3);
        genExpIR(op);
        genAssignIR(Assign);
        insertIRTable(new IREntry(Jump, new IRItem(for_begin, Const, String)));
        insertIRTable(new IREntry(Label, new IRItem(for_end, Const, String)));
    } else {
        /*error*/
    }
    OUTPUT("<循环语句>");
    //fout << "<循环语句>" << endl;
}

void condition() {
    if (expression() != 1) {
        errIllegalTypeInCond(lastSym.lineNo);
    }
    if (sym.type != LSS && sym.type != LEQ && sym.type != GRE && sym.type != GEQ && sym.type != NEQ && sym.type != EQL) {
        /*error*/
    }
    branchType = conditionMap[sym.type];
    sym = getSym(fin);
    if (expression() != 1) {
        errIllegalTypeInCond(lastSym.lineNo);
    }
    OUTPUT("<条件>");
    //fout << "<条件>" << endl;
}

int expression() {
    int termCount = 0;
    int termRet;
    IROp op = Add;
    if (sym.type == PLUS || sym.type == MINU) {
        op = sym.type==MINU?Sub:Add;
        pushExpStack(new IRItem("0", Const, Int, 0));
        sym = getSym(fin);
        termCount++;
    }
    termRet = term();
    termCount++;
    if (termCount > 1) {
        genExpIR(op);
    }
    while (sym.type == PLUS || sym.type == MINU) {
        op = sym.type==PLUS?Add:Sub;
        sym = getSym(fin);
        term();
        genExpIR(op);
        termCount++;
    }
    OUTPUT("<表达式>");
    if (termCount > 1) {
        return 1;
    } else {
        return termRet;
    }
    //fout << "<表达式>" << endl;
}

void genExpIR(IROp op) {
    IRItem* item2 = popExpStack();
    IRItem* item1 = popExpStack();
    IRItem* item3;
    if (item1->getKind() == Const && item2->getKind() == Const) {
        int value1 = item1->getValue();
        int value2 = item2->getValue();
        int value3 = 0;
        switch (op) {
            case Add: {
                value3 = value1 + value2;
                break;
            }
            case Sub: {
                value3 = value1 - value2;
                break;
            }
            case Mult: {
                value3 = value1 * value2;
                break;
            }
            case Div: {
                value3 = value1 / value2;
                break;
            }
            default:break;
        }
        item3 = new IRItem(to_string(value3), Const, Int, value3);
    } else {
        if (item1->getKind() == TmpVar && item1->getType()!=Char) {
            item3 = item1;
        } else if (item2->getKind() == TmpVar && item2->getType()!=Char) {
            item3 = item2;
        } else {
            varOffset += 4;
            item3 = new IRItem(genTmpVarName(), TmpVar, op==ArrayLoad&&item1->getType()==Char?Char:Int, varOffset);
        }
        insertIRTable(new IREntry(op, item1, item2, item3));
    }
    pushExpStack(item3);
}

int term() {
    int factorCount = 0;
    int factorRet;
    IROp op = Mult;
    factorRet = factor();
    factorCount++;
    while (sym.type == MULT || sym.type == DIV) {
        op = sym.type==MULT?Mult:Div;
        sym = getSym(fin);
        factor();
        genExpIR(op);
        factorCount++;
    }
    OUTPUT("<项>");
    if (factorCount > 1) {
        return 1;
    } else {
        return factorRet;
    }
    //fout << "<项>" << endl;
}

int factor() {
    int ret;
    if (sym.type == IDENFR) {
        Symbol tmp = sym;
        sym = getSym(fin);
        if (sym.type == LPARENT) {
            pushSym(tmp);
            pushSym(sym);
            sym = getSym(fin);
            int tmp = funcStatement();
            pushExpStack(new IRItem(retItem->getName(), retItem->getKind(), retItem->getType(), retItem->getValue()));
            return tmp;
        }
        errUndefId(lastSym.token, lastSym.lineNo);
        IRItem* item = getIRItem(lastSym.token);
        pushExpStack(item);
        ret = (getIdType(lastSym.token) == INT) ? 1 : 2;
        if (sym.type == LBRACK) {
            sym = getSym(fin);
            if (expression() != 1) {
                errArrayIndexNotInt(lastSym.lineNo);
            }
            if (sym.type != RBRACK) {
                errLackRBrack(sym.lineNo);
            } else {
                sym = getSym(fin);
            }
            if (sym.type == LBRACK) {
                sym = getSym(fin);
                if (expression() != 1) {
                    errArrayIndexNotInt(lastSym.lineNo);
                }
                if (sym.type != RBRACK) {
                    errLackRBrack(sym.lineNo);
                } else {
                    sym = getSym(fin);
                }
                IRItem* item1 = popExpStack();
                IRItem* item2 = popExpStack();
                if (item1->getKind() == Const && item2->getKind() == Const) {
                    int value = item2->getValue() * item->getSize1() + item1->getValue();
                    auto item3 = new IRItem(to_string(value), Const, Int, value);
                    pushExpStack(item3);
                } else {
                    pushExpStack(item2);
                    pushExpStack(new IRItem("size", Const, Int, item->getSize1()));
                    genExpIR(Mult);
                    pushExpStack(item1);
                    genExpIR(Add);
                }
            }
            genExpIR(ArrayLoad);
        }
    } else if (sym.type == LPARENT) {
        sym = getSym(fin);
        expression();
        IRItem* item = popExpStack();
        pushExpStack(new IRItem(item->getName(), item->getKind(), Int, item->getValue()));
        if (sym.type != RPARENT) {
            errLackRParent(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
        ret = 1;
    } else if (sym.type == PLUS || sym.type == MINU || sym.type == INTCON) {
        integer();
        pushExpStack(new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant));
        ret = 1;
    } else if (sym.type == CHARCON) {
        pushExpStack(new IRItem(sym.token, Const, Char, sym.token[0]));
        sym = getSym(fin);
        ret = 2;
    } else {
        ret = 0;
        /*error*/
    }
    OUTPUT("<因子>");
    return ret;
    //fout << "<因子>" << endl;
}

void stepLength() {
    unsignedInteger();
    OUTPUT("<步长>");
    //fout << "<步长>" << endl;
}

void ifStatement() {
    if (sym.type != IFTK) {
        /*error*/
    }
    if ((sym = getSym(fin)).type != LPARENT) {
        /*error*/
    }
    sym = getSym(fin);
    condition();
    string ifNo = genIfLabel();
    string ifEnd = "if_" + ifNo + "_end";
    IRItem* item2 = popExpStack();
    IRItem* item1 = popExpStack();
    insertIRTable(new IREntry(branchType, item1, item2, new IRItem(ifEnd, Const, String)));
    if (sym.type != RPARENT) {
        errLackRParent(lastSym.lineNo);
    } else {
        sym = getSym(fin);
    }
    statement();
    if (sym.type == ELSETK) {
        sym = getSym(fin);
        string elseEnd = "else_" + ifNo + "_end";
        insertIRTable(new IREntry(Jump, new IRItem(elseEnd, Const, String)));
        insertIRTable(new IREntry(Label, new IRItem(ifEnd, Const, String)));
        statement();
        insertIRTable(new IREntry(Label, new IRItem(elseEnd, Const, String)));
    } else {
        insertIRTable(new IREntry(Label, new IRItem(ifEnd, Const, String)));
    }
    OUTPUT("<条件语句>");
    //fout << "<条件语句>" << endl;
}

int funcStatement() {
    string name;
    if (sym.type != IDENFR) {
        /*error*/
    }
    name = sym.token;
    bool undef = errUndefFuncId(name, sym.lineNo);
    bool withRet = isRetFunc(name);
    if ((sym = getSym(fin)).type != LPARENT) {
        /*error*/
    }
    sym = getSym(fin);
    vector<IdType> localParaTable = valueParaList();
    if (sym.type != RPARENT) {
        errLackRParent(lastSym.lineNo);
    } else {
        if (!undef) {
            errUnmatchedPara(name, localParaTable, sym.lineNo);
        }
        sym = getSym(fin);
    }
    IdType type = getFuncType(name);
    if (withRet) {
        insertIRTable(new IREntry(FuncCall, new IRItem("varOffset", Const, Int, varOffset), new IRItem("func_" + strToLower(name), Func, type==INT?Int:Char)));
        varOffset += 4;
        retItem = new IRItem(genTmpVarName(), TmpVar, type==INT?Int:Char, varOffset);
        insertIRTable(new IREntry(FuncCallEnd, new IRItem(retItem->getName(), retItem->getKind(), retItem->getType(), retItem->getValue())));
        OUTPUT("<有返回值函数调用语句>");
    } else{
        insertIRTable(new IREntry(FuncCall, new IRItem("varOffset", Const, Int, varOffset), new IRItem("func_" + strToLower(name), Func, Void)));
        OUTPUT("<无返回值函数调用语句>");
    }
    return type == VOID? 0 : type == INT ? 1 : 2;
    //fout << "<有返回值函数调用语句>" << endl;
}

vector<IdType> valueParaList() {
    vector<IdType> localParaTable;
    IdType type;
    string name = "valuePara";
    int paraCount = 0;
    if (sym.type == RPARENT) {
        //fout << "<值参数表>" << endl;
    } else {
        int expRet = expression();
        type = (expRet == 0) ? VOID : (expRet == 1) ? INT : CHAR;
        if (type != VOID) {
            localParaTable.emplace_back(type);
        }
        paraCount++;
        while (sym.type == COMMA) {
            sym = getSym(fin);
            expRet = expression();
            type = (expRet == 0) ? VOID : (expRet == 1) ? INT : CHAR;
            if (type != VOID) {
                localParaTable.emplace_back(type);
            }
            paraCount++;
        }
    }
    for (int i = paraCount - 1; i >= 0; i--) {
        IRItem* item = popExpStack();
        insertIRTable(new IREntry(PushPara, item, new IRItem("pos", LocVar, item->getType(), varOffset + 12 + i * 4)));
    }
    OUTPUT("<值参数表>");
    return localParaTable;
    //fout << "<值参数表>" << endl;
}

void assignStatement() {
    IROp op = Assign;
    if (sym.type != IDENFR) {
        /*error*/
    }
    errUndefId(sym.token, sym.lineNo);
    errChangeConst(sym.token, sym.lineNo);
    IRItem* item = getIRItem(sym.token);
    pushExpStack(item);
    sym = getSym(fin);
    if (sym.type == ASSIGN) {
        sym = getSym(fin);
        expression();
    } else if (sym.type == LBRACK) {
        op = ArrayStore;
        sym = getSym(fin);
        if (expression() != 1) {
            errArrayIndexNotInt(lastSym.lineNo);
        }
        if (sym.type != RBRACK) {
            errLackRBrack(sym.lineNo);
        } else {
            sym = getSym(fin);
        }
        if (sym.type == ASSIGN) {
            sym = getSym(fin);
            expression();
        } else if (sym.type == LBRACK) {
            sym = getSym(fin);
            if (expression() != 1) {
                errArrayIndexNotInt(lastSym.lineNo);
            }
            IRItem* item1 = popExpStack();
            IRItem* item2 = popExpStack();
            if (item1->getKind() == Const && item2->getKind() == Const) {
                int value = item2->getValue() * item->getSize1() + item1->getValue();
                auto item3 = new IRItem(to_string(value), Const, Int, value);
                pushExpStack(item3);
            } else {
                pushExpStack(item2);
                pushExpStack(new IRItem("size", Const, Int, item->getSize1()));
                genExpIR(Mult);
                pushExpStack(item1);
                genExpIR(Add);
            }
            if (sym.type != RBRACK) {
                errLackRBrack(sym.lineNo);
            } else {
                sym = getSym(fin);
            }
            if (sym.type != ASSIGN) {
                /*error*/
            }
            sym = getSym(fin);
            expression();
        } else {
            /*error*/
        }
    } else {
        /*error*/
    }
    genAssignIR(op);
    OUTPUT("<赋值语句>");
    //fout << "<赋值语句>" << endl;
}

void genAssignIR(IROp op) {
    IRItem* item1 = popExpStack();
    IRItem* item2 = popExpStack();
    if (op == Assign) {
        insertIRTable(new IREntry(op, item1, item2));
    } else if (op == ArrayStore) {
        IRItem* item3 = popExpStack();
        insertIRTable(new IREntry(op, item1, item2, item3));
    }
}

void scanfStatement() {
    if (sym.type != SCANFTK) {
        /*error*/
    }
    if ((sym = getSym(fin)).type != LPARENT) {
        /*error*/
    }
    if ((sym = getSym(fin)).type != IDENFR) {
        /*error*/
    }
    errUndefId(sym.token, sym.lineNo);
    errChangeConst(sym.token, sym.lineNo);
    insertIRTable(new IREntry(Scanf, getIRItem(sym.token)));
    if ((sym = getSym(fin)).type != RPARENT) {
        errLackRParent(lastSym.lineNo);
    } else {
        sym = getSym(fin);
    }
    OUTPUT("<读语句>");
    //fout << "<读语句>" << endl;
}

void printfStatement() {
    if (sym.type != PRINTFTK) {
        /*error*/
    }
    if ((sym = getSym(fin)).type != LPARENT) {
        /*error*/
    }
    sym = getSym(fin);
    if (sym.type == STRCON) {
        parseStr();
        insertIRTable(new IREntry(Printf, new IRItem(nowStringName, Const, String)));
        if (sym.type == COMMA) {
            sym = getSym(fin);
            expression();
            insertIRTable(new IREntry(Printf, popExpStack()));
        }
    } else {
        expression();
        insertIRTable(new IREntry(Printf, popExpStack()));
    }
    if (sym.type != RPARENT) {
        errLackRParent(lastSym.lineNo);
    } else {
        sym = getSym(fin);
    }
    insertIRTable(new IREntry(Printf, new IRItem("newLine", Const, String)));
    OUTPUT("<写语句>");
    //fout << "<写语句>" << endl;
}

void parseStr() {
    if (sym.type != STRCON) {
        /*error*/
    }
    nowStringName = genGlbStrName();
    glbString string1;
    string1.name = nowStringName;
    string1.content = sym.token;
    insertGlbStrings(string1);
    sym = getSym(fin);
    OUTPUT("<字符串>");
    //fout << "<字符串>" << endl;
}

void switchStatement() {
    if (sym.type != SWITCHTK) {
        /*error*/
    }
    string switchNo = genSwitchLabel();
    if ((sym = getSym(fin)).type != LPARENT) {
        /*error*/
    }
    sym = getSym(fin);
    switchType = (expression() == 1) ? INT : CHAR;
    if (sym.type != RPARENT) {
        errLackRParent(lastSym.lineNo);
    } else {
        sym = getSym(fin);
    }
    if (sym.type != LBRACE) {
        /*error*/
    }
    sym = getSym(fin);
    switchList(switchNo);
    if (sym.type != DEFAULTTK) {
        errLackDefault(sym.lineNo);
    } else {
        switchDefault();
    }
    if (sym.type != RBRACE) {
        /*error*/
    }
    insertIRTable(new IREntry(Label, new IRItem("switch_" + switchNo + "_end", Const, String)));
    sym = getSym(fin);
    OUTPUT("<情况语句>");
    //fout << "<情况语句>" << endl;
}

void switchList(string switchNo) {
    int switchCaseNo = 0;
    switchSubStatement(switchNo, switchCaseNo++);
    while (sym.type == CASETK) {
        switchSubStatement(switchNo, switchCaseNo++);
    }
    popExpStack();
    OUTPUT("<情况表>");
    //fout << "<情况表>" << endl;
}

void switchSubStatement(string switchNo, int switchCaseNo) {
    if (sym.type != CASETK) {
        /*error*/
    }
    sym = getSym(fin);
    IdType constType = (constant() == 1) ? INT : CHAR;
    IRItem* item1 = expStack.back();
    IRItem* item2 = constType==INT?
            new IRItem(to_string(nowIntConstant), Const, Int, nowIntConstant):
            new IRItem(nowCharConstant, Const, Char, nowCharConstant[0]);
    string switchCaseEnd = "switch_" + switchNo + "_case_" + to_string(switchCaseNo) + "_end";
    insertIRTable(new IREntry(Bne, item1, item2, new IRItem(switchCaseEnd, Const, String)));
    if (switchType != constType) {
        errUnmatchedConstType(sym.lineNo);
    }
    if (sym.type != COLON) {
        /*error*/
    }
    sym = getSym(fin);
    statement();
    insertIRTable(new IREntry(Jump, new IRItem("switch_" + switchNo + "_end", Const, String)));
    insertIRTable(new IREntry(Label, new IRItem(switchCaseEnd, Const, String)));
    OUTPUT("<情况子语句>");
    //fout << "<情况子语句>" << endl;
}

void switchDefault() {
    if (sym.type != DEFAULTTK) {
        /*error*/
    }
    if ((sym = getSym(fin)).type != COLON) {
        /*error*/
    }
    sym = getSym(fin);
    statement();
    OUTPUT("<缺省>");
    //fout << "<缺省>" << endl;
}

void returnStatement() {
    if (sym.type != RETURNTK) {
        /*error*/
    }
    sym = getSym(fin);
    if (sym.type == LPARENT) {
        sym = getSym(fin);
        if (funcType == VOID) {
            errVoidFuncRet(sym.lineNo);
        } else if (sym.type == RPARENT) {
            errRetFuncUnmatch(sym.lineNo);
        }
        IdType expType = (expression() == 1) ? INT :CHAR;
        if (funcType != VOID && funcType != expType) {
            errRetFuncUnmatch(sym.lineNo);
        }
        if (sym.type != RPARENT) {
            errLackRParent(lastSym.lineNo);
        } else {
            sym = getSym(fin);
        }
        insertIRTable(new IREntry(FuncRet, new IRItem("ret", Func, funcType==INT?Int:Char), popExpStack()));
    } else if (sym.type == SEMICN) {
        if (funcType != VOID) {
            errRetFuncUnmatch(sym.lineNo);
        } else if (isMain) {
            insertIRTable(new IREntry(Jump, new IRItem("main_end", Const, String)));
        } else {
            insertIRTable(new IREntry(FuncRet, new IRItem("ret", Func, Void)));
        }
    }
    hasRet = true;
    OUTPUT("<返回语句>");
    //fout << "<返回语句>" << endl;
}

void mainFunc() {
    if (sym.type != VOIDTK) {
        /*error*/
    }
    if ((sym = getSym(fin)).type != MAINTK) {
        /*error*/
    }
    isMain = true;
    if ((sym = getSym(fin)).type != LPARENT) {
        /*error*/
    }
    if ((sym = getSym(fin)).type != RPARENT) {
        errLackRParent(lastSym.lineNo);
    } else {
        sym = getSym(fin);
    }
    if (sym.type != LBRACE) {
        /*error*/
    }
    insertIRTable(new IREntry(Label, new IRItem("main", Const, String)));
    sym = getSym(fin);
    compoundStatement();
    if (sym.type != RBRACE) {
        /*error*/
    }
    sym = getSym(fin);
    outputList.emplace_back("<主函数>");
}