/* based on DynLib.c contributed by Daniel A. Koepke 
 * Reorg, Steve Dekorte, 2003-08-30
 * See _BSDLicense.txt
 */
 
#ifndef DYNLIB_DEFINED
#define DYNLIB_DEFINED 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void DynLibNoArgFunction(void);
typedef void DynLibOneArgFunction(void *arg);

typedef struct
{
	char *path;
	char *initFuncName;
	void *initArg;
	char *freeFuncName;
	void *freeArg;
	char *error;
	void *handle;
	int refCount;
} DynLib;

DynLib *DynLib_new(void);
void DynLib_free(DynLib *self);

void DynLib_setPath_(DynLib *self, const char *path);
char *DynLib_path(DynLib *self);

void DynLib_setInitFuncName_(DynLib *self, const char *name);
char *DynLib_initFuncName(DynLib *self);
void DynLib_setInitArg_(DynLib *self, void *arg);

void DynLib_setFreeFuncName_(DynLib *self, const char *name);
char *DynLib_freeFuncName(DynLib *self);
void DynLib_setFreeArg_(DynLib *self, void *arg);

void DynLib_setError_(DynLib *self, const char *path);
char *DynLib_error(DynLib *self);

void DynLib_open(DynLib *self);
unsigned char DynLib_isOpen(DynLib *self);
void DynLib_close(DynLib *self);
void *DynLib_pointerForSymbolName_(DynLib *self, const char *symbolName);

#ifdef __cplusplus
}
#endif
#endif
