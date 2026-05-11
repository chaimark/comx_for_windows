#ifndef __JSONSETLIB_H__
#define __JSONSETLIB_H__

#include "StrLib.h"

extern void addJsonItemData(strnew JsonStringSpace, const char *FromStr, ...);
#define newRootJsonObject(JsonStringSpace) JsonStringSpace.Name._char[0] = '{'
#define newRootJsonArray(JsonStringSpace) JsonStringSpace.Name._char[0] = '['

#define newClassJsonItem(JsonStringSpace, name, code, ClassType_1, ClassType_2)           \
    do {                                                                                  \
        if (strlen(name) == 0) {                                                          \
            addJsonItemData(JsonStringSpace, ClassType_1);                                \
        } else {                                                                          \
            addJsonItemData(JsonStringSpace, name ":" ClassType_1);                       \
        }                                                                                 \
        code;                                                                             \
        catString(JsonStringSpace.Name._char, ClassType_2, JsonStringSpace.MaxLen, 1);    \
        char *lastAddr = &JsonStringSpace.Name._char[strlen(JsonStringSpace.Name._char)]; \
        swapChr((lastAddr - 1), (lastAddr - 2));                                          \
    } while (0)

#define newSubObjectJsonItem(JsonStringSpace, name, code) newClassJsonItem(JsonStringSpace, name, code, "{", "}")
#define newSubArrayJsonItem(JsonStringSpace, name, code) newClassJsonItem(JsonStringSpace, name, code, "[", "]")

#endif
