#ifndef __FILE
#define __FILE 1

__hidden char               *__drawable_FileKey(const char *file);
__hidden char               *__drawable_FileRealFile(const char *file);
__hidden char               *__drawable_FileExtension(const char *file);
__hidden int                 __drawable_FileExists(const char *s);
__hidden int                 __drawable_FileIsFile(const char *s);
__hidden int                 __drawable_FileIsDir(const char *s);
__hidden char              **__drawable_FileDir(char *dir, int *num);
__hidden void                __drawable_FileFreeDirList(char **l, int num);
__hidden void                __drawable_FileDel(char *s);
__hidden time_t              __drawable_FileModDate(const char *s);
__hidden char               *__drawable_FileHomeDir(int uid);
__hidden char               *__drawable_FileField(char *s, int field);
__hidden int                 __drawable_FilePermissions(const char *s);
__hidden int                 __drawable_FileCanRead(const char *s);
__hidden int                 __drawable_IsRealFile(const char *s);

#endif
