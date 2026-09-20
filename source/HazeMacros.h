#ifndef MACROS_H
#define MACROS_H

#define PTR_FREE_ASSERT(x) \
    if (!(x) || !*(x)) return;


#ifdef __cplusplus

#define EXPORT_CPP_BEGIN extern "C" {
#define EXPORT_CPP_END   }

#else

#define EXPORT_CPP_BEGIN
#define EXPORT_CPP_END

#endif

#define WINDOWS 0
#define LINUX   0
#define MACOS   0
#define BSD     0

#ifdef _WIN32
    #undef WINDOWS
    #define WINDOWS 1
#endif

#ifdef __linux__
    #undef LINUX
    #define LINUX 1
#endif

#ifdef __APPLE__
    #undef MACOS
    #define MACOS 1
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    #undef BSD
    #define BSD 1
#endif

#endif
