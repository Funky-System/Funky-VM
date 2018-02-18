#include "funkyvm/funkyvm.h"
#include "os.h"

#ifdef FUNKY_VM_OS_MACOS
#include <mach-o/dyld.h>
#include <limits.h>
#include <string.h>

static const char path_separator =
#ifdef _WIN32
        '\\';
#else
        '/';
#endif

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
char* get_executable_path() {

}
#endif

#ifdef FUNKY_VM_OS_LINUX
char* get_executable_path() {

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