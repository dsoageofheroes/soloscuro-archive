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
    #new-char-test
    player-test
    region-manager-test
    region-test
    smoke-test
    lua-test
    lua-combat-01
    lua-combat-02
    lua-combat-03
    lua-combat-04
    lua-combat-05
    lua-combat-06
    lua-combat-07
    lua-combat-08
    lua-combat-09
    lua-combat-10
    lua-combat-11
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
