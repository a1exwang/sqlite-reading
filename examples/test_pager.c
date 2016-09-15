#include "../build/sqlite3/tsrc/sqlite3.h"
#include "../build/sqlite3/tsrc/sqliteInt.h"

#include <stdio.h>
#include "utils.h"


void reiniter(DbPage *page) {

}

int main() {
  sqlite3_file *pFile;
  sqlite3_vfs *pVfs;
  Pager *pPager;
  DbPage *pPage;
  int status;
  u32 pageSize = 1024;
  void *pageData;

  sqlite3OsInit();
  pVfs = sqlite3_vfs_find(0);

  status = sqlite3PagerOpen(pVfs, &pPager, "test_pager.sqlite3", 0, 0, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, reiniter);
  errorReport("sqlite3PagerOpen", status);

  sqlite3PagerSetPagesize(pPager, &pageSize, 0);
  errorReport("sqlite3PagerSetPagesize", status);

  status = sqlite3PagerSharedLock(pPager);
  errorReport("sqlite3PagerSharedLock", status);
  status = sqlite3PagerBegin(pPager, 1, 0);
  errorReport("sqlite3PagerBegin", status);

  status = sqlite3PagerGet(pPager, 3, &pPage, 0);
  errorReport("sqlite3PagerGet", status);

  status = sqlite3PagerWrite(pPage);
  errorReport("sqlite3PagerWrite", status);
  pageData = sqlite3PagerGetData(pPage);
  if (pageData != NULL) {
    memset(pageData, 'Y', 30);
  }
  status = sqlite3PagerCommitPhaseOne(pPager, NULL, 0);
  errorReport("sqlite3PagerCommitPhaseOne", status);


  status = sqlite3PagerFlush(pPager);
  errorReport("sqlite3PagerFlush", status);

  sqlite3PagerUnref(pPage);

  status = sqlite3PagerClose(pPager);
  errorReport("sqlite3PagerClose", status);

  status = sqlite3PagerOpen(pVfs, &pPager, "test_pager.sqlite3", 0, 0, SQLITE_OPEN_READONLY, NULL);
  errorReport("sqlite3PagerOpen", status);

  status = sqlite3PagerSharedLock(pPager);
  errorReport("sqlite3PagerSharedLock", status);

  status = sqlite3PagerGet(pPager, 3, &pPage, 0);
  errorReport("sqlite3PagerGet", status);
  pageData = sqlite3PagerGetData(pPage);
  if (pageData != NULL) {
    printf("sqlite3PagerGetData: %s", (char*) pageData);
  }
  sqlite3PagerUnref(pPage);

  status = sqlite3PagerClose(pPager);
  errorReport("sqlite3PagerClose", status);

  return 0;
}
