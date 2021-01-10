#ifndef __MIPS_H__
#define __MIPS_H__

#include <iostream>
#include <fstream>
#include <map>
#include "IR.h"

using namespace std;

void genStrings();
void genMips();
string loadToReg(IRItem* item, string regName);
void storeToMem(IRItem* item, string regName);

#endif
