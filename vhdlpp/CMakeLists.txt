include_directories(${PROJECT_SOURCE_DIR}/vhdlpp)
include_directories(${PROJECT_SOURCE_DIR}/driver)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/ivl)
include_directories(${PROJECT_SOURCE_DIR}/libmisc)
include_directories(${PROJECT_BINARY_DIR}/vhdlpp)

add_custom_command(
        OUTPUT lexor.cc
        COMMAND ${FLEX} -s --outfile=lexor.cc
            ${PROJECT_SOURCE_DIR}/vhdlpp/lexor.lex
        COMMENT "Generating lexor.cc"
    )

add_custom_command(
        OUTPUT lexor_keyword.cc
        COMMAND ${GPERF} -o -i 7 -C -k 1-4,6,9,$$ -H keyword_hash -N check_identifier
             -t ${PROJECT_SOURCE_DIR}/vhdlpp/lexor_keyword.gperf > lexor_keyword.cc
        COMMENT "Generating lexor_keyword.cc"
    )

add_custom_command(
        OUTPUT parse.cc
        OUTPUT parse.h
        COMMAND ${BISON} --verbose -t -d -o parse.cc ${PROJECT_SOURCE_DIR}/vhdlpp/parse.y
        COMMAND mv -f parse.hh parse.h
        COMMENT "Generating parse.cc"
    )

file(GLOB tgt_vhdlpp_src
    "*.c" "*.cc"
)

add_executable(vhdlpp parse.cc main.cc architec.cc compiler.cc entity.cc std_funcs.cc std_types.cc
expression.cc package.cc scope.cc sequential.cc subprogram.cc vsignal.cc vtype.cc
vtype_match.cc architec_elaborate.cc entity_elaborate.cc expression_elaborate.cc
expression_evaluate.cc sequential_elaborate.cc vtype_elaborate.cc entity_stream.cc
expression_stream.cc vtype_stream.cc lexor.cc lexor_keyword.cc 
parse_misc.cc library.cc vhdlreal.cc vhdlint.cc architec_emit.cc entity_emit.cc
expression_emit.cc package_emit.cc sequential_emit.cc subprogram_emit.cc vtype_emit.cc
debug.cc architec_debug.cc expression_debug.cc sequential_debug.cc
${PROJECT_SOURCE_DIR}/libmisc/StringHeap.cc ${PROJECT_SOURCE_DIR}/libmisc/LineInfo.cc)

target_link_libraries(vhdlpp m bz2 z pthread termcap)

install(TARGETS vhdlpp DESTINATION lib/epicsim)

