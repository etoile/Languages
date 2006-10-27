
#ifdef __cplusplus
extern "C" {
#endif


#ifdef _WIN32

    int truncate(const char *path, long length);
    
#else

    #include <unistd.h>
    
#endif


#ifdef __cplusplus
}
#endif
