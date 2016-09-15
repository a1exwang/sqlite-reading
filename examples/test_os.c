#include "../build/sqlite3/tsrc/sqlite3.h"
#include "../build/sqlite3/tsrc/sqliteInt.h"

#include <stdio.h>

/**
 *
 * void sqlite3OsClose(sqlite3_file*);
 *  int sqlite3OsRead(sqlite3_file*, void*, int amt, i64 offset);
 *  int sqlite3OsWrite(sqlite3_file*, const void*, int amt, i64 offset);
    int sqlite3OsTruncate(sqlite3_file*, i64 size);
 *  int sqlite3OsSync(sqlite3_file*, int);
 *  int sqlite3OsFileSize(sqlite3_file*, i64 *pSize);
    int sqlite3OsLock(sqlite3_file*, int);
    int sqlite3OsUnlock(sqlite3_file*, int);
    int sqlite3OsCheckReservedLock(sqlite3_file *id, int *pResOut);
    int sqlite3OsFileControl(sqlite3_file*,int,void*);
    void sqlite3OsFileControlHint(sqlite3_file*,int,void*);
    #define SQLITE_FCNTL_DB_UNCHANGED 0xca093fa0
 *  int sqlite3OsSectorSize(sqlite3_file *id);
    int sqlite3OsDeviceCharacteristics(sqlite3_file *id);
    int sqlite3OsShmMap(sqlite3_file *,int,int,int,void volatile **);
    int sqlite3OsShmLock(sqlite3_file *id, int, int, int);
    void sqlite3OsShmBarrier(sqlite3_file *id);
    int sqlite3OsShmUnmap(sqlite3_file *id, int);
    int sqlite3OsFetch(sqlite3_file *id, i64, int, void **);
    int sqlite3OsUnfetch(sqlite3_file *, i64, void *);


    int sqlite3OsOpen(sqlite3_vfs *, const char *, sqlite3_file*, int, int *);
    int sqlite3OsDelete(sqlite3_vfs *, const char *, int);
    int sqlite3OsAccess(sqlite3_vfs *, const char *, int, int *pResOut);
    int sqlite3OsFullPathname(sqlite3_vfs *, const char *, int, char *);
    void *sqlite3OsDlOpen(sqlite3_vfs *, const char *);
    void sqlite3OsDlError(sqlite3_vfs *, int, char *);
    void (*sqlite3OsDlSym(sqlite3_vfs *, void *, const char *))(void);
    void sqlite3OsDlClose(sqlite3_vfs *, void *);
    int sqlite3OsRandomness(sqlite3_vfs *, int, char *);
    int sqlite3OsSleep(sqlite3_vfs *, int);
    int sqlite3OsGetLastError(sqlite3_vfs*);
    int sqlite3OsCurrentTimeInt64(sqlite3_vfs *, sqlite3_int64*);
 */

int main() {
  sqlite3_file *pFile;
  sqlite3_vfs *pVfs;
  int outFlags, status;
  i64 fileSize, sectorSize;

  sqlite3OsInit();
  pVfs = sqlite3_vfs_find(0);
  status = sqlite3OsOpenMalloc(pVfs, "test_os.sqlite3", &pFile, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE , &outFlags);
  if (status != SQLITE_OK) {
    fprintf(stderr, "sqlite3OsOpenMalloc: failed. status = %d", status);
  }

  size_t sizeBuffer = 0x40;
  char *buffer = malloc(sizeBuffer + 1);
  memset(buffer, 0x61, sizeBuffer);

  sectorSize = sqlite3OsSectorSize(pFile);
  printf("sqlite3OsSectorSize: SectorSize = %lld\n", sectorSize);

  status = sqlite3OsWrite(pFile, buffer, (int)sizeBuffer, 0);
  if (status != SQLITE_OK) {
    fprintf(stderr, "sqlite3OsWrite: failed. status = %d", status);
  }

//  status = sqlite3OsSync(pFile, SQLITE_SYNC_NORMAL);
//  if (status != SQLITE_OK) {
//    fprintf(stderr, "sqlite3OsSync: failed. status = %d", status);
//  }

  status = sqlite3OsFileSize(pFile, &fileSize);
  if (status != SQLITE_OK) {
    fprintf(stderr, "sqlite3OsFileSize: failed. status = %d\n", status);
  }
  printf("sqlite3OsFileSize: filesize = %lld\n", fileSize);


  memset(buffer, 0, sizeBuffer + 1);
  status = sqlite3OsRead(pFile, buffer, (int)sizeBuffer, 0);
  if (status != SQLITE_OK) {
    fprintf(stderr, "sqlite3OsRead: failed. status = %d", status);
  }
  printf("sqlite3OsRead: %s", buffer);

  sqlite3OsCloseFree(pFile);
  return 0;
}
