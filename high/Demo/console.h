#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "windows.h"
#include "stdio.h"
#include "memory.h"

enum TColor {BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY,
             DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED,
             LIGHTMAGENTA, YELLOW, WHITE};

int wherex(void);
int wherey(void);
COORD WhereXY(void);
void clrscr(void);
void gotoxy(int x, int y);
void textcolor(int color);
void textbackground(int color);
void textattr(int color);
void setwindow(int width, int height);
void SetWindow(SMALL_RECT rect, COORD size);
void GetWindow(SMALL_RECT *rect, COORD *size);
void HideCursor(void);
void ShowCursor(void);
void SetMaximizedWindow();

#endif
