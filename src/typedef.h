#ifndef TYPEDEF_H
#define TYPEDEF_H

const int MAXPATHS = 1000;

enum WIRETYPE { PI, PO, INN };
enum PATHTYPE { LONG, SHORT };
enum AGINGTYPE { DCC_NONE, DCC_S, DCC_F, DCC_M, FF, NORMAL, WORST, BEST };//FF表示Tcq的老化 worst/best為最小,最大老化

#endif
