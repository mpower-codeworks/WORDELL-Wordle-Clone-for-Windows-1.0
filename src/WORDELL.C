/*

     / / / / / / / / / /
   / /  wordell.c  / / /
 / / / / / / / / / / /

         2026
        mpower

 Microsoft Windows 1.0 port
     Microsoft C 4.0

build with:

    MAKE WORDELL

*/

#include "windows.h"
#include "wordell.h"
#include "wordpack.h"

/* screen states
*/
#define SCREEN_MENU   0
#define SCREEN_GAME   1
#define SCREEN_HELP   2
#define SCREEN_STATS  3
#define SCREEN_RESET  4

/* tile states
*/
#define TILE_EMPTY    0
#define TILE_ABSENT   1
#define TILE_PRESENT  2
#define TILE_CORRECT  3

#define MAX_TRIES     6
#define ALPHA_SIZE    27

/* resource strings copied from the original
** Windows 1.03 HELLO sample structure
*/
char szAppName[16];
char szAbout[16];
char szTitle[16];

/* application instance and About callback
*/
static HANDLE hInst;
static FARPROC lpprocAbout;

/* current UI state
*/
static int screenMode;
static int gameOver;
static int guessRow;
static int guessLen;

/* game state
*/
static char answer[WORD_LENGTH + 1];
static char guesses[MAX_TRIES][WORD_LENGTH + 1];
static unsigned char tiles[MAX_TRIES][WORD_LENGTH];
static char alpha[ALPHA_SIZE];
static char statusText[80];

/* persistent player statistics
** Windows 1.x only has the system profile API,
** so these are stored in a [Wordell] WIN.INI section
*/
static unsigned int gamesPlayed;
static unsigned int gamesWon[MAX_TRIES];
static unsigned int gamesLost;
static char profileName[] = "Wordell";

/* ===================
** function prototypes
** ===================
*/
long FAR PASCAL WordellWndProc (
    HWND,
    unsigned,
    WORD,
    LONG
);

BOOL FAR PASCAL About (
    HWND,
    unsigned,
    WORD,
    LONG
);

BOOL WordellInit (
    HANDLE
);

void drawScreen (
    HWND,
    HDC
);

void drawMenu (
    HDC,
    int,
    int
);

void drawGame (
    HDC,
    int,
    int
);

void drawHelp (
    HDC,
    int,
    int
);

void drawStats (
    HDC,
    int,
    int
);

void drawReset (
    HDC,
    int,
    int
);

void drawCentered (
    HDC,
    int,
    int,
    char *
);

void drawText (
    HDC,
    int,
    int,
    char *
);

void drawTile (
    HDC,
    int,
    int,
    int,
    int,
    int,
    int
);

int textLen (
    char *
);

void textCopy (
    char *,
    char *
);

void textAppend (
    char *,
    char *
);

int textEqual (
    char *,
    char *
);

void uintText (
    unsigned int,
    char *
);

void appendUInt (
    char *,
    unsigned int
);

void setStatus (
    char *
);

void setGuessPrompt (
    void
);

void showMenu (
    HWND
);

void startGame (
    HWND
);

void gameChar (
    HWND,
    int
);

void submitGuess (
    HWND
);

void scoreGuess (
    int
);

void removeAlpha (
    char *
);

void clearCurrentGuess (
    void
);

void statsLoad (
    void
);

void statsSave (
    void
);

void statsReset (
    void
);

void statsRecordWin (
    int
);

void statsRecordLoss (
    void
);

unsigned int statsWins (
    void
);

unsigned int statsPercent (
    unsigned int,
    unsigned int
);

/* =====
** About
** =====
*/
BOOL FAR PASCAL About (hDlg, message, wParam, lParam)
    HWND hDlg;
    unsigned message;
    WORD wParam;
    LONG lParam;
{
    if (message == WM_COMMAND) {
        EndDialog(hDlg, TRUE);
        return TRUE;
    }

    if (message == WM_INITDIALOG) {
        return TRUE;
    }

    return FALSE;
}

/* =======
** textLen
** =======
*/
int textLen (text)
    char *text;
{
    int len;

    len = 0;

    if (text != (char *)0) {
        while (text[len]) {
            ++len;
        }
    }

    return len;
}

/* ========
** textCopy
** ========
*/
void textCopy (dst, src)
    char *dst;
    char *src;
{
    if (dst == (char *)0) {
        return;
    }

    if (src == (char *)0) {
        dst[0] = 0;
        return;
    }

    while (*src) {
        *dst = *src;
        ++dst;
        ++src;
    }

    *dst = 0;
}

/* ==========
** textAppend
** ==========
*/
void textAppend (dst, src)
    char *dst;
    char *src;
{
    if (dst == (char *)0 || src == (char *)0) {
        return;
    }

    while (*dst) {
        ++dst;
    }

    while (*src) {
        *dst = *src;
        ++dst;
        ++src;
    }

    *dst = 0;
}

/* =========
** textEqual
** =========
*/
int textEqual (a, b)
    char *a;
    char *b;
{
    if (a == (char *)0 || b == (char *)0) {
        return 0;
    }

    while (*a && *b) {
        if (*a != *b) {
            return 0;
        }

        ++a;
        ++b;
    }

    return *a == *b;
}

/* ========
** uintText
** ========
*/
void uintText (value, text)
    unsigned int value;
    char *text;
{
    char work[6];
    int len;
    int i;

    if (text == (char *)0) {
        return;
    }

    if (value == 0) {
        text[0] = '0';
        text[1] = 0;
        return;
    }

    len = 0;

    while (value > 0 && len < 5) {
        work[len] = (char)('0' + (value % 10));
        value /= 10;
        ++len;
    }

    i = 0;
    while (len > 0) {
        --len;
        text[i] = work[len];
        ++i;
    }

    text[i] = 0;
}

/* ==========
** appendUInt
** ==========
*/
void appendUInt (text, value)
    char *text;
    unsigned int value;
{
    char number[6];

    uintText(value, number);
    textAppend(text, number);
}

/* =========
** setStatus
** =========
*/
void setStatus (text)
    char *text;
{
    textCopy(statusText, text);
}

/* ==============
** setGuessPrompt
** ==============
*/
void setGuessPrompt () {
    char line[80];

    textCopy(line, "Guess ");
    appendUInt(line, (unsigned int)(guessRow + 1));
    textAppend(line, " of 6   ENTER submits   ESC quits round");
    setStatus(line);
}

/* =========
** statsWins
** =========
*/
unsigned int statsWins () {
    unsigned int total;
    int i;

    total = 0;
    i = 0;

    while (i < MAX_TRIES) {
        total += gamesWon[i];
        ++i;
    }

    return total;
}

/* ============
** statsPercent
** ============
*/
unsigned int statsPercent (value, total)
    unsigned int value;
    unsigned int total;
{
    unsigned long work;

    if (total == 0) {
        return 0;
    }

    work = (unsigned long)value * 100L;
    return (unsigned int)(work / total);
}

/* =========
** statsLoad
** =========
*/
void statsLoad () {
    char key[8];
    int i;

    gamesPlayed = (unsigned int)GetProfileInt(
        (LPSTR)profileName,
        (LPSTR)"GAMES",
        0
    );

    gamesLost = (unsigned int)GetProfileInt(
        (LPSTR)profileName,
        (LPSTR)"LOST",
        0
    );

    i = 0;
    while (i < MAX_TRIES) {
        textCopy(key, "WIN");
        key[3] = (char)('1' + i);
        key[4] = 0;

        gamesWon[i] = (unsigned int)GetProfileInt(
            (LPSTR)profileName,
            (LPSTR)key,
            0
        );

        ++i;
    }
}

/* =========
** statsSave
** =========
*/
void statsSave () {
    char key[8];
    char value[8];
    int i;

    uintText(gamesPlayed, value);
    WriteProfileString(
        (LPSTR)profileName,
        (LPSTR)"GAMES",
        (LPSTR)value
    );

    uintText(gamesLost, value);
    WriteProfileString(
        (LPSTR)profileName,
        (LPSTR)"LOST",
        (LPSTR)value
    );

    i = 0;
    while (i < MAX_TRIES) {
        textCopy(key, "WIN");
        key[3] = (char)('1' + i);
        key[4] = 0;

        uintText(gamesWon[i], value);

        WriteProfileString(
            (LPSTR)profileName,
            (LPSTR)key,
            (LPSTR)value
        );

        ++i;
    }
}

/* ==========
** statsReset
** ==========
*/
void statsReset () {
    int i;

    gamesPlayed = 0;
    gamesLost = 0;

    i = 0;
    while (i < MAX_TRIES) {
        gamesWon[i] = 0;
        ++i;
    }

    statsSave();
}

/* ==============
** statsRecordWin
** ==============
*/
void statsRecordWin (tries)
    int tries;
{
    ++gamesPlayed;

    if (tries >= 1 && tries <= MAX_TRIES) {
        ++gamesWon[tries - 1];
    }

    statsSave();
}

/* ===============
** statsRecordLoss
** =================
*/
void statsRecordLoss () {
    ++gamesPlayed;
    ++gamesLost;
    statsSave();
}

/* ============
** drawCentered
** ============
*/
void drawCentered (hDC, width, y, text)
    HDC hDC;
    int width;
    int y;
    char *text;
{
    DWORD extent;
    int len;
    int x;

    len = textLen(text);
    if (len <= 0) {
        return;
    }

    extent = GetTextExtent(
        hDC,
        (LPSTR)text,
        (short)len
    );

    x = (width - (int)LOWORD(extent)) / 2;
    if (x < 0) {
        x = 0;
    }

    TextOut(
        hDC,
        (short)x,
        (short)y,
        (LPSTR)text,
        (short)len
    );
}

/* ========
** drawText
** ========
*/
void drawText (hDC, x, y, text)
    HDC hDC;
    int x;
    int y;
    char *text;
{
    int len;

    len = textLen(text);
    if (len <= 0) {
        return;
    }

    TextOut(
        hDC,
        (short)x,
        (short)y,
        (LPSTR)text,
        (short)len
    );
}

/* ========
** drawTile
** ========
*/
void drawTile (hDC, x, y, width, height, state, ch)
    HDC hDC;
    int x;
    int y;
    int width;
    int height;
    int state;
    int ch;
{
    RECT rect;
    HBRUSH brush;
    HBRUSH frameBrush;
    DWORD extent;
    char letter[2];
    int tx;
    int ty;

    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;

    if (state == TILE_CORRECT) {
        brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    } else if (state == TILE_PRESENT) {
        brush = (HBRUSH)GetStockObject(GRAY_BRUSH);
    } else if (state == TILE_ABSENT) {
        brush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    } else {
        brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    }

    frameBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

    FillRect(hDC, (LPRECT)&rect, brush);
    FrameRect(hDC, (LPRECT)&rect, frameBrush);

    if (ch == 0) {
        return;
    }

    letter[0] = (char)ch;
    letter[1] = 0;

    extent = GetTextExtent(
        hDC,
        (LPSTR)letter,
        (short)1
    );

    tx = x + ((width - (int)LOWORD(extent)) / 2);
    ty = y + ((height - (int)HIWORD(extent)) / 2);

    SetBkMode(hDC, TRANSPARENT);

    if (state == TILE_CORRECT) {
        SetTextColor(hDC, RGB(255, 255, 255));
    } else {
        SetTextColor(hDC, RGB(0, 0, 0));
    }

    TextOut(
        hDC,
        (short)tx,
        (short)ty,
        (LPSTR)letter,
        (short)1
    );

    SetTextColor(hDC, RGB(0, 0, 0));
}

/* ========
** drawMenu
** ========
*/
void drawMenu (hDC, width, height)
    HDC hDC;
    int width;
    int height;
{
    int y;

    y = height / 5;

    drawCentered(hDC, width, y, "W O R D E L L");
    drawCentered(
        hDC,
        width,
        y + 24,
        "for Microsoft Windows 1.x"
    );

    drawCentered(hDC, width, y +  70, "(S)tart new game");
    drawCentered(hDC, width, y +  88, "(H)ow to play");
    drawCentered(hDC, width, y + 106, "(P)layer stats");
    drawCentered(hDC, width, y + 142, "(Q)uit Wordell");
}

/* ========
** drawGame
** ========
*/
void drawGame (hDC, width, height)
    HDC hDC;
    int width;
    int height;
{
    int cellWidth;
    int cellHeight;
    int gapX;
    int gapY;
    int boardWidth;
    int boardHeight;
    int x0;
    int y0;
    int row;
    int col;
    int ch;
    char alphaLine[40];

    /* keep it tight to fit on screen
    */
    gapX = 4;
    gapY = 2;

    cellWidth = (width - 80 - (gapX * 4)) / 5;
    if (cellWidth > 52) {
        cellWidth = 52;
    }
    if (cellWidth < 24) {
        cellWidth = 24;
    }

    cellHeight = (height - 70 - (gapY * 5)) / 6;
    if (cellHeight > 16) {
        cellHeight = 16;
    }
    if (cellHeight < 12) {
        cellHeight = 12;
    }

    boardWidth = (cellWidth * 5) + (gapX * 4);
    boardHeight = (cellHeight * 6) + (gapY * 5);

    x0 = (width - boardWidth) / 2;
    y0 = 36;

    drawCentered(hDC, width, 2, "W O R D E L L");
    drawCentered(hDC, width, 19, statusText);

    row = 0;
    while (row < MAX_TRIES) {
        col = 0;

        while (col < WORD_LENGTH) {
            ch = guesses[row][col];

            drawTile(
                hDC,
                x0 + (col * (cellWidth + gapX)),
                y0 + (row * (cellHeight + gapY)),
                cellWidth,
                cellHeight,
                tiles[row][col],
                ch
            );

            ++col;
        }

        ++row;
    }

    textCopy(alphaLine, "Unused: ");
    textAppend(alphaLine, alpha);

    drawCentered(
        hDC,
        width,
        y0 + boardHeight + 6,
        alphaLine
    );

    if (gameOver) {
        drawCentered(
            hDC,
            width,
            y0 + boardHeight + 20,
            "Press any key to return to the main screen"
        );
    }
}

/* ========
** drawHelp
** ========
*/
void drawHelp (hDC, width, height)
    HDC hDC;
    int width;
    int height;
{
    int labelX;
    int textX;

    labelX = (width / 2) - 155;
    textX = (width / 2) - 55;

    if (labelX < 4) {
        labelX = 4;
    }
    if (textX < 104) {
        textX = 104;
    }

    drawCentered(hDC, width, 4, "WORDELL - HOW TO PLAY");

    drawCentered(
        hDC,
        width,
        28,
        "Guess the five-letter word within six tries."
    );

    drawCentered(
        hDC,
        width,
        48,
        "After each guess the tiles show:"
    );

    drawText(hDC, labelX, 68, "BLACK");
    drawText(hDC, textX, 68, "correct letter and position");

    drawText(hDC, labelX, 84, "GRAY");
    drawText(hDC, textX, 84, "correct letter, wrong position");

    drawText(hDC, labelX, 100, "LIGHT GRAY");
    drawText(hDC, textX, 100, "letter not in the word");

    drawCentered(
        hDC,
        width,
        120,
        "Unused letters are shown below the board."
    );

    drawCentered(
        hDC,
        width,
        136,
        "ESC during a round returns to the main screen."
    );

    drawCentered(
        hDC,
        width,
        height - 14,
        "Press any key"
    );
}

/* =========
** drawStats
** =========
*/
void drawStats (hDC, width, height)
    HDC hDC;
    int width;
    int height;
{
    char line[80];
    unsigned int wins;
    int i;
    int mainX;
    int leftX;
    int rightX;

    wins = statsWins();

    mainX = (width / 2) - 70;
    leftX = (width / 2) - 145;
    rightX = (width / 2) + 20;

    if (mainX < 4) {
        mainX = 4;
    }
    if (leftX < 4) {
        leftX = 4;
    }

    drawCentered(hDC, width, 4, "PLAYER STATS");
    drawCentered(hDC, width, 14, "------------");

    textCopy(line, "Games played: ");
    appendUInt(line, gamesPlayed);
    drawText(hDC, mainX, 22, line);

    textCopy(line, "Games won: ");
    appendUInt(line, wins);
    textAppend(line, " (");
    appendUInt(line, statsPercent(wins, gamesPlayed));
    textAppend(line, "%)");
    drawText(hDC, mainX, 38, line);

    textCopy(line, "Games lost: ");
    appendUInt(line, gamesLost);
    textAppend(line, " (");
    appendUInt(line, statsPercent(gamesLost, gamesPlayed));
    textAppend(line, "%)");
    drawText(hDC, mainX, 54, line);

    drawCentered(hDC, width, 74, "Wins by guess level");

    i = 0;
    while (i < 3) {
        textCopy(line, "Guess ");
        appendUInt(line, (unsigned int)(i + 1));
        textAppend(line, ": ");
        appendUInt(line, gamesWon[i]);
        textAppend(line, " (");
        appendUInt(line, statsPercent(gamesWon[i], wins));
        textAppend(line, "%)");

        drawText(
            hDC,
            leftX,
            92 + (i * 16),
            line
        );

        textCopy(line, "Guess ");
        appendUInt(line, (unsigned int)(i + 4));
        textAppend(line, ": ");
        appendUInt(line, gamesWon[i + 3]);
        textAppend(line, " (");
        appendUInt(line, statsPercent(gamesWon[i + 3], wins));
        textAppend(line, "%)");

        drawText(
            hDC,
            rightX,
            92 + (i * 16),
            line
        );

        ++i;
    }

    drawCentered(
        hDC,
        width,
        height - 30,
        "(R)eset statistics"
    );

    drawCentered(
        hDC,
        width,
        height - 14,
        "Press any other key to return"
    );
}

/* =========
** drawReset
** =========
*/
void drawReset (hDC, width, height)
    HDC hDC;
    int width;
    int height;
{
    drawCentered(
        hDC,
        width,
        (height / 2) - 18,
        "Reset all player statistics?"
    );

    drawCentered(
        hDC,
        width,
        (height / 2) + 8,
        "Press Y to erase or any other key to cancel"
    );
}

/* ==========
** drawScreen
** ==========
*/
void drawScreen (hWnd, hDC)
    HWND hWnd;
    HDC hDC;
{
    RECT rect;
    int width;
    int height;

    GetClientRect(hWnd, (LPRECT)&rect);

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(0, 0, 0));

    if (screenMode == SCREEN_GAME) {
        drawGame(hDC, width, height);
    } else if (screenMode == SCREEN_HELP) {
        drawHelp(hDC, width, height);
    } else if (screenMode == SCREEN_STATS) {
        drawStats(hDC, width, height);
    } else if (screenMode == SCREEN_RESET) {
        drawReset(hDC, width, height);
    } else {
        drawMenu(hDC, width, height);
    }
}

/* =================
** clearCurrentGuess
** =================
*/
void clearCurrentGuess () {
    int col;

    col = 0;

    while (col < WORD_LENGTH) {
        guesses[guessRow][col] = 0;
        tiles[guessRow][col] = TILE_EMPTY;
        ++col;
    }

    guesses[guessRow][WORD_LENGTH] = 0;
    guessLen = 0;
}

/* ===========
** removeAlpha
** ===========
*/
void removeAlpha (guess)
    char *guess;
{
    int i;
    int j;

    i = 0;

    while (i < WORD_LENGTH) {
        j = 0;

        while (j < 26) {
            if (alpha[j] == guess[i]) {
                alpha[j] = '_';
                break;
            }

            ++j;
        }

        ++i;
    }
}

/* ==========
** scoreGuess
** ==========
*/
void scoreGuess (row)
    int row;
{
    char copy[WORD_LENGTH + 1];
    int i;
    int j;

    textCopy(copy, answer);

    i = 0;
    while (i < WORD_LENGTH) {
        if (guesses[row][i] == copy[i]) {
            tiles[row][i] = TILE_CORRECT;
            copy[i] = '_';
        } else {
            tiles[row][i] = TILE_ABSENT;
        }

        ++i;
    }

    i = 0;
    while (i < WORD_LENGTH) {
        if (tiles[row][i] != TILE_CORRECT) {
            j = 0;

            while (j < WORD_LENGTH) {
                if (copy[j] == guesses[row][i]) {
                    tiles[row][i] = TILE_PRESENT;
                    copy[j] = '_';
                    break;
                }

                ++j;
            }
        }

        ++i;
    }
}

/* ===========
** submitGuess
** ===========
*/
void submitGuess (hWnd)
    HWND hWnd;
{
    char line[80];

    if (guessLen != WORD_LENGTH) {
        setStatus("Must be five letters");
        MessageBeep(0);
        InvalidateRect(hWnd, (LPRECT)0, TRUE);
        return;
    }

    guesses[guessRow][WORD_LENGTH] = 0;

    /* the six solution words absent from the master
    ** validation list must still be accepted when
    ** they are the answer
    */
    if (
        !textEqual(guesses[guessRow], answer) &&
        !wordpackHasWord(guesses[guessRow])) {

        clearCurrentGuess();
        setStatus("Word not in list");
        MessageBeep(0);
        InvalidateRect(hWnd, (LPRECT)0, TRUE);
        return;
    }

    scoreGuess(guessRow);
    removeAlpha(guesses[guessRow]);

    if (textEqual(guesses[guessRow], answer)) {
        statsRecordWin(guessRow + 1);

        textCopy(line, "You win! The word is ");
        textAppend(line, answer);
        setStatus(line);

        gameOver = 1;
        InvalidateRect(hWnd, (LPRECT)0, TRUE);
        return;
    }

    ++guessRow;
    guessLen = 0;

    if (guessRow >= MAX_TRIES) {
        statsRecordLoss();

        textCopy(line, "Better luck next time. The word was ");
        textAppend(line, answer);
        setStatus(line);

        gameOver = 1;
        InvalidateRect(hWnd, (LPRECT)0, TRUE);
        return;
    }

    setGuessPrompt();
    InvalidateRect(hWnd, (LPRECT)0, TRUE);
}

/* =========
** startGame
** =========
*/
void startGame (hWnd)
    HWND hWnd;
{
    unsigned long seed;
    unsigned int game;
    int row;
    int col;

    seed = (unsigned long)GetCurrentTime();
    game = (unsigned int)(seed % SOLUTION_COUNT);

    if (!wordpackSolution(game, answer)) {
        textCopy(answer, "TESTS");
    }

    row = 0;
    while (row < MAX_TRIES) {
        col = 0;

        while (col < WORD_LENGTH) {
            guesses[row][col] = 0;
            tiles[row][col] = TILE_EMPTY;
            ++col;
        }

        guesses[row][WORD_LENGTH] = 0;
        ++row;
    }

    textCopy(alpha, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    guessRow = 0;
    guessLen = 0;
    gameOver = 0;
    screenMode = SCREEN_GAME;

    setGuessPrompt();

    InvalidateRect(hWnd, (LPRECT)0, TRUE);
}

/* ========
** showMenu
** ========
*/
void showMenu (hWnd)
    HWND hWnd;
{
    screenMode = SCREEN_MENU;
    gameOver = 0;
    InvalidateRect(hWnd, (LPRECT)0, TRUE);
}

/* ========
** gameChar
** ========
*/
void gameChar (hWnd, ch)
    HWND hWnd;
    int ch;
{
    if (gameOver) {
        showMenu(hWnd);
        return;
    }

    if (ch == 27) {
        showMenu(hWnd);
        return;
    }

    if (ch == 8 || ch == 127) {
        if (guessLen > 0) {
            --guessLen;
            guesses[guessRow][guessLen] = 0;
            setGuessPrompt();
            InvalidateRect(hWnd, (LPRECT)0, FALSE);
        }
        return;
    }

    if (ch == '\r' || ch == '\n') {
        submitGuess(hWnd);
        return;
    }

    if (ch >= 'a' && ch <= 'z') {
        ch &= ~0x20;
    }

    if (ch >= 'A' && ch <= 'Z') {
        if (guessLen < WORD_LENGTH) {
            guesses[guessRow][guessLen] = (char)ch;
            ++guessLen;
            guesses[guessRow][guessLen] = 0;

            setGuessPrompt();
            InvalidateRect(hWnd, (LPRECT)0, FALSE);
        } else {
            MessageBeep(0);
        }
    }
}

/* ===========
** WordellInit
** ===========
** procedure called when the application
** is loaded for the first time
*/
BOOL WordellInit (hInstance)
    HANDLE hInstance;
{
    PWNDCLASS pClass;

    LoadString(
        hInstance,
        IDSNAME,
        (LPSTR)szAppName,
        sizeof(szAppName)
    );

    LoadString(
        hInstance,
        IDSABOUT,
        (LPSTR)szAbout,
        sizeof(szAbout)
    );

    LoadString(
        hInstance,
        IDSTITLE,
        (LPSTR)szTitle,
        sizeof(szTitle)
    );

    pClass = (PWNDCLASS)LocalAlloc(
        LPTR,
        sizeof(WNDCLASS)
    );

    if (pClass == (PWNDCLASS)0) {
        return FALSE;
    }

    pClass->hCursor =
        LoadCursor((HANDLE)0, IDC_ARROW);

    pClass->hIcon =
        LoadIcon(
            hInstance,
            MAKEINTRESOURCE(WORDELLICON)
        );

    pClass->lpszMenuName = (LPSTR)0;
    pClass->lpszClassName = (LPSTR)szAppName;

    pClass->hbrBackground =
        (HBRUSH)GetStockObject(WHITE_BRUSH);

    pClass->hInstance = hInstance;
    pClass->style = CS_HREDRAW | CS_VREDRAW;
    pClass->lpfnWndProc = WordellWndProc;

    if (!RegisterClass((LPWNDCLASS)pClass)) {
        return FALSE;
    }

    LocalFree((HANDLE)pClass);
    return TRUE;
}

/* =============
** program entry
** =============
*/
int PASCAL WinMain (hInstance, hPrevInstance, lpszCmdLine, cmdShow)
    HANDLE hInstance;
    HANDLE hPrevInstance;
    LPSTR lpszCmdLine;
    int cmdShow;
{
    MSG msg;
    HWND hWnd;
    HMENU hMenu;

    if (!hPrevInstance) {
        if (!WordellInit(hInstance)) {
            return FALSE;
        }
    } else {
        GetInstanceData(
            hPrevInstance,
            (PSTR)szAppName,
            sizeof(szAppName)
        );

        GetInstanceData(
            hPrevInstance,
            (PSTR)szAbout,
            sizeof(szAbout)
        );

        GetInstanceData(
            hPrevInstance,
            (PSTR)szTitle,
            sizeof(szTitle)
        );
    }

    hWnd = CreateWindow(
        (LPSTR)szAppName,
        (LPSTR)szTitle,
        WS_TILEDWINDOW,
        0,
        0,
        0,
        0,
        (HWND)0,
        (HMENU)0,
        hInstance,
        (LPSTR)0
    );

    if (hWnd == (HWND)0) {
        return FALSE;
    }

    hInst = hInstance;

    lpprocAbout = MakeProcInstance(
        (FARPROC)About,
        hInstance
    );

    hMenu = GetSystemMenu(hWnd, FALSE);

    ChangeMenu(
        hMenu,
        0,
        (LPSTR)0,
        999,
        MF_APPEND | MF_SEPARATOR
    );

    ChangeMenu(
        hMenu,
        0,
        (LPSTR)szAbout,
        IDSABOUT,
        MF_APPEND | MF_STRING
    );

    statsLoad();
    screenMode = SCREEN_MENU;

    ShowWindow(hWnd, cmdShow);
    UpdateWindow(hWnd);

    while (GetMessage((LPMSG)&msg, (HWND)0, 0, 0)) {
        TranslateMessage((LPMSG)&msg);
        DispatchMessage((LPMSG)&msg);
    }

    return (int)msg.wParam;
}

/* ==============
** WordellWndProc
** ==============
*/
long FAR PASCAL WordellWndProc (hWnd, message, wParam, lParam)
    HWND hWnd;
    unsigned message;
    WORD wParam;
    LONG lParam;
{
    PAINTSTRUCT ps;
    int ch;

    switch (message) {
    case WM_SYSCOMMAND:
        if (wParam == IDSABOUT) {
            DialogBox(
                hInst,
                MAKEINTRESOURCE(ABOUTBOX),
                hWnd,
                lpprocAbout
            );

            return 0L;
        }

        return DefWindowProc(
            hWnd,
            message,
            wParam,
            lParam
        );

    case WM_CHAR:
        ch = (int)(wParam & 0x00FF);

        if (screenMode == SCREEN_GAME) {
            gameChar(hWnd, ch);
            return 0L;
        }

        if (ch >= 'a' && ch <= 'z') {
            ch &= ~0x20;
        }

        if (screenMode == SCREEN_HELP) {
            showMenu(hWnd);
            return 0L;
        }

        if (screenMode == SCREEN_STATS) {
            if (ch == 'R') {
                screenMode = SCREEN_RESET;
                InvalidateRect(hWnd, (LPRECT)0, TRUE);
            } else {
                showMenu(hWnd);
            }

            return 0L;
        }

        if (screenMode == SCREEN_RESET) {
            if (ch == 'Y') {
                statsReset();
            }

            screenMode = SCREEN_STATS;
            InvalidateRect(hWnd, (LPRECT)0, TRUE);
            return 0L;
        }

        if (ch == 'S') {
            startGame(hWnd);
        } else if (ch == 'H') {
            screenMode = SCREEN_HELP;
            InvalidateRect(hWnd, (LPRECT)0, TRUE);
        } else if (ch == 'P') {
            screenMode = SCREEN_STATS;
            InvalidateRect(hWnd, (LPRECT)0, TRUE);
        } else if (ch == 'Q' || ch == 27) {
            DestroyWindow(hWnd);
        }

        return 0L;

    case WM_PAINT:
        BeginPaint(
            hWnd,
            (LPPAINTSTRUCT)&ps
        );

        drawScreen(hWnd, ps.hdc);

        EndPaint(
            hWnd,
            (LPPAINTSTRUCT)&ps
        );

        return 0L;

    case WM_DESTROY:
        if (lpprocAbout != (FARPROC)0) {
            FreeProcInstance(lpprocAbout);
        }

        PostQuitMessage(0);
        return 0L;
    }

    return DefWindowProc(
        hWnd,
        message,
        wParam,
        lParam
    );
}
