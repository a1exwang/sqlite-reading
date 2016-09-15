cmake_minimum_required (VERSION 2.6)
project (sqlite-reading)

# These are SQLite3 build macros
add_definitions(-DSQLITE_CORE)
add_definitions(-DSQLITE_OS_UNIX=1)
add_definitions(-DSQLITE_PRIVATE=)

add_custom_target(sqlite3 
  COMMAND ${CMAKE_SOURCE_DIR}/buildsqlite3.sh
  )

# Test targets
add_executable(test_os examples/test_os.c build/sqlite3/sqlite3.c)
target_link_libraries(test_os dl pthread)
add_dependencies(test_os sqlite3)

add_executable(test_pager examples/test_pager.c build/sqlite3/sqlite3.c)
target_link_libraries(test_pager dl pthread)
add_dependencies(test_pager sqlite3)

add_executable(test_btree examples/test_btree.c build/sqlite3/sqlite3.c)
target_link_libraries(test_btree dl pthread)
add_dependencies(test_btree sqlite3)
