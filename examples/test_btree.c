#include "../build/sqlite3/tsrc/sqlite3.h"
#include "../build/sqlite3/tsrc/sqliteInt.h"
#include "utils.h"

#include <stdio.h>

int fnCompare(void* unused, int aSize, const void *a, int bSize, const void *b) {
  printf("fnComparee called, aSize = %d, bSize = %d", aSize, bSize);
  return *((int*)a) - *((int*)b);
}
CollSeq collSeq;
u8 sortOrder[1];

int main() {
  sqlite3_vfs *pVfs;
  sqlite3 *pConn;
  Btree *pBtree;
  int outFlags, status;
  i64 fileSize, sectorSize;
  int pageNo;

  sqlite3OsInit();
  pVfs = sqlite3_vfs_find(0);

  sqlite3_open("test_btree.sqlite3", &pConn);

  status = sqlite3BtreeOpen(pVfs, "test_btree.sqlite3", pConn, &pBtree, 0, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
  errorReport("sqlite3BtreeOpen", status);

  status = sqlite3BtreeBeginTrans(pBtree, 1);
  errorReport("sqlite3BtreeBeginTrans", status);

  // CreateTable
  status = sqlite3BtreeCreateTable(pBtree, &pageNo, BTREE_INTKEY);
  errorReport("sqlite3BtreeCreateTable", status);
  printf("sqlite3BtreeCreateTable: Btree page number=%d\n", pageNo);

  // OpenWrite
  int cursorSize = sqlite3BtreeCursorSize();
  BtCursor *pCur = malloc((size_t) cursorSize);

  collSeq.enc = SQLITE_UTF8;
  collSeq.zName = "collSeqName";
  collSeq.pUser = 0;
  collSeq.xCmp = fnCompare;
  collSeq.xDel = 0;

  KeyInfo info;
  info.nRef = 1;
  info.enc = SQLITE_UTF8;
  info.nField = 1;
  info.nXField = 1;
  info.db = pConn;
  info.aSortOrder = sortOrder;
  info.aColl[0] = &collSeq;
  status = sqlite3BtreeCursor(pBtree, pageNo, 1, &info, pCur);
  errorReport("sqlite3BtreeCursor", status);

  printf("sqlite3BtreeCursorSize: %d\n", cursorSize);


  // NewRowid
  int res;
  sqlite3_int64 v;
  status = sqlite3BtreeLast(pCur, &res);
  errorReport("sqlite3BtreeLast", status);

  if (res) {
    v = 1;   /* IMP: R-61914-48074 */
  }
  else {
    v = sqlite3BtreeIntegerKey(pCur) + 1;
  }

  // Insert
  sqlite3_int64 id = v;

  BtreePayload payload;
  payload.pKey = 0;
  payload.nKey = id;
  payload.pData = 0;
  payload.nData = 0;
  payload.nZero = 0;
  status = sqlite3BtreeInsert(pCur, &payload, 1, 0);
  errorReport("sqliteBtreeInsert", status);


  // Close
  status = sqlite3BtreeCommit(pBtree);
  errorReport("sqlite3BtreeCommit", status);

  status = sqlite3BtreeClose(pBtree);
  errorReport("sqlite3BtreeClose", status);
  return 0;
}
