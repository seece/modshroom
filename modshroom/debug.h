#ifndef DEBUG_H
#define DEBUG_H

#define dprint(expr) printf(#expr " = %d\n", expr);
#define dprints(_fmt, expr) printf(#expr " = " #_fmt "\n", expr);

#endif