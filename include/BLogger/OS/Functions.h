#pragma once

#include <stdio.h>
#include <time.h>

#ifdef _WIN32
    #define UPDATE_TIME(to, from) localtime_s(&to, &from)
    #ifdef BLOGGER_UNICODE_MODE
        #define OPEN_FILE(file, path) _wfopen_s(&file, path.c_str(), BLOGGER_FILEMODE)
    #else
        #define OPEN_FILE(file, path) fopen_s(&file, path.c_str(), BLOGGER_FILEMODE)
    #endif
    #define MEMORY_COPY(dst, dst_size, src, src_size) memcpy_s(dst, (dst_size) * sizeof(bl_char), src, (src_size) * sizeof(bl_char))
    #define MEMORY_MOVE(dst, dst_size, src, src_size) memmove_s(dst, (dst_size) * sizeof(bl_char), src, (src_size) * sizeof(bl_char))

    // A windows-only safer version of alloca that
    // can allocate on the heap as well. Requires freeing.
    struct anonymous_stack_allocator
    {
    public:
        void* buffer;
    
        anonymous_stack_allocator(size_t size)
            : buffer(nullptr)
        {
            buffer = _malloca(size);
        }
    
        ~anonymous_stack_allocator()
        {
            _freea(buffer);
        }
    };
    
    #define STACK_ALLOC(size, out_ptr) anonymous_stack_allocator anonbuf_##out_ptr((size) * sizeof(bl_char)); \
                                       out_ptr = static_cast<decltype(out_ptr)>(anonbuf_##out_ptr.buffer)

    #ifdef BLOGGER_UNICODE_MODE
        #include "BLogger/Core.h"
        #include <io.h>
        #include <fcntl.h>
        #include <stringapiset.h>

        namespace BLogger {
            static inline void init_unicode()
            {
                auto ignored = _setmode(_fileno(stdout), _O_U16TEXT);
                ignored =      _setmode(_fileno(stdin), _O_U16TEXT);
                ignored =      _setmode(_fileno(stderr), _O_U16TEXT);
            }

            static inline void unicode_file_write(bl_char* data, size_t size, FILE* file)
            {
                size_t byte_limit = size * 2;
                char* narrow; STACK_ALLOC(byte_limit, narrow);
                    auto result =
                    WideCharToMultiByte(
                        CP_UTF8, NULL,
                        data,
                        static_cast<int32_t>(size),
                        narrow,
                        static_cast<int32_t>(byte_limit),
                        NULL, NULL
                    );
                if (!result) throw ""; // handle this later
                fwrite(narrow, 1, result, file);
            }
        }
        #define FILE_WRITE(data, size, file) ::BLogger::unicode_file_write(data, size, file)
        #define INIT_UNICODE_MODE ::BLogger::init_unicode
    #else
        static inline void blogger_dummy_func() {}
        #define INIT_UNICODE_MODE blogger_dummy_func
        #define FILE_WRITE(data, size, file) fwrite(data, 1, size, file)
    #endif
#elif defined(__linux__)
    #include <cstring>
    #include <algorithm>
    #include <clocale>

    #define UPDATE_TIME(to, from) localtime_r(&from, &to)
    #define STACK_ALLOC(size, out_ptr) out_ptr = static_cast<decltype(out_ptr)>(alloca((size) * sizeof(bl_char)))

    #ifdef BLOGGER_UNICODE_MODE
        #include "BLogger/Core.h"
        namespace BLogger {

            static inline const char* wide_to_narrow_unfreed(const bl_char* wide)
            {
                constexpr size_t filename_size = 128;
                char* narrow = new char[filename_size];
                auto result = wcstombs(narrow, wide, filename_size);
                narrow[result] = '\0';
                if(result == -1) throw ""; // TODO: handle this later
                return narrow;
            }

            static inline void open_unicode_file(FILE*& out_file, const bl_char* path)
            {
                auto free_me = wide_to_narrow_unfreed(path);
                auto fptr = fopen(free_me, BLOGGER_FILEMODE);
                delete[] free_me;
                out_file = fptr;
            }

            static inline void unicode_file_write(bl_char* data, size_t size, FILE* file)
            {
                size_t byte_limit = size * 2;
                char* narrow; STACK_ALLOC(byte_limit, narrow);
                auto result = wcstombs(narrow, data, byte_limit);
                if (result == -1) throw ""; // TODO: handle this later
                fwrite(narrow, 1, result, file);
            }
        }
        #define FILE_WRITE(data, size, file) ::BLogger::unicode_file_write(data, size, file)
        #define OPEN_FILE(file, path) ::BLogger::open_unicode_file(file, path.c_str())
    #else
        #define OPEN_FILE(file, path) file = fopen(path.c_str(), BLOGGER_FILEMODE)
        #define FILE_WRITE(data, size, file) fwrite(data, 1, size, file)
    #endif
    #define MEMORY_COPY(dst, dst_size, src, src_size) memcpy(dst, src, (src_size) * sizeof(bl_char))
    #define MEMORY_MOVE(dst, dst_size, src, src_size) memmove(dst, src, (src_size) * sizeof(bl_char))

    #ifdef BLOGGER_UNICODE_MODE
        namespace BLogger {
            static inline void init_unicode()
            {
                setlocale(LC_ALL, "en_US.utf8");
            }
        }
        #define INIT_UNICODE_MODE init_unicode
    #else
        static inline void blogger_dummy_func() {}
        #define INIT_UNICODE_MODE blogger_dummy_func
    #endif
#else
    #error Sorry, your platform is currently not supported! Please let me know it you think it should be.
#endif
