#include <iostream>
#include "Parser.h"
#include "Mips.h"
using namespace std;

ifstream fin("./testfile.txt");
ofstream fout("./output.txt");
ofstream ferr("./error.txt");
ofstream fmips("./mips.txt");
ofstream fir("./ir.txt");

Symbol sym;

int main() {
    parse();
    outputIR();
    genMips();
    fin.close();
    fout.close();
    ferr.close();
    fmips.close();
    fir.close();
    return 0;
}
