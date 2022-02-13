include (FindLua)
find_package(ZLIB REQUIRED )

include_directories(../include)
include_directories(../unity)

set(TEST_NAMES 
    #animation-test
    append-gff-test
    base-test
    #combat-test
    entity-list-test
    entity-test
    gff-test
    #load-test
    lua-test
    #new-char-test
    player-test
    region-manager-test
    region-test
    smoke-test
)

foreach(TEST_NAME ${TEST_NAMES})
    add_executable(${TEST_NAME}_app ${TEST_NAME}.c base.c)
    target_link_libraries(${TEST_NAME}_app unity soloscuro xmi2mid spng ${ZLIB_LIBRARIES} ${LUA_LIBRARIES})
    add_test(
        NAME              ${TEST_NAME} 
        COMMAND           ${TEST_NAME}_app
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../
        )
endforeach()
