set(GENERATOR_SRCS
    extract.c
)

add_executable(extractor ${GENERATOR_SRCS})

add_custom_command(
    TARGET extractor
    POST_BUILD
    #COMMAND echo ${CMAKE_CURRENT_BINARY_DIR} 
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/extractor ../../src/powers/wizard/ ../../src/powers/priest/ ../../src/powers/psionic/ ../../src/powers/innate/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
