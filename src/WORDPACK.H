/*

     / / / / / / / / / /
   / / wordpack.h  / /
 / / / / / / / / / /

         2026
        mpower

  compact Wordell word data
  for Microsoft Windows 1.0

*/

#ifndef WORDPACK_H
#define WORDPACK_H

#define WORD_LENGTH      5
#define ALL_WORD_COUNT   12947
#define SOLUTION_COUNT   2315

int wordpackHasWord (
    char *
);

int wordpackSolution (
    unsigned int,
    char *
);

#endif
