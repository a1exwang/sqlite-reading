#pragma once

void errorReport(char *msg, int status) {
  if (status != SQLITE_OK) {
    fprintf(stderr, "%s: failed. status = %d", msg, status);
    exit(1);
  }
}
