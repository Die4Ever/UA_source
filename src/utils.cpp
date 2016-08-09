#include "includes.h"
#include "utils.h"
#include "inttypes.h"
#include <SDL2/SDL_timer.h>


int read_yes_no_status(const char *file, int result)
{
    char buf[128];

    FILE *fil = fopen(file, "r");
    if ( fil )
    {
        if ( fgets(buf, 128, fil) )
        {
            char *lend = strpbrk(buf, "; \n");
            if ( lend )
                *lend = 0;
            result = strcasecmp(buf, "yes") == 0;
        }
        fclose(fil);
    }
    return result;
}


void sub_4BF181(DWORD sec)
{
    Sleep(sec);
}

float SWAP32F(float f)
{
    uint32_t tmp = *(uint32_t *)&f;
    tmp = SWAP32(tmp);
    return *(float *)&tmp;
}


static HCURSOR cursorCache[11];
static char    cursorNames[11][128];
static int     cursor_num = 0;


HCURSOR uaLoadCursor(HINSTANCE, const char *name)
{
    for(int i = 0; i < cursor_num; i++)
    {
        if (strcasecmp(name, cursorNames[i]) == 0)
        {
            return cursorCache[i];
        }
    }

    char buf[256];
    strcpy(buf, "res/");
    strcat(buf, name);
    strcat(buf, ".cur");

    HCURSOR tmp = LoadCursorFromFile(buf);
    if (!tmp)
        return NULL;

    if (cursor_num < 11)
    {
        cursorCache[cursor_num] = tmp;
        strcpy(cursorNames[cursor_num], name);
        cursor_num++;
    }
    return tmp;
}

HICON uaLoadIcon(HINSTANCE, const char *name)
{
    char buf[256];
    strcpy(buf, "res/");
    strcat(buf, name);
    strcat(buf, ".ico");

    return (HICON) LoadImage(NULL, buf, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
}

int read_reg_key(const CHAR *keyname, char *outBuf, int bufsize)
{
    char Data[260];

    memset(outBuf, 0, bufsize);
    int v5 = 0;
    HKEY phkResult;

    if ( !RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Microsoft Games\\Urban Assault\\1.0", 0, 1, &phkResult) )
    {
        DWORD cbData = 260;
        DWORD Type;
        if ( !RegQueryValueExA(phkResult, keyname, 0, &Type, (LPBYTE)Data, &cbData) && Type == 1 )
        {
            v5 = 1;
            strncpy(outBuf, Data, bufsize - 1);
        }
        RegCloseKey(phkResult);
    }
    return v5;
}

const char *get_lang_string(char **array, int id, const char *def)
{
    const char *v4 = array[id];
    if ( !v4 || !strcmp(v4, "<>") )
        return def;
    return v4;
}


int strnicmp (const char *s1, const char *s2, size_t n)
{
    const char *s2end = s2 + n;

    while (s2 < s2end && *s2 != 0 && toupper(*s1) == toupper(*s2))
        s1++, s2++;
    if (s2end == s2)
        return 0;
    return (int) (toupper(*s1) - toupper(*s2));
}

int dround(float val)
{
    return val + 0.5;
}

int dround(double val)
{
    return val + 0.5;
}


int DO = 0; //Shutup "MAKE ME" screams

void dprintf(const char *fmt, ...)
{
    if (DO)
    {
        va_list args;
        va_start (args, fmt);

        vprintf(fmt, args);

        va_end(args);
    }
}

DWORD profiler_begin()
{
    Uint64 freq = SDL_GetPerformanceFrequency();

    if ( !freq )
        return 0;

    Uint64 cnt = SDL_GetPerformanceCounter();
    return cnt / (freq / 10000);
}

DWORD profiler_end(DWORD prev)
{
    Uint64 freq = SDL_GetPerformanceFrequency();

    if ( !freq )
        return 0 - prev;

    Uint64 cnt = SDL_GetPerformanceCounter();
    return cnt / (freq / 10000) - prev;
}
