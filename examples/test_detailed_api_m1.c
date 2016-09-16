#include <stdio.h>
#include <stdlib.h>

#include <sqlite3ext.h>
#include <sqliteInt.h>
#include <vdbeInt.h>
#include "utils.h"

// create table haha (id integer primary key, name string);
// insert into haha (name) values ("alex");
// insert into haha (name) values ("betty");
// insert into haha (name) values ("ciara");
// select * from haha;

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
  //  rc = sqlite3_prepare_v2(db, "create table haha (id integer primary key, name string);", -1, &pStmt, NULL);

  Parse *pParse;
  sqlite3_stmt *pStmt;

  pParse = sqlite3StackAllocZero(db, sizeof(*pParse));
  pParse->pReprepare = 0 /* pReprepare */;
  pParse->db = db;
  pParse->nQueryLoop = 0;  /* Logarithmic, so 0 really means 1 */

  char *zErrMsg;
  char *zSql = "create table haha (id integer primary key, name string);";
  sqlite3RunParser(pParse, zSql, &zErrMsg);
  if (pParse->rc == SQLITE_DONE)
    pParse->rc = SQLITE_OK;

  Vdbe *pVdbe = pParse->pVdbe;
  sqlite3VdbeSetSql(pVdbe, zSql, (int)(pParse->zTail-zSql), /* saveSqlFlag */ 1);
  pStmt = (sqlite3_stmt*)pParse->pVdbe;

  while( pParse->pTriggerPrg ){
    TriggerPrg *pT = pParse->pTriggerPrg;
    pParse->pTriggerPrg = pT->pNext;
    sqlite3DbFree(db, pT);
  }
  sqlite3ParserReset(pParse);
  sqlite3StackFree(db, pParse);
  rc = sqlite3ApiExit(db, rc);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "create table error: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  // ----------------------------------- sqlite3_prepare_v2 END -----------------------
  // rc = sqlite3_step(pStmt);
  Vdbe *p = (Vdbe*)pStmt;  /* the prepared statement */
  sqlite3_reset((sqlite3_stmt*)p);

  db = p->db;
  if( p->pc<0 ){
    /* If there are no other statements currently running, then
    ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
    ** from interrupting a statement that has not yet started.
    */
    if( db->nVdbeActive==0 ){
      db->u1.isInterrupted = 0;
    }

    db->nVdbeActive++;
    if( p->readOnly==0 ) db->nVdbeWrite++;
    if( p->bIsReader ) db->nVdbeRead++;
    p->pc = 0;
  }
  db->nVdbeExec++;
  rc = sqlite3VdbeExec(p);
  db->nVdbeExec--;

  if( rc==SQLITE_DONE ){
    p->rc = SQLITE_OK;
  }

  db->errCode = rc;
  if (SQLITE_NOMEM == sqlite3ApiExit(p->db, p->rc)) {
    p->rc = SQLITE_NOMEM_BKPT;
  }

  rc = rc & db->errMask;
  if (rc == SQLITE_DONE) {
    printf("table created\n");
  }
  else {
    fprintf(stderr, "create table error: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }
  sqlite3_finalize(pStmt);
  // -------------------- sqlite3_step END -----------------------------------

  // insert
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
