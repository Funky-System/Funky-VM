//
// Created by Bas du Pré on 18-02-18.
//

#ifndef FUNKY_VM_OS_H
#define FUNKY_VM_OS_H

#if defined(_WIN32) || defined(_WIN64)
#define FUNKY_VM_OS_WINDOWS
#define FUNKY_VM_OS "Windows"
#elif defined(__APPLE__) || defined(__MACH__)
#define FUNKY_VM_OS_MACOS
#define FUNKY_VM_OS "MacOS"
#elif defined(__linux__) || defined(linux) || defined(__linux)
#define FUNKY_VM_OS_LINUX
#define FUNKY_VM_OS "Linux"
#else
#error Funky VM is not supported on this Operating System
#endif

#endif //FUNKY_VM_OS_H