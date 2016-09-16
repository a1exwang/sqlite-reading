#include <stdio.h>
#include <stdlib.h>

#include "../build/sqlite3/sqlite3.h"

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
  char *zErrMsg;

  int rc = sqlite3_open("test_simple_api.sqlite3", &db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  rc = sqlite3_exec(db, "create table haha (id integer primary key, name string);", callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  rc = sqlite3_exec(db, "insert into haha (name) values ('alex');", callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
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
  rc = sqlite3_exec(db, "select * from haha;", callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  sqlite3_close(db);
  return 0;
}
