add_custom_command(
        OUTPUT cflexor.c
        COMMAND ${FLEX} -s --outfile=cflexor.c
            ${CMAKE_SOURCE_DIR}/driver/cflexor.lex
        COMMENT "Generating cflexor.c"
    )

add_custom_command(
        OUTPUT cfparse.c
        COMMAND ${BISON} --verbose -t -p cf -d -o
            cfparse.c ${CMAKE_SOURCE_DIR}/driver/cfparse.y
        COMMENT "Generating cfparse.c"
    )

option(USE_VERIFIC_PARSER "Using Verific as frontend parser" OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fno-exceptions")

## include directories
set(INC_DIRS ${CMAKE_SOURCE_DIR}/driver ${CMAKE_SOURCE_DIR}/ivl)
if (USE_VERIFIC_PARSER)
    set(INC_DIRS ${INC_DIRS}
            ${CMAKE_SOURCE_DIR}/veri-parser/transform
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/commands
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/verilog
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/vhdl
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/hdl_file_sort
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/hier_tree
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/verilog_nl
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/database
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/util
            ${CMAKE_SOURCE_DIR}/veri-parser/parser/include/containers
            )
endif()
include_directories(${INC_DIRS})

## link libs
link_directories(
        ${CMAKE_SOURCE_DIR}/veri-parser/parser/lib
)

add_definitions(-DIVL_ROOT="/usr/local/lib/epicsim" -DIVL_SUFFIX="" -DIVL_INC="/usr/local/include" -DIVL_LIB="/usr/local/lib" 
    -DGALAXSIM_LIB_ROOT="${CMAKE_INSTALL_PREFIX}/lib/epicsim" -DDLLIB="-ldl")

add_executable(epicsim-driver main.c substit.c cflexor.c cfparse.c)
if (USE_VERIFIC_PARSER)
    set(LIBS veritransform)
    target_link_libraries(epicsim-driver ${LIBS})
endif()
target_link_libraries(epicsim-driver pthread)

install(TARGETS epicsim-driver DESTINATION bin)
install(PROGRAMS ${PROJECT_SOURCE_DIR}/bin/epicsim DESTINATION bin)
install(PROGRAMS ${PROJECT_SOURCE_DIR}/bin/epicsim-vpi DESTINATION bin)
