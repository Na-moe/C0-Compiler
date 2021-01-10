#include <cctype>
#include <map>
#include "Lexer.h"

#define isletter(c) (isalpha(c) || (c == '_'))
#define ischar(c) ((c == '+') || (c == '-') || (c == '*') || (c == '/') || isletter(c) || isdigit(c))
#define isInString(c) ((c == ' ') || (c == '!') || (c > 34 && c < 127))

using namespace std;

extern Symbol sym;
Symbol lastSym;

list<string> outputList;

static char ch = ' ';
int lineNo = 1;

map<string, tCode> tk2tCode = {
        {"const", CONSTTK}, {"int", INTTK}, {"char", CHARTK},
        {"void", VOIDTK}, {"main", MAINTK},
        {"if", IFTK}, {"else", ELSETK},
        {"switch", SWITCHTK}, {"case", CASETK}, {"default", DEFAULTTK},
        {"while", WHILETK}, {"for", FORTK},
        {"scanf", SCANFTK}, {"printf", PRINTFTK},
        {"return", RETURNTK},
        {"+", PLUS}, {"-", MINU}, {"*", MULT}, {"/", DIV},
        {"<", LSS}, {"<=", LEQ}, {">", GRE}, {">=", GEQ}, {"==", EQL}, {"!=", NEQ},
        {":", COLON}, {"=", ASSIGN}, {";", SEMICN}, {",", COMMA},
        {"(", LPARENT}, {")", RPARENT}, {"[", LBRACK}, {"]", RBRACK}, {"{", LBRACE}, {"}", RBRACE}
};

map<tCode, string> tCode2str = {
        {IDENFR, "IDENFR"},
        {INTCON, "INTCON"}, {CHARCON, "CHARCON"}, {STRCON, "STRCON"},
        {CONSTTK, "CONSTTK"}, {INTTK, "INTTK"}, {CHARTK, "CHARTK"},
        {VOIDTK, "VOIDTK"}, {MAINTK, "MAINTK"},
        {SWITCHTK, "SWITCHTK"}, {CASETK, "CASETK"}, {DEFAULTTK, "DEFAULTTK"},
        {IFTK, "IFTK"}, {ELSETK, "ELSETK"},
        {WHILETK, "WHILETK"}, {FORTK, "FORTK"},
        {SCANFTK, "SCANFTK"}, {PRINTFTK, "PRINTFTK"},
        {RETURNTK, "RETURNTK"},
        {PLUS, "PLUS"}, {MINU, "MINU"}, {MULT, "MULT"}, {DIV, "DIV"},
        {LSS, "LSS"}, {LEQ, "LEQ"}, {GRE, "GRE"}, {GEQ, "GEQ"}, {EQL, "EQL"}, {NEQ, "NEQ"},
        {COLON, "COLON"}, {ASSIGN, "ASSIGN"}, {SEMICN, "SEMICN"}, {COMMA, "COMMA"},
        {LPARENT, "LPARENT"}, {RPARENT, "RPARENT"}, {LBRACK, "LBRACK"}, {RBRACK, "RBRACK"}, {LBRACE, "LBRACE"}, {RBRACE, "RBRACE"},
};

Symbol preGetSym(ifstream &fin) {
    lastSym = sym;
    tCode symCode;
    string token;
    while (isspace(ch)) {
        if (ch == '\n') {
            lineNo++;
        }
        ch = fin.get();
    }
    if (ch == EOF) {
        Symbol symbol;
        return symbol;
    }
    if (isletter(ch)) {
        while (isletter(ch) | isdigit(ch)) {
            token.append(1, ch);
            ch = fin.get();
        }
        auto it = tk2tCode.find(strToLower(token));
        symCode = (it == tk2tCode.end()) ? IDENFR : it->second;
    } else if (isdigit(ch)) {
        while (isdigit(ch)) {
            token.append(1, ch);
            ch = fin.get();
        }
        symCode = INTCON;
    } else if (ch == '\'') {
        ch = fin.get();
        if (!ischar(ch)) {
            errSym(lineNo);
        }
        token.append(1, ch);
        ch = fin.get();//error at not '\''
        ch = fin.get();
        symCode = CHARCON;
    } else if (ch == '"') {
        ch = fin.get();
        if (ch == '"') {
            errSym(lineNo);
        }
        while (ch != '"') {
            if (!isInString(ch)) {
                errSym(lineNo);
            }
            token.append(1, ch);
            if (ch == '\\') {
                token.append(1, ch);
            }
            ch = fin.get();
        }
        ch = fin.get();
        symCode = STRCON;
    } else {
        token.append(1, ch);
        switch (ch) {
            case '+': case '-': case '*': case '/':
            case ':': case ';': case ',':
            case '(': case ')': case '[': case ']': case '{': case '}':
                break;
            case '<': case '>': case '=': case '!':
                ch = fin.peek();
                if (ch == '=') {
                    ch = fin.get();
                    token.append(1, ch);
                } // else error at only '!'
                break;
            default:
                errSym(lineNo);
        }
        symCode = tk2tCode[strToLower(token)];
        ch = fin.get();
    }
    outputList.push_back(tCode2str[symCode] + " " + token);
    Symbol symbol;
    symbol.type = symCode;
    symbol.token = token;
    symbol.lineNo = lineNo;
    return symbol;
}

queue<Symbol> symQueue;
Symbol getSym(ifstream &fin) {
    if (symQueue.empty()) {
        return preGetSym(fin);
    } else {
        lastSym = sym;
        Symbol tmp = symQueue.front();
        symQueue.pop();
        return tmp;
    }
}

void pushSym(Symbol sym) {
    symQueue.push(sym);
}