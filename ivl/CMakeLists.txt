include_directories(${PROJECT_SOURCE_DIR}/ivl)
include_directories(${PROJECT_SOURCE_DIR}/)
include_directories(${PROJECT_SOURCE_DIR}/driver)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/libmisc)
include_directories(${PROJECT_SOURCE_DIR}/vvp)

add_custom_command(
        OUTPUT parse.cc
        OUTPUT parse.h
        COMMAND ${BISON} --verbose -t -p VL -d -o parse.cc ${PROJECT_SOURCE_DIR}/ivl/parse.y
        COMMAND mv -f parse.hh parse.h
        COMMENT "Generating parse.cc"
    )

add_custom_command(
        OUTPUT lexor.cc
        COMMAND ${FLEX} -s --outfile=lexor.cc ${PROJECT_SOURCE_DIR}/ivl/lexor.lex
        COMMENT "Generating lexor.cc"
    )

add_custom_command(
        OUTPUT lexor_keyword.cc
        COMMAND ${GPERF} -o -i 7 -C -k 1-4,6,9,$$ -H keyword_hash -N check_identifier
             -t ${PROJECT_SOURCE_DIR}/ivl/lexor_keyword.gperf > lexor_keyword.cc
        COMMENT "Generating lexor_keyword.cc"
    )

add_custom_command(
        OUTPUT syn-rules.cc
        COMMAND ${BISON} --verbose -t -p syn_ -o syn-rules.cc ${PROJECT_SOURCE_DIR}/ivl/syn-rules.y
        COMMENT "Generating syn-rules.cc"
    )

add_custom_command(
        OUTPUT version_tag.h
        COMMAND bash ${PROJECT_SOURCE_DIR}/scripts/CREATE_VERSION.sh
        COMMAND cp -f ${PROJECT_BINARY_DIR}/ivl/version_tag.h ${PROJECT_SOURCE_DIR}/ivl/version_tag.h
        COMMENT "Generating version-tag.h"
)

set(CMAKE_EXE_LINKER_FLAGS "-rdynamic")

add_definitions(-DHAVE_CONFIG_H)
add_executable(epicsim-compiler main.cc async.cc design_dump.cc discipline.cc dup_expr.cc elaborate.cc
        elab_expr.cc elaborate_analog.cc elab_lval.cc elab_net.cc
        elab_scope.cc elab_sig.cc elab_sig_analog.cc elab_type.cc
        emit.cc eval.cc eval_attrib.cc eval_tree.cc expr_synth.cc functor.cc lexor.cc 
        lexor_keyword.cc link_const.cc load_module.cc netlist.cc netmisc.cc nettypes.cc
        net_analog.cc net_assign.cc net_design.cc netclass.cc netdarray.cc
        netenum.cc netparray.cc netqueue.cc netscalar.cc netstruct.cc netvector.cc
        net_event.cc net_expr.cc net_func.cc net_func_eval.cc net_link.cc net_modulo.cc
        net_nex_input.cc net_nex_output.cc net_proc.cc net_scope.cc net_tran.cc
        net_udp.cc pad_to_width.cc parse.cc parse_misc.cc pform.cc pform_analog.cc
        pform_disciplines.cc pform_dump.cc pform_package.cc pform_pclass.cc
        pform_class_type.cc pform_string_type.cc pform_struct_type.cc pform_types.cc
        symbol_search.cc sync.cc sys_funcs.cc verinum.cc verireal.cc vpi_modules.cc target.cc
        Attrib.cc HName.cc Module.cc PClass.cc PDelays.cc PEvent.cc PExpr.cc PFunction.cc
        PGate.cc PGenerate.cc PModport.cc PNamedItem.cc PPackage.cc PScope.cc PSpec.cc
        PTask.cc PUdp.cc PWire.cc Statement.cc AStatement.cc ${PROJECT_SOURCE_DIR}/libmisc/LineInfo.cc
        ${PROJECT_SOURCE_DIR}/libmisc/StringHeap.cc cprop.cc exposenodes.cc nodangle.cc synth.cc 
        synth2.cc syn-rules.cc t-dll.cc t-dll-api.cc t-dll-expr.cc t-dll-proc.cc t-dll-analog.cc version_tag.h)


        
target_link_libraries(epicsim-compiler dl pthread)

install(TARGETS epicsim-compiler DESTINATION lib/epicsim)
install(FILES constants.vams DESTINATION lib/epicsim/include)
install(FILES disciplines.vams DESTINATION lib/epicsim/include)
install(FILES ${PROJECT_SOURCE_DIR}/ivl/ivl_target.h DESTINATION include/epicsim)
install(FILES ${PROJECT_SOURCE_DIR}/ivl/_pli_types.h DESTINATION include/epicsim)
install(FILES ${PROJECT_SOURCE_DIR}/ivl/sv_vpi_user.h DESTINATION include/epicsim)
install(FILES ${PROJECT_SOURCE_DIR}/ivl/vpi_user.h DESTINATION include/epicsim)
install(FILES ${PROJECT_SOURCE_DIR}/ivl/acc_user.h DESTINATION include/epicsim)
install(FILES ${PROJECT_SOURCE_DIR}/ivl/veriuser.h DESTINATION include/epicsim)
install(FILES ${PROJECT_SOURCE_DIR}/ivl/ivl_alloc.h DESTINATION include/epicsim)