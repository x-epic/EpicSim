
include_directories(${PROJECT_SOURCE_DIR}/ivlpp)
include_directories(${PROJECT_SOURCE_DIR}/driver)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/ivl)

add_custom_command(
        OUTPUT lexor.c
        COMMAND ${FLEX} --outfile=lexor.c
            ${PROJECT_SOURCE_DIR}/ivlpp/lexor.lex
        COMMENT "Generating lexor.c"
    )

add_definitions(-DHAVE_CONFIG_H)

add_executable(epicsim-precompiler main.c lexor.c)

install(TARGETS epicsim-precompiler DESTINATION lib/epicsim)
