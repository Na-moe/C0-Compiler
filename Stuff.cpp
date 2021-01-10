#include "Stuff.h"

string strToLower(string raw) {
    string lower;
    lower.resize(raw.size());
    transform(raw.begin(), raw.end(), lower.begin(), ::tolower);
    return lower;
}