


#pragma once

#include <windows.h>

#include "common.h"


class FileBase
{
public:
    virtual bool Open(char *path, char *mode) = 0;
    virtual void Close() = 0;
    virtual DWORD Read(void *buffer, DWORD size) = 0;
    virtual bool Write(void *buffer, DWORD size) = 0;
    virtual DWORD Tell() = 0;
    virtual DWORD GetSize() = 0;
    virtual bool Seek(LONG offset, DWORD origin) = 0;

    virtual ~FileBase()
    {
    }
};


class File : public FileBase
{
public:

    File();

    ~File();




    virtual bool Open(char *path, char *mode);

    virtual void Close();

    virtual DWORD Read(void *buffer, DWORD size);

    virtual bool Write(void *buffer, DWORD size);

    virtual DWORD Tell();

    virtual DWORD GetSize();

    virtual bool Seek(LONG offset, DWORD origin);


    virtual void *ReadAll(DWORD maxSize);



    DWORD ReadDword(void *dst)
    {
        return this->Read(dst, 4);
    }

    HANDLE handle;

    DWORD access;
};
ASSERT_SIZE(File, 0xc);



extern const DWORD g_SeekOrigins[3];


extern char *g_FileModes[3];



void MakeFullPath(char *dst, char *path);



struct DatArchiveEntry
{
    DatArchiveEntry()
    {
        this->name = NULL;
    }
    ~DatArchiveEntry()
    {
        if (this->name != NULL)
        {
            GlobalFree(this->name);
            this->name = NULL;
        }
    }


    char *name;



    DWORD offset;

    DWORD size;
    DWORD checksum;
};
ASSERT_SIZE(DatArchiveEntry, 0x10);



struct DatArchiveHeader
{
    int numEntries;
    DWORD tableOffset;
    DWORD decodedSize;
};
ASSERT_SIZE(DatArchiveHeader, 0xc);



struct DatArchiveCursor
{

    DWORD SkipDword()
    {
        this->cur += 4;
        return *(DWORD *)this->cur;
    }


    BYTE *SkipString()
    {
        this->cur += strlen((char *)this->cur) + 1;
        return this->cur;
    }

    BYTE *cur;
};



class DatArchive
{
public:

    DatArchive();

    ~DatArchive();



    bool Open(char *path);

    void Release();

    DWORD GetEntrySize(char *name);


    void *Decode(char *name, void *buffer);

    DatArchiveEntry *FindEntry(char *name);



    bool ReadTable(char *path);



    DatArchiveEntry *BuildEntryTable(BYTE *table, int count, DWORD tableOffset);

    char *DuplicateString(char *str);

    DatArchiveEntry *entries;
    int numEntries;

    char *path;
    File *file;
};
ASSERT_SIZE(DatArchive, 0x10);
extern DatArchive g_Archive;
