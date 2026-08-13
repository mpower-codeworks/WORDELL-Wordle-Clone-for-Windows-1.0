/*

     / / / / / / / / / /
   / / wordpack.c / / /
 / / / / / / / / / /

         2026
        mpower

 Microsoft Windows 1.0 port
     Microsoft C 4.0

the DOS and Apple II ports keep the word lists in
five-byte packed files.  windows 1.x has a 64K small
model data limit, so this port stores the same words
in a smaller form

ALL words:
    first letter is implied by its group
    remaining four letters use five bits each
    one word = three bytes

SOLutions:
    all five letters use five bits each
    one word = four bytes

the generated data is split across several C files
to keep each source unit reasonable for a 1986 compiler

*/

#include "wordpack.h"

extern unsigned char packA[];
extern unsigned char packB[];
extern unsigned char packC[];
extern unsigned char packD[];
extern unsigned char packE[];
extern unsigned char packF[];
extern unsigned char packG[];
extern unsigned char packH[];
extern unsigned char packI[];
extern unsigned char packJ[];
extern unsigned char packK[];
extern unsigned char packL[];
extern unsigned char packM[];
extern unsigned char packN[];
extern unsigned char packO[];
extern unsigned char packP[];
extern unsigned char packQ[];
extern unsigned char packR[];
extern unsigned char packS[];
extern unsigned char packT[];
extern unsigned char packU[];
extern unsigned char packV[];
extern unsigned char packW[];
extern unsigned char packX[];
extern unsigned char packY[];
extern unsigned char packZ[];

extern unsigned char solPacked[];

/* number of accepted words in each first-letter group
*/
static unsigned int allCount[26] = {
    736, 908, 920, 681, 303, 595, 637,
    488, 165, 202, 375, 575, 693, 325,
    262, 857, 78, 628, 1560, 815, 189,
    242, 411, 16, 181, 105
};

/* pointers to the generated first-letter groups
*/
static unsigned char *allGroup[26] = {
    packA, packB, packC, packD, packE, packF, packG,
    packH, packI, packJ, packK, packL, packM, packN,
    packO, packP, packQ, packR, packS, packT, packU,
    packV, packW, packX, packY, packZ
};

/* ==========
** packLetter
** ==========
** converts one ASCII letter to 0-25
*/
static int packLetter (ch)
    int ch;
{
    if (ch >= 'a' && ch <= 'z') {
        ch &= ~0x20;
    }

    if (ch < 'A' || ch > 'Z') {
        return -1;
    }

    return ch - 'A';
}

/* ===============
** wordpackHasWord
** ===============
** checks one five-letter guess against
** the complete accepted-word list
*/
int wordpackHasWord (word)
    char *word;
{
    int group;
    int value;
    unsigned int i;
    unsigned int offset;
    unsigned long code;

    if (word == (char *)0) {
        return 0;
    }

    group = packLetter(word[0]);
    if (group < 0) {
        return 0;
    }

    code = 0L;

    value = packLetter(word[1]);
    if (value < 0) {
        return 0;
    }
    code |= (unsigned long)value;

    value = packLetter(word[2]);
    if (value < 0) {
        return 0;
    }
    code |= ((unsigned long)value << 5);

    value = packLetter(word[3]);
    if (value < 0) {
        return 0;
    }
    code |= ((unsigned long)value << 10);

    value = packLetter(word[4]);
    if (value < 0) {
        return 0;
    }
    code |= ((unsigned long)value << 15);

    if (word[5] != 0) {
        return 0;
    }

    i = 0;
    offset = 0;

    while (i < allCount[group]) {
        if (
            allGroup[group][offset] ==
                (unsigned char)(code & 0xFF) &&
            allGroup[group][offset + 1] ==
                (unsigned char)((code >> 8) & 0xFF) &&
            allGroup[group][offset + 2] ==
                (unsigned char)((code >> 16) & 0xFF)) {

            return 1;
        }

        offset += 3;
        ++i;
    }

    return 0;
}

/* ================
** wordpackSolution
** ================
** expands one packed solution to uppercase ASCII
*/
int wordpackSolution (index, word)
    unsigned int index;
    char *word;
{
    unsigned int offset;
    unsigned long code;
    int i;

    if (word == (char *)0 || index >= SOLUTION_COUNT) {
        return 0;
    }

    offset = index * 4;

    code =
        (unsigned long)solPacked[offset] |
        ((unsigned long)solPacked[offset + 1] << 8) |
        ((unsigned long)solPacked[offset + 2] << 16) |
        ((unsigned long)solPacked[offset + 3] << 24);

    i = 0;
    while (i < WORD_LENGTH) {
        word[i] = (char)('A' + (int)(code & 31L));
        code >>= 5;
        ++i;
    }

    word[WORD_LENGTH] = 0;
    return 1;
}
