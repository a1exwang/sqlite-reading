#include <stdio.h>
#include <stdlib.h>

#include <sqlite3ext.h>
#include <sqliteInt.h>
#include <btreeInt.h>
#include <vdbeInt.h>
#include "utils.h"

// create table haha (id integer primary key, name string);
// insert into haha (name) values ("alex");
// insert into haha (name) values ("betty");
// insert into haha (name) values ("ciara");
// select * from haha;

int fnCompare(void* unused, int aSize, const void *a, int bSize, const void *b) {
  printf("fnComparee called, aSize = %d, bSize = %d", aSize, bSize);
  return *((int*)a) - *((int*)b);
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  for (int i = 0; i < argc; ++i) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
static const int aHardLimit[] = {
        SQLITE_MAX_LENGTH,
        SQLITE_MAX_SQL_LENGTH,
        SQLITE_MAX_COLUMN,
        SQLITE_MAX_EXPR_DEPTH,
        SQLITE_MAX_COMPOUND_SELECT,
        SQLITE_MAX_VDBE_OP,
        SQLITE_MAX_FUNCTION_ARG,
        SQLITE_MAX_ATTACHED,
        SQLITE_MAX_LIKE_PATTERN_LENGTH,
        SQLITE_MAX_VARIABLE_NUMBER,      /* IMP: R-38091-32352 */
        SQLITE_MAX_TRIGGER_DEPTH,
        SQLITE_MAX_WORKER_THREADS,
};

int main(int argc, char **argv) {
  sqlite3 *db;
  char zFilename[256];
  char rm_command[512];
  sprintf(zFilename, "%s.sqlite", __FILE__);
  int rc;

//  sprintf(rm_command, "rm -f \"%s\"", zFilename);
//  system(rm_command);

  // sqlite3_open(filename, &db);
  rc = sqlite3_initialize();
  errorReport("sqlite3_initialize", rc);

  unsigned flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
  flags &= (unsigned) ~( SQLITE_OPEN_DELETEONCLOSE |
                         SQLITE_OPEN_EXCLUSIVE |
                         SQLITE_OPEN_MAIN_DB |
                         SQLITE_OPEN_TEMP_DB |
                         SQLITE_OPEN_TRANSIENT_DB |
                         SQLITE_OPEN_MAIN_JOURNAL |
                         SQLITE_OPEN_TEMP_JOURNAL |
                         SQLITE_OPEN_SUBJOURNAL |
                         SQLITE_OPEN_MASTER_JOURNAL |
                         SQLITE_OPEN_NOMUTEX |
                         SQLITE_OPEN_FULLMUTEX |
                         SQLITE_OPEN_WAL
  );

  db = sqlite3MallocZero(sizeof(sqlite3));
  db->errMask = 0xff;
  db->nDb = 2;
  db->magic = SQLITE_MAGIC_BUSY;
  db->aDb = db->aDbStatic;

  memcpy(db->aLimit, aHardLimit, sizeof(db->aLimit));
  db->aLimit[SQLITE_LIMIT_WORKER_THREADS] = SQLITE_DEFAULT_WORKER_THREADS;
  db->autoCommit = 1;
  db->nextAutovac = -1;
  db->szMmap = sqlite3GlobalConfig.szMmap;
  db->nextPagesize = 0;
  db->nMaxSorterMmap = 0x7FFFFFFF;
  db->flags |= SQLITE_ShortColNames | SQLITE_EnableTrigger | SQLITE_CacheSpill;
  db->openFlags = flags;
  sqlite3HashInit(&db->aCollSeq);
  sqlite3HashInit(&db->aModule);

  char *zOpen = 0;
  rc = sqlite3ParseUri(/* zVfs */0, zFilename, &flags, &db->pVfs, &zOpen, /* zErrMsg */0);
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);

  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  if( !db->mallocFailed ) ENC(db) = SCHEMA_ENC(db);
  sqlite3BtreeLeave(db->aDb[0].pBt);
  db->aDb[1].pSchema = sqlite3SchemaGet(db, 0);

  /* The default safety_level for the main database is FULL; for the temp
  ** database it is OFF. This matches the pager layer defaults.
  */
  db->aDb[0].zName = "main";
  db->aDb[0].safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  db->aDb[1].zName = "temp";
  db->aDb[1].safety_level = PAGER_SYNCHRONOUS_OFF;
  db->magic = SQLITE_MAGIC_OPEN;

  /* Register all built-in functions, but do not attempt to read the
  ** database schema yet. This is delayed until the first time the database
  ** is accessed.
  */
  sqlite3Error(db, SQLITE_OK);
  sqlite3RegisterPerConnectionBuiltinFunctions(db);

  // ------------------------------- sqlite3_open END ----------------------------------
  // Manually Manipulate Btree

  // Transaction
  sqlite3BtreeBeginTrans(db->aDb[0].pBt, 1);
  errorReport("sqlite3BtreeBeginTrans", rc);

  // LockTable
  rc = sqlite3BtreeLockTable(db->aDb[0].pBt, 1, 1);
  errorReport("sqlite3BtreeLockTable", rc);

  // CreateTable
  int pgNo;
  rc = sqlite3BtreeCreateTable(db->aDb[0].pBt, &pgNo, 0);
  errorReport("sqlite3BtreeCreateTable", rc);

  // OpenWrite 0 1 0 5 *
  int p1 = 0; /* P1 */
  int pageNo = 1;  /* P2 */
  int iDb = 0; /* P3 */
  int nField = 5; /* P4 */

  int cursorSize = sqlite3BtreeCursorSize();
  BtCursor *pCur = malloc((size_t) cursorSize);

  rc = sqlite3BtreeCursor(db->aDb[iDb].pBt, pageNo, 1 /* is Writable ? */, 0, pCur);
  errorReport("sqlite3BtreeCursor", rc);
  printf("sqlite3BtreeCursorSize: %d\n", cursorSize);


  // v = NewRowid
  int res;
  sqlite3_int64 newRowid;
  rc = sqlite3BtreeLast(pCur, &res);
  errorReport("sqlite3BtreeLast", rc);
  if (res) {
    newRowid = 1;   /* IMP: R-61914-48074 */
  }else{
    newRowid = sqlite3BtreeIntegerKey(pCur);
    newRowid++;
  }

//  // Blob
//  // Insert
  int p5 = 0x08;
  BtreePayload x;
//  x.nKey = newRowid;
//  x.pData = "\x06\x00\x00\x00\x00\x00";
//  x.nData = 6;
//  x.nZero = 0;
//  x.pKey = 0;
  int seekResult = 0;
//  rc = sqlite3BtreeInsert(pCur, &x, (p5 & OPFLAG_APPEND) != 0, seekResult);
//  errorReport("sqlite3BtreeInsert", rc);
//
//  // Close
//  rc = sqlite3BtreeCloseCursor(pCur);
//  errorReport("sqlite4BtreeCloseCursor", rc);
//  free(pCur);
//
//  // OpenWrite 1 1 0 5 *
//  p1 = 1; /* P1 */
//  pageNo = 1;  /* P2 */
//  iDb = 0; /* P3 */
//  nField = 5; /* P4 */
//
//  cursorSize = sqlite3BtreeCursorSize();
//  pCur = malloc((size_t) cursorSize);
//  rc = sqlite3BtreeCursor(db->aDb[iDb].pBt, pageNo, 1 /* is Writable ? */, 0, pCur);
//  errorReport("sqlite3BtreeCursor 2", rc);

  // MakeRecord & Insert
  p5 = 0x08;

  char *tmp = "\x06\x17\x15\x15\x01\x7b" // id, name, tbl_name, rootpage, sql
    "table" "hehe" "hehe" "\x00" "CREATE TABLE hehe (id INTEGER PRIMARY KEY, name STRING)";
  int len = 75;

  char *pBuf = malloc(len);
  memcpy(pBuf, tmp, len);
  pBuf[0x13] = 2;

  x.nKey = newRowid;
  x.pData = pBuf;
  x.nData = len;
  x.nZero = 0;
  x.pKey = 0;
  rc = sqlite3BtreeInsert(pCur, &x, (p5 & OPFLAG_APPEND) != 0, seekResult);
  errorReport("sqlite3BtreeInsert", rc);

  // Close
  rc = sqlite3BtreeCloseCursor(pCur);
  errorReport("sqlite3BtreeCloseCursor", rc);
  // free(pCur);
  rc = sqlite3BtreeCommit(db->aDb[iDb].pBt);
  errorReport("sqlite3BtreeCommit", rc);

  rc = sqlite3BtreeClose(db->aDb[iDb].pBt);
  errorReport("sqlite3BtreeClose", rc);

//  sqlite3_close(db);
  exit(0);

  // insert
  sqlite3_stmt *pStmt;
  rc = sqlite3_prepare_v2(db, "insert into haha (name) values ('alex');", -1, &pStmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "insert error: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  rc = sqlite3_step(pStmt);
  if (rc == SQLITE_DONE) {
    printf("row inserted\n");
  }
  else {
    fprintf(stderr, "insert error: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }
  sqlite3_finalize(pStmt);

  // insert into
  char *zErrMsg;
  rc = sqlite3_exec(db, "insert into haha (name) values ('betty');", callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  rc = sqlite3_exec(db, "insert into haha (name) values ('ciara');", callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  // select
  rc = sqlite3_prepare_v2(db, "select * from haha;", -1, &pStmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "insert error: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  while (1) {
    rc = sqlite3_step(pStmt);
    if (rc == SQLITE_DONE) {
      printf("select done\n");
      break;
    }
    else if (rc == SQLITE_ROW) {
      sqlite3_int64 id = sqlite3_column_int64(pStmt, 0);
      const unsigned char *pName = sqlite3_column_text(pStmt, 1);
      printf("row: %lld, %s\n", id, pName);
    }
    else {
      fprintf(stderr, "select error: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);
    }
  }
  sqlite3_finalize(pStmt);

  sqlite3_close(db);
  return 0;
}
