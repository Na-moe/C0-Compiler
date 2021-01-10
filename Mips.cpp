#include "Mips.h"

extern ofstream fmips;

map<IROp, string> b2str = {
        {Bgt, "bgt"},
        {Bge, "bge"},
        {Blt, "blt"},
        {Ble, "ble"},
        {Beq, "beq"},
        {Bne, "bne"}
};

void genStrings() {
    fmips << ".data" << endl;
    for (glbString str: glbStrings) {
        fmips << "\t" << str.name << ":\t" << ".asciiz \"" << str.content << "\"" << endl;
    }
    fmips << "\tnewLine:\t.asciiz \"\\n\"" << endl;
}

int tmpRegNo = -1;

void genMips() {
    //strings
    genStrings();
    //.text
    fmips << ".text" << endl;
    int i;
    //glbVarDef
    fmips << "\taddu\t$gp\t$gp\t0x2000" <<  endl;
    for (i = 0; i < IRTable.size(); i++) {
        if (IRTable[i]->getOp() == VarIntDef || IRTable[i]->getOp() == VarCharDef) {
            i++;
            if (IRTable[i]->getOp() == Assign) {
                tmpRegNo = -1;
                IRItem* item1 = IRTable[i]->getItem1();
                IRItem* item2 = IRTable[i]->getItem2();
                string regName = loadToReg(item1, "tmp");
                storeToMem(item2, regName);
            } else if (IRTable[i]->getOp() == ArrayStore) {
                for (; i < IRTable.size() && IRTable[i]->getOp() == ArrayStore; i++) {
                    tmpRegNo = -1;
                    IRItem *item1 = IRTable[i]->getItem1();
                    IRItem *item2 = IRTable[i]->getItem2();
                    IRItem *item3 = IRTable[i]->getItem3();
                    string valueReg = loadToReg(item1, "tmp");
                    int index = item2->getValue();
                    int array = item3->getValue();
                    string reg = item3->getKind() == GlbVar ? "($gp)" : "($sp)";
                    fmips << "\tsw\t" << valueReg << "\t" << to_string(-array + index * 4) << reg << endl;
                }
                i--;
            } else {
                i--;
            }
        } else {
            break;
        }
    }
    //j main
    fmips << "\tj\tmain" << endl;
    //mips
    for (; i < IRTable.size(); i++) {
        IRItem* item1 = IRTable[i]->getItem1();
        IRItem* item2 = IRTable[i]->getItem2();
        IRItem* item3 = IRTable[i]->getItem3();
        IROp op = IRTable[i]->getOp();
        tmpRegNo = -1;
        switch (op) {
            case ConstIntDef:
            case ConstCharDef:
                break;
            case VarIntDef:
            case VarCharDef: {
                for (; i < IRTable.size(); i++) {
                    if (IRTable[i]->getOp() != VarIntDef && IRTable[i]->getOp() != VarCharDef) {
                        break;
                    }
                }
                i--;
                break;
            }
            case Add: {
                string src1 = loadToReg(item1, "tmp");
                string src2 = loadToReg(item2, "tmp");
                tmpRegNo++;
                string dst = "$t" + to_string(tmpRegNo);
                fmips << "\taddu\t" << dst << "\t" << src1 << "\t" << src2 << endl;
                storeToMem(item3, dst);
                break;
            }
            case Sub: {
                string src1 = loadToReg(item1, "tmp");
                string src2 = loadToReg(item2, "tmp");
                tmpRegNo++;
                string dst = "$t" + to_string(tmpRegNo);
                fmips << "\tsubu\t" << dst << "\t" << src1 << "\t" << src2 << endl;
                storeToMem(item3, dst);
                break;
            }
            case Mult: {
                string src1 = loadToReg(item1, "tmp");
                int val = item2->getValue();
                if (item2->getKind()==Const && !(val&(val-1))) {
                    int pow = 0;
                    for (; val != 1; val /= 2) {
                        pow++;
                    }
                    string dst = "$t" + to_string(tmpRegNo);
                    fmips << "\tsll\t" << dst << "\t" << src1 << "\t" << to_string(pow) << endl;
                    storeToMem(item3, dst);
                } else {
                    string src2 = loadToReg(item2, "tmp");
                    tmpRegNo++;
                    string dst = "$t" + to_string(tmpRegNo);
                    fmips << "\tmult\t" << src1 << "\t" << src2 << endl;
                    fmips << "\tmflo\t" << dst << endl;
                    storeToMem(item3, dst);
                }
                break;
            }
            case Div: {
                string src1 = loadToReg(item1, "tmp");
                string src2 = loadToReg(item2, "tmp");
                tmpRegNo++;
                string dst = "$t" + to_string(tmpRegNo);
                fmips << "\tdiv\t" << src1 << "\t" << src2 << endl;
                fmips << "\tmflo\t" << dst << endl;
                storeToMem(item3, dst);
                break;
            }
            case Assign: {
                string regName = loadToReg(item1, "tmp");
                storeToMem(item2, regName);
                break;
            }
            case Scanf: {
                int v0 = 0;
                switch (item1->getType()) {
                    case Int: {
                        v0 = 5;
                        break;
                    }
                    case Char: {
                        v0 = 12;
                        break;
                    }
                    default:break;
                }
                fmips << "\tli\t$v0\t" << to_string(v0) << endl;
                fmips << "\tsyscall" << endl;
                storeToMem(item1, "$v0");
                break;
            }
            case Printf: {
                int v0 = 0;
                switch (item1->getType()) {
                    case Int: {
                        v0 = 1;
                        string regName = loadToReg(item1, "tmp");
                        fmips << "\tmove\t$a0\t" << regName << endl;
                        break;
                    }
                    case Char: {
                        v0 = 11;
                        string regName = loadToReg(item1, "tmp");
                        fmips << "\tmove\t$a0\t" << regName << endl;
                        break;
                    }
                    case String: {
                        v0 = 4;
                        fmips << "\tla\t$a0\t" << item1->getName() << endl;
                        break;
                    }
                    default:break;
                }
                fmips << "\tli\t$v0\t" << to_string(v0) << endl;
                fmips << "\tsyscall" << endl;
                break;
            }
            case ArrayStore: {
                string valueReg = loadToReg(item1, "tmp");
                string indexReg = loadToReg(item2, "tmp");
                string reg = item3->getKind()==GlbVar?"$gp":"$sp";
                fmips << "\tsll\t" << indexReg << "\t" << indexReg << "\t2" << endl;
                fmips << "\taddu\t" << indexReg << "\t" << indexReg << "\t" << reg << endl;
                int array = item3->getValue();
                fmips << "\tsw\t" << valueReg << "\t" << to_string(-array) << "(" << indexReg << ")" << endl;
                break;
            }
            case ArrayLoad: {
                string indexReg = loadToReg(item2, "tmp");
                string reg = item1->getKind()==GlbVar?"$gp":"$sp";
                fmips << "\tsll\t" << indexReg << "\t" << indexReg << "\t2" << endl;
                fmips << "\taddu\t" << indexReg << "\t" << indexReg << "\t" << reg << endl;
                string valueReg = "$t" + to_string(++tmpRegNo);
                int array = item1->getValue();
                fmips << "\tlw\t" << valueReg << "\t" << to_string(-array) << "(" << indexReg << ")" << endl;
                storeToMem(item3, valueReg);
                break;
            }
            case Bgt:
            case Bge:
            case Blt:
            case Ble:
            case Beq:
            case Bne: {
                string reg1 = loadToReg(item1, "tmp");
                string reg2 = loadToReg(item2, "tmp");
                fmips << "\t" << b2str[op] << "\t" << reg1 << "\t" << reg2 << "\t" << item3->getName() << endl;
                break;
            }
            case Jump: {
                fmips << "\tj\t" << item1->getName() << endl;
                break;
            }
            case FuncDef: {
                fmips << item1->getName() << ":" << endl;
                fmips << "\tsw\t$ra\t-4($sp)" << endl;
                break;
            }
            case PushPara: {
                string reg = loadToReg(item1, "tmp");
                storeToMem(item2, reg);
                break;
            }
            case FuncCall: {
                int size = item1->getValue();
                fmips << "\tsw\t$sp\t" << to_string(-(size+8)) << "($sp)" << endl;
                fmips << "\tsubi\t$sp\t$sp\t" << to_string(size) << endl;
                fmips << "\tjal\t" << item2->getName() << endl;
                fmips << "\tlw\t$sp\t-8($sp)" << endl;
                break;
            }
            case FuncCallEnd: {
                storeToMem(item1, "$v0");
                break;
            }
            case FuncRet: {
                loadToReg(new IRItem("ra", LocVar, Int, 4), "$ra");
                if (item1->getType() != Void) {
                    loadToReg(item2, "$v0");
                }
                fmips << "\tjr\t$ra" << endl;
                break;
            }
            case Label: {
                fmips << item1->getName() << ":" << endl;
                break;
            }
        }
    }
}

string loadToReg(IRItem* item, string regName) {
    int varOffset = item->getValue();
    if (regName == "tmp") {
        tmpRegNo++;
        regName = "$t" + to_string(tmpRegNo);
    }
    switch (item->getKind()) {
        case Const: {
            fmips << "\tli\t" << regName << "\t" << to_string(item->getValue()) << endl;
            break;
        }
        case GlbVar: {
            fmips << "\tlw\t" << regName << "\t" << to_string(-varOffset) << "($gp)" << endl;
            break;
        }
        case LocVar:
        case TmpVar:
            fmips << "\tlw\t" << regName << "\t" << to_string(-varOffset) << "($sp)" << endl;
            break;
        default:break;
    }
    return regName;
}

void storeToMem(IRItem* item, string regName) {
    int varOffset = item->getValue();
    switch (item->getKind()) {
        case Const: {
            break;
        }
        case GlbVar: {
            fmips << "\tsw\t" << regName << "\t" << to_string(-varOffset) << "($gp)" << endl;
            break;
        }
        case LocVar:
        case TmpVar:
            fmips << "\tsw\t" << regName << "\t" << to_string(-varOffset) << "($sp)" << endl;
            break;
        default:break;
    }
}