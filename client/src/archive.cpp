

#include "archive.h"
#include "lzss.h"
#include "unsorted.h"

DatArchive::DatArchive()
{
    this->entries = NULL;
    this->numEntries = 0;
    this->path = NULL;
    this->file = NULL;
}

DatArchiveEntry *DatArchive::FindEntry(char *name)
{
    DatArchiveEntry *cur;
    int left;

    if (this->entries == NULL)
    {
        return NULL;
    }
    for (cur = this->entries, left = this->numEntries; left > 0; left--, cur++)
    {
        if (_stricmp(name, cur->name) == 0)
        {
            return cur;
        }
    }
    return NULL;
}

char *DatArchive::DuplicateString(char *str)
{
    char *copy;

    copy = (char *)GlobalAlloc(GMEM_FIXED, strlen(str) + 1);
    if (copy != NULL)
    {
        strcpy(copy, str);
    }
    return copy;
}

void *DatArchive::Decode(char *name, void *buffer)
{
    DatArchiveEntry *found;
    DWORD decodedSize;
    void *decoded;
    HGLOBAL packed;
    DWORD packedSize;

    packed = NULL;
    if (this->file == NULL)
    {
        return NULL;
    }
    found = this->FindEntry(name);
    if (found == NULL)
    {
        goto fail;
    }
    if (!this->file->Open(this->path, g_FileModes[0]))
    {
        goto fail;
    }
    packedSize = found[1].offset - found->offset;
    decodedSize = found->size;
    packed = GlobalAlloc(GMEM_FIXED, packedSize);
    if (packed == NULL)
    {
        goto fail;
    }
    if (!this->file->Seek(found->offset, g_SeekOrigins[0]))
    {
        goto fail;
    }
    if (this->file->Read(packed, packedSize) == 0)
    {
        goto fail;
    }
    decoded = LzssDecompress((BYTE *)packed, packedSize, (BYTE *)buffer, decodedSize);
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    return decoded;

fail:
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    return NULL;
}

DWORD DatArchive::GetEntrySize(char *name)
{
    DatArchiveEntry *entry;

    entry = this->FindEntry(name);
    if (entry != NULL)
    {
        return entry->size;
    }
    return 0;
}

DatArchiveEntry *DatArchive::BuildEntryTable(BYTE *table, int count, DWORD tableOffset)
{
    DatArchiveEntry *entries;
    int i;
    DatArchiveCursor p;

    entries = NULL;
    entries = new DatArchiveEntry[count + 1];
    if (entries == NULL)
    {
        goto fail;
    }
    p.cur = table;
    for (i = 0; i < count; i++)
    {
        entries[i].name = this->DuplicateString((char *)p.cur);
        p.SkipString();
        entries[i].offset = *(DWORD *)p.cur;
        p.SkipDword();
        entries[i].size = *(DWORD *)p.cur;
        p.SkipDword();
        entries[i].checksum = *(DWORD *)p.cur;
        p.SkipDword();
    }
    entries[count].offset = tableOffset;
    entries[count].size = 0;
    return entries;

fail:
    if (entries != NULL)
    {
        delete[] entries;
        entries = NULL;
    }
    return NULL;
}

void DatArchive::Release()
{
    if (this->path != NULL)
    {
        GlobalFree(this->path);
        this->path = NULL;
    }
    if (this->entries != NULL)
    {
        delete[] this->entries;
        this->entries = NULL;
    }
    if (this->file != NULL)
    {
        delete this->file;
        this->file = NULL;
    }
    this->numEntries = 0;
}

bool DatArchive::ReadTable(char *path)
{
    BYTE *unpacked;
    DWORD decodedSize;
    DWORD magic;
    DWORD size;
    DWORD tableOffset;
    DatArchiveHeader archiveHeader;
    HGLOBAL packed;
    DatArchiveHeader *decryptedHeader;
    BYTE *decryptedTable;

    packed = NULL;
    unpacked = NULL;
    if (this->file == NULL)
    {
        return false;
    }
    if (!this->file->Open(path, g_FileModes[0]))
    {
        goto fail;
    }
    if (!this->file->ReadDword(&magic))
    {
        goto fail;
    }
    if (magic != 0x5a474250)
    {
        goto fail;
    }
    if (!this->file->Read(&archiveHeader, sizeof(archiveHeader)))
    {
        goto fail;
    }
    decryptedHeader = (DatArchiveHeader *)DecryptData((BYTE *)&archiveHeader, sizeof(archiveHeader), 0x1b, 0x37,
                                                      sizeof(archiveHeader), 0x400);
    archiveHeader = *decryptedHeader;
    g_MemoryTracker.Free(decryptedHeader);
    this->numEntries = archiveHeader.numEntries - 123456;
    tableOffset = archiveHeader.tableOffset - 345678;
    decodedSize = archiveHeader.decodedSize - 567891;
    if (this->numEntries <= 0)
    {
        goto fail;
    }
    size = this->file->GetSize();
    if (tableOffset >= size)
    {
        goto fail;
    }
    size -= tableOffset;
    this->file->Seek(tableOffset, g_SeekOrigins[0]);
    packed = GlobalAlloc(GMEM_FIXED, size);
    if (packed == NULL)
    {
        goto fail;
    }
    if (this->file->Read(packed, size) == 0)
    {
        goto fail;
    }
    decryptedTable = DecryptData((BYTE *)packed, size, 0x3e, 0x9b, 0x80, 0x400);
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }

    packed = decryptedTable;
    unpacked = (BYTE *)LzssDecompress((BYTE *)packed, size, NULL, decodedSize);
    if (unpacked == NULL)
    {
        goto fail;
    }
    this->entries = this->BuildEntryTable(unpacked, this->numEntries, tableOffset);
    if (this->entries == NULL)
    {
        goto fail;
    }
    g_MemoryTracker.Free(packed);
    if (unpacked != NULL)
    {
        GlobalFree(unpacked);
        unpacked = NULL;
    }
    return true;

fail:
    g_MemoryTracker.Free(packed);
    if (unpacked != NULL)
    {
        GlobalFree(unpacked);
        unpacked = NULL;
    }
    if (this->file != NULL)
    {
        delete this->file;
        this->file = NULL;
    }
    DebugPrint("ファイル %s のオープン中にエラーが発生しました\r\n", path);
    while (0)
    {
    }
    return false;
}

DatArchive::~DatArchive()
{
    this->Release();
}

bool DatArchive::Open(char *path)
{
    this->Release();
    this->file = new File();
    if (this->file == NULL)
    {
        return false;
    }
    if (this->ReadTable(path))
    {
        this->path = this->DuplicateString(path);
        if (this->path != NULL)
        {
            return true;
        }
    }
    this->Release();
    return false;
}
