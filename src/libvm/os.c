#include <string.h>

#include "funkyvm/funkyvm.h"
#include "os.h"

static const char path_separator =
#ifdef _WIN32
        '\\';
#else
        '/';
#endif

#ifdef FUNKY_VM_OS_MACOS
#include <mach-o/dyld.h>
#include <limits.h>
#include <string.h>

char* get_executable_filepath() {
    char path[PATH_MAX];
    uint32_t size = PATH_MAX;
    _NSGetExecutablePath(path, &size);
    char path2[PATH_MAX];
    realpath(path, path2);
    return strdup(path2);
}
#endif

#ifdef FUNKY_VM_OS_WINDOWS
#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char* get_executable_filepath() {
    DWORD last_error;
    DWORD result;
    DWORD path_size = 1024;
    char* path      = malloc(1024);

    for (;;)
    {
        memset(path, 0, path_size);
        result     = GetModuleFileName(0, path, path_size - 1);
        last_error = GetLastError();

        if (0 == result)
        {
            free(path);
            path = 0;
            return NULL;
        }
        else if (result == path_size - 1)
        {
            free(path);
            /* May need to also check for ERROR_SUCCESS here if XP/2K */
            if (ERROR_INSUFFICIENT_BUFFER != last_error)
            {
                path = 0;
                free(path);
                return NULL;
            }
            path_size = path_size * 2;
            path = malloc(path_size);
        }
        else
        {
            return path;
        }
    }

    /*if (!path)
    {
        fprintf(stderr, "Failure: %d\n", last_error);
    }
    else
    {
        printf("path=%s\n", path);
    }*/

}
#endif

#ifdef FUNKY_VM_OS_LINUX
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

char* get_executable_filepath() {
    char path[PATH_MAX];
    char dest[PATH_MAX];
    memset(dest, 0, sizeof(dest)); // readlink does not null terminate!
    struct stat info;
    pid_t pid = getpid();
    sprintf(path, "/proc/%d/exe", pid);
    if (readlink(path, dest, PATH_MAX) == -1) {
        perror("readlink");
    } else {
        return strdup(dest);
    }
    return NULL;
}
#endif

#ifdef FUNKY_VM_OS_EMSCRIPTEN
char* get_executable_filepath() {
    return strdup("");
}
#endif

char* get_executable_path(const char* append) {
    char* path = get_executable_filepath();
    char* ret = malloc(strlen(path) + strlen(append) + 1);
    strcpy(ret, path);
    strcpy(strrchr(ret, path_separator) + 1, append);
    free(path);
    return ret;
}