#ifndef __IR_H__
#define __IR_H__

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

using namespace std;

enum IROp {
    ConstIntDef, ConstCharDef, VarIntDef, VarCharDef,
    Scanf, Printf,
    Assign, Add, Sub, Mult, Div,
    ArrayLoad, ArrayStore,
    Bge, Bgt, Ble, Blt, Beq, Bne, Jump,
    FuncDef, FuncCall, PushPara, FuncCallEnd, FuncRet,
    Label
};

enum IRItemKind {
    Const, GlbVar, LocVar, TmpVar, Func
};

enum IRItemType {
    Int, Char, Void, String
};

struct glbString {
    string name;
    string content;
};

class IRItem {
private:
    string name;
    IRItemKind kind;
    IRItemType type;
    int value;
    int size1;
public:
    IRItem(string aname, IRItemKind akind, IRItemType atype, int avalue=0, int asize1=0);
    string getName() {return name;}
    IRItemKind getKind() {return kind;}
    IRItemType getType() {return type;}
    int getValue() {return value;}
    int getSize1() {return size1;}
    string toString();
};

class IREntry {
private:
    IROp op;
    IRItem* item1=NULL;
    IRItem* item2=NULL;
    IRItem* item3=NULL;
public:
    IREntry(IROp aop);
    IREntry(IROp aop, IRItem* aitem1);
    IREntry(IROp aop, IRItem* aitem1, IRItem* aitem2);
    IREntry(IROp aop, IRItem* aitem1, IRItem* aitem2, IRItem* aitem3);
    IROp getOp() {return op;}
    IRItem* getItem1() {return item1;}
    IRItem* getItem2() {return item2;}
    IRItem* getItem3() {return item3;}
    string toString();
};

void insertIRTable(IREntry* entry);
void insertGlbStrings(glbString str);
void outputIR();

extern vector<IREntry*> IRTable;
extern vector<glbString> glbStrings;

#endif
