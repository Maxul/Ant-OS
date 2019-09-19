#include "ok.h"

void *memset(void *s, u16 c, i64 n)
{
    u16 *p = s;
    while (n--)
        *p++ = c;
    return s;
}
