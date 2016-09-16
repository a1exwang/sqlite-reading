cmake_minimum_required (VERSION 2.6)
project (sqlite-reading)

# These are SQLite3 build macros
add_definitions(-DSQLITE_CORE)
add_definitions(-DSQLITE_OS_UNIX=1)
add_definitions(-DSQLITE_PRIVATE=)

add_custom_target(sqlite3 
  COMMAND ${CMAKE_SOURCE_DIR}/buildsqlite3.sh
  )

# All sqlite source files are here
set(SQLITE_SOURCE_FILES
        # interface
        build/sqlite3/tsrc/main.c
        build/sqlite3/tsrc/legacy.c
        build/sqlite3/tsrc/vdbeapi.c
        build/sqlite3/tsrc/prepare.c

        # tokenizer & parser
        build/sqlite3/tsrc/tokenize.c
        build/sqlite3/tsrc/parse.c

        # codegen
        build/sqlite3/tsrc/walker.c
        build/sqlite3/tsrc/alter.c
        build/sqlite3/tsrc/attach.c
        build/sqlite3/tsrc/auth.c
        build/sqlite3/tsrc/build.c
        build/sqlite3/tsrc/delete.c
        build/sqlite3/tsrc/expr.c
        build/sqlite3/tsrc/insert.c
        build/sqlite3/tsrc/pragma.c
        build/sqlite3/tsrc/resolve.c
        build/sqlite3/tsrc/select.c
        build/sqlite3/tsrc/trigger.c
        build/sqlite3/tsrc/update.c
        build/sqlite3/tsrc/vacuum.c
        build/sqlite3/tsrc/where.c
        build/sqlite3/tsrc/wherecode.c
        build/sqlite3/tsrc/whereexpr.c

        # vdbe
        build/sqlite3/tsrc/vdbe.c
        build/sqlite3/tsrc/vdbeaux.c
        build/sqlite3/tsrc/func.c
        build/sqlite3/tsrc/date.c
        build/sqlite3/tsrc/vdbemem.c
        build/sqlite3/tsrc/rowset.c
        build/sqlite3/tsrc/opcodes.c
        build/sqlite3/tsrc/vdbetrace.c
        build/sqlite3/tsrc/vdbesort.c
        build/sqlite3/tsrc/vdbeblob.c

        # btree
        build/sqlite3/tsrc/btree.c
        build/sqlite3/tsrc/btmutex.c

        # pager
        build/sqlite3/tsrc/pager.c

        # os
        build/sqlite3/tsrc/os.c
        build/sqlite3/tsrc/os_unix.c
        build/sqlite3/tsrc/mutex.c
        build/sqlite3/tsrc/mutex_unix.c
        build/sqlite3/tsrc/mutex_noop.c
        build/sqlite3/tsrc/threads.c

        # utils
        build/sqlite3/tsrc/util.c
        build/sqlite3/tsrc/hash.c
        build/sqlite3/tsrc/printf.c
        build/sqlite3/tsrc/global.c
        build/sqlite3/tsrc/pcache.c
        build/sqlite3/tsrc/pcache1.c
        build/sqlite3/tsrc/malloc.c
        build/sqlite3/tsrc/mem1.c
        build/sqlite3/tsrc/fault.c
        build/sqlite3/tsrc/status.c
        build/sqlite3/tsrc/random.c
        build/sqlite3/tsrc/wal.c
        build/sqlite3/tsrc/bitvec.c
        build/sqlite3/tsrc/memjournal.c
        build/sqlite3/tsrc/backup.c
        build/sqlite3/tsrc/utf.c
        build/sqlite3/tsrc/callback.c
        build/sqlite3/tsrc/loadext.c
        build/sqlite3/tsrc/ctime.c
        build/sqlite3/tsrc/vtab.c
        build/sqlite3/tsrc/analyze.c
        build/sqlite3/tsrc/fkey.c
        build/sqlite3/tsrc/complete.c
        build/sqlite3/tsrc/table.c
        )
include_directories(build/sqlite3/tsrc)

add_library(sqlite3_obj OBJECT ${SQLITE_SOURCE_FILES})

# Test targets
add_executable(shell build/sqlite3/shell.c $<TARGET_OBJECTS:sqlite3_obj>)
target_link_libraries(shell dl pthread)
add_dependencies(shell sqlite3 sqlite3_obj)

add_executable(test_os examples/test_os.c $<TARGET_OBJECTS:sqlite3_obj>)
target_link_libraries(test_os dl pthread)
add_dependencies(test_os sqlite3 sqlite3_obj)

add_executable(test_pager examples/test_pager.c $<TARGET_OBJECTS:sqlite3_obj>)
target_link_libraries(test_pager dl pthread)
add_dependencies(test_pager sqlite3 sqlite3_obj)

add_executable(test_btree examples/test_btree.c $<TARGET_OBJECTS:sqlite3_obj>)
target_link_libraries(test_btree dl pthread)
add_dependencies(test_btree sqlite3 sqlite3_obj)

add_executable(test_simple_api examples/test_simple_api.c $<TARGET_OBJECTS:sqlite3_obj>)
target_link_libraries(test_simple_api dl pthread)
add_dependencies(test_simple_api sqlite3 sqlite3_obj)

add_executable(test_detailed_api examples/test_detailed_api.c $<TARGET_OBJECTS:sqlite3_obj>)
target_link_libraries(test_detailed_api dl pthread)
add_dependencies(test_detailed_api sqlite3 sqlite3_obj)

add_executable(test_detailed_api_m1 examples/test_detailed_api_m1.c $<TARGET_OBJECTS:sqlite3_obj>)
target_link_libraries(test_detailed_api_m1 dl pthread)
add_dependencies(test_detailed_api_m1 sqlite3 sqlite3_obj)

add_executable(test_detailed_api_m2 examples/test_detailed_api_m2.c $<TARGET_OBJECTS:sqlite3_obj>)
target_link_libraries(test_detailed_api_m2 dl pthread)
add_dependencies(test_detailed_api_m2 sqlite3 sqlite3_obj)
