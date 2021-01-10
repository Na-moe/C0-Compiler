#include "IR.h"

extern ofstream fir;

vector<IREntry*> IRTable;
vector<glbString> glbStrings;

extern map<IROp, string> b2str;
map<IRItemType, string> type2str = {
        {Int, "int"},
        {Char, "char"},
        {Void, "void"},
        {String, "string"},
};

IRItem::IRItem(string aname, IRItemKind akind, IRItemType atype, int avalue, int asize1) {name=aname, kind=akind, type=atype, value=avalue; size1=asize1;}
string IRItem::toString() {
    if (kind == Const) {
        if (type == Char) {
            return "'" + string(1, char(value)) + "'";
        } else if (type == Int){
            return to_string(value);
        } else {
            return name;
        }
    } else {
        return name;
    }
}

IREntry::IREntry(IROp aop) {op = aop;}
IREntry::IREntry(IROp aop, IRItem* aitem1) {op = aop; item1 = aitem1;}
IREntry::IREntry(IROp aop, IRItem* aitem1, IRItem* aitem2) {op = aop; item1 = aitem1; item2 = aitem2;}
IREntry::IREntry(IROp aop, IRItem* aitem1, IRItem* aitem2, IRItem* aitem3) {op = aop; item1 = aitem1; item2 = aitem2; item3 = aitem3;}
string IREntry::toString() {
    string result;
    switch (op) {
        case ConstIntDef:
            result.append("Const Int ").append(item1->getName()).append(" = ").append(item1->toString());
            break;
        case ConstCharDef:
            result.append("Const Char ").append(item1->getName()).append(" = ").append(item1->toString());
            break;
        case VarIntDef:
            result.append("Var Int ").append(item1->getName()).append(item1->getSize1()>0?"[]":"");
            break;
        case VarCharDef:
            result.append("Var Char ").append(item1->getName()).append(item1->getSize1()>0?"[]":"");
            break;
        case Scanf:
            result.append("Scanf ").append(item1->toString());
            break;
        case Printf:
            result.append("Printf ").append(item1->toString());
            break;
        case Assign:
            result.append("").append(item1->toString()).append("\t=\t").append(item2->toString());
            break;
        case Add:
            result.append(item3->toString()).append("\t=\t").append(item1->toString()).append("\t+\t").append(item2->toString());
            break;
        case Sub:
            result.append(item3->toString()).append("\t=\t").append(item1->toString()).append("\t-\t").append(item2->toString());
            break;
        case Mult:
            result.append(item3->toString()).append("\t=\t").append(item1->toString()).append("\t*\t").append(item2->toString());
            break;
        case Div:
            result.append(item3->toString()).append("\t=\t").append(item1->toString()).append("\t/\t").append(item2->toString());
            break;
        case ArrayLoad:
            result.append(item3->toString()).append("\t=\t").append(item1->toString()).append("[").append(item2->toString()).append("]");
            break;
        case ArrayStore:
            result.append(item3->toString()).append("[").append(item2->toString()).append("]").append("\t=\t").append(item1->toString());
            break;
        case Bge:
        case Bgt:
        case Ble:
        case Blt:
        case Beq:
        case Bne:
            result.append(b2str[op]).append("\t").append(item1->toString()).append("\t").append(item2->toString()).append("\t").append(item3->toString());
            break;
        case Jump:
            result.append("J\t").append(item1->toString());
            break;
        case FuncDef:
            result.append("Func ").append(type2str[item1->getType()]).append(" ").append(item1->getName());
            break;
        case FuncCall:
            result.append("Call ").append(item2->toString());
            break;
        case PushPara:
            result.append("Push ").append(item1->toString());
            break;
        case FuncCallEnd:
            result.append(item1->toString()).append(" = retValue");
            break;
        case FuncRet:
            result.append("Ret ").append(item1->getType()==Void?"Void":item1->toString());
            break;
        case Label:
            result.append("Label ").append(item1->toString());
            break;
    }
    return result;
}

void outputIR() {
    for (IREntry* ir: IRTable) {
        fir << ir->toString() << endl;
    }
}

void insertIRTable(IREntry* entry) {
    IRTable.emplace_back(entry);
}

void insertGlbStrings(glbString str) {
    glbStrings.push_back(str);
}