#include <stdio.h>
#include <stdlib.h>

#include "sqlite3ext.h"
#include "sqliteInt.h"

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

int main(int argc, char **argv) {
  sqlite3 *db;

  int rc = sqlite3_open("test_simple_api.sqlite3", &db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  // create table
  sqlite3_stmt *pStmt;
  rc = sqlite3_prepare_v2(db, "create table haha (id integer primary key, name string);", -1, &pStmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "create table error: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  rc = sqlite3_step(pStmt);
  if (rc == SQLITE_DONE) {
    printf("table created\n");
  }
  else {
    fprintf(stderr, "create table error: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }
  sqlite3_finalize(pStmt);

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
