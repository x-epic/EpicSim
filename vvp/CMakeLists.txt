include_directories(${PROJECT_SOURCE_DIR}/vvp)
include_directories(${PROJECT_SOURCE_DIR}/driver)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/ivl)
include_directories(${PROJECT_SOURCE_DIR}/libmisc)


add_custom_command(
    OUTPUT lexor.cc
    COMMAND ${FLEX} -s --outfile=lexor.cc ${PROJECT_SOURCE_DIR}/vvp/lexor.lex
    COMMENT "Generating lexor.cc"
)

add_custom_command(
    OUTPUT parse.cc
    OUTPUT parse.h
    COMMAND ${BISON} --verbose -t -d -o parse.cc ${PROJECT_SOURCE_DIR}/vvp/parse.y
    COMMAND mv -f parse.hh parse.h
    COMMENT "Generating parse.cc"
)

add_custom_command(
    OUTPUT tables.cc
	COMMAND gcc -o draw_tt ${PROJECT_SOURCE_DIR}/vvp/draw_tt.c
    COMMAND ./draw_tt > tables.cc; rm -f draw_tt
    COMMENT "Generating tables.cc"
)

file(GLOB tgt_vvp_src
    "*.c" "*.cc"
)

set(CMAKE_EXE_LINKER_FLAGS "-rdynamic")
add_executable(epicsim-vvp main.cc parse.cc parse_misc.cc lexor.cc arith.cc array_common.cc array.cc bufif.cc compile.cc
    concat.cc dff.cc class_type.cc enum_type.cc extend.cc file_line.cc latch.cc npmos.cc part.cc
    permaheap.cc reduce.cc resolv.cc sfunc.cc stop.cc substitute.cc
    symbols.cc ufunc.cc codes.cc vthread.cc schedule.cc statistics.cc tables.cc udp.cc
    vvp_island.cc vvp_net.cc vvp_net_sig.cc vvp_object.cc vvp_cobject.cc vvp_darray.cc event.cc 
    logic.cc delay.cc words.cc island_tran.cc vpi_modules.cc vpi_callback.cc vpi_cobject.cc vpi_const.cc vpi_darray.cc
    vpi_event.cc vpi_iter.cc vpi_mcd.cc vpi_priv.cc vpi_scope.cc vpi_real.cc vpi_signal.cc vpi_string.cc 
    vpi_tasks.cc vpi_time.cc vpi_vthr_vector.cc vpip_bin.cc vpip_hex.cc vpip_oct.cc
    vpip_to_dec.cc vpip_format.cc vvp_vpi.cc)
target_link_libraries(epicsim-vvp dl m z pthread)

add_library(epicsim_simu_core STATIC main.cc parse.cc parse_misc.cc lexor.cc arith.cc array_common.cc array.cc
    bufif.cc compile.cc concat.cc dff.cc class_type.cc enum_type.cc extend.cc file_line.cc latch.cc npmos.cc part.cc
    permaheap.cc reduce.cc resolv.cc sfunc.cc stop.cc substitute.cc symbols.cc ufunc.cc codes.cc
    vthread.cc schedule.cc statistics.cc tables.cc udp.cc vvp_island.cc vvp_net.cc vvp_net_sig.cc
    vvp_object.cc vvp_cobject.cc vvp_darray.cc event.cc logic.cc delay.cc
    words.cc island_tran.cc vpi_modules.cc vpi_callback.cc vpi_cobject.cc vpi_const.cc vpi_darray.cc
    vpi_event.cc vpi_iter.cc vpi_mcd.cc vpi_priv.cc vpi_scope.cc vpi_real.cc vpi_signal.cc
    vpi_string.cc vpi_tasks.cc vpi_time.cc vpi_vthr_vector.cc vpip_bin.cc vpip_hex.cc vpip_oct.cc
    vpip_to_dec.cc vpip_format.cc vvp_vpi.cc)


install(TARGETS epicsim-vvp DESTINATION bin)
install(TARGETS epicsim_simu_core DESTINATION lib/epicsim)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/vvp/ DESTINATION include/epicsim FILES_MATCHING PATTERN "*.h")
