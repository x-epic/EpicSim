include_directories(${PROJECT_SOURCE_DIR}/driver)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/ivl)
include_directories(${PROJECT_SOURCE_DIR}/libmisc)
include_directories(${PROJECT_SOURCE_DIR}/vpi)

add_custom_command(
    OUTPUT sys_readmem_lex.c
    COMMAND ${FLEX} --outfile=sys_readmem_lex.c ${PROJECT_SOURCE_DIR}/vpi/sys_readmem_lex.lex
    COMMENT "Generating sys_readmem_lex.c"
)

add_custom_command(
    OUTPUT sdf_lexor.c
    COMMAND ${FLEX} --outfile=sdf_lexor.c ${PROJECT_SOURCE_DIR}/vpi/sdf_lexor.lex
    COMMENT "Generating sdf_lexor.c"
)

add_custom_command(
    OUTPUT sdf_parse.c
    OUTPUT sdf_parse.h
    COMMAND ${BISON} --verbose -t -p sdf -d -o sdf_parse.c ${PROJECT_SOURCE_DIR}/vpi/sdf_parse.y
    COMMENT "Generating sdf_parse.c"
)

add_custom_command(
    OUTPUT table_mod_lexor.c
    OUTPUT table_mod_lexor.h
    COMMAND ${FLEX} --outfile=table_mod_lexor.c
        ${PROJECT_SOURCE_DIR}/vpi/table_mod_lexor.lex
    COMMENT "Generating table_mod_lexor.c"
)

add_custom_command(
    OUTPUT table_mod_parse.c
    OUTPUT table_mod_parse.h
    COMMAND ${BISON} --verbose -t -p tblmod -d -o table_mod_parse.c ${PROJECT_SOURCE_DIR}/vpi/table_mod_parse.y
    COMMENT "Generating table_mod_parse.c"
)

add_definitions(-D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DHAVE_CONFIG_H -DICARUS_VPI_CONST=const)

add_library(vpi STATIC libvpi.c ${PROJECT_SOURCE_DIR}/ivl/veriuser.h)


if(DEFINED ENV{VERDI_HOME})

	find_library( FSDBWriter
		NAMES nffw
		HINTS "$ENV{VERDI_HOME}/share/FsdbWriter/linux64"
	)

	find_path( FSDBheaderPath 
	    ffwAPI.h 
	    PATHS "$ENV{VERDI_HOME}/share/FsdbWriter"
	)

else()

	find_library( FSDBWriter
		NAMES nffw
	)

	find_path( FSDBheaderPath 
	    ffwAPI.h 
	)
	
endif()

MESSAGE( STATUS "FSDBWriter path: ${FSDBWriter}")
MESSAGE( STATUS "FSDBheaderPath: ${FSDBheaderPath}")

if( FSDBWriter AND FSDBheaderPath )

	MESSAGE( STATUS "complie with FSDB")
	add_definitions("-DUSE_FSDB")

	add_library(system.vpi SHARED sys_table.c sys_convert.c sys_countdrivers.c sys_darray.c sys_deposit.c
	    sys_display.c sys_fileio.c sys_finish.c sys_icarus.c sys_plusargs.c sys_queue.c
	    sys_random.c sys_random_mti.c sys_readmem.c sys_readmem_lex.c sys_scanf.c
	    sys_sdf.c sys_time.c sys_vcd.c sys_fsdb.c sys_vcdoff.c vcd_priv.c mt19937int.c
	    sys_priv.c sdf_parse.c sdf_lexor.c stringheap.c vams_simparam.c
	    table_mod.c table_mod_parse.c table_mod_lexor.c vcd_priv2.cc)

	target_include_directories(system.vpi PUBLIC ${FSDBheaderPath} )

	target_link_libraries(system.vpi z  X11 Xext fontconfig m nsl  #Xss Xt Xmu iberty
		 pthread rt dl png ${FSDBWriter})

else()

	MESSAGE( STATUS "compile without FSDB")
	
	add_library(system.vpi SHARED sys_table.c sys_convert.c sys_countdrivers.c sys_darray.c sys_deposit.c
	    sys_display.c sys_fileio.c sys_finish.c sys_icarus.c sys_plusargs.c sys_queue.c
	    sys_random.c sys_random_mti.c sys_readmem.c sys_readmem_lex.c sys_scanf.c
	    sys_sdf.c sys_time.c sys_vcd.c sys_vcdoff.c vcd_priv.c mt19937int.c
	    sys_priv.c sdf_parse.c sdf_lexor.c stringheap.c vams_simparam.c
	    table_mod.c table_mod_parse.c table_mod_lexor.c vcd_priv2.cc)

endif()

#set_target_properties(system.vpi PROPERTIES LINK_FLAGS -pg)

set_target_properties(system.vpi PROPERTIES PREFIX "")
set_target_properties(system.vpi PROPERTIES SUFFIX "")

add_library(va_math.vpi SHARED va_math.c)
set_target_properties(va_math.vpi PROPERTIES PREFIX "")
set_target_properties(va_math.vpi PROPERTIES SUFFIX "")

add_library(vhdl_sys.vpi SHARED vhdl_table.c sys_priv.c)
set_target_properties(vhdl_sys.vpi PROPERTIES PREFIX "")
set_target_properties(vhdl_sys.vpi PROPERTIES SUFFIX "")

add_library(vhdl_textio.vpi SHARED vhdl_textio.c sys_priv.c)
set_target_properties(vhdl_textio.vpi PROPERTIES PREFIX "")
set_target_properties(vhdl_textio.vpi PROPERTIES SUFFIX "")

add_library(v2005_math.vpi SHARED sys_clog2.c v2005_math.c)
set_target_properties(v2005_math.vpi PROPERTIES PREFIX "")
set_target_properties(v2005_math.vpi PROPERTIES SUFFIX "")

add_library(vpi_debug.vpi SHARED vpi_debug.c)
set_target_properties(vpi_debug.vpi PROPERTIES PREFIX "")
set_target_properties(vpi_debug.vpi PROPERTIES SUFFIX "")

add_library(v2009.vpi SHARED v2009_table.c v2009_array.c 
    v2009_bitvec.c v2009_enum.c v2009_string.c sys_priv.c)
set_target_properties(v2009.vpi PROPERTIES PREFIX "")
set_target_properties(v2009.vpi PROPERTIES SUFFIX "")

install(TARGETS vpi DESTINATION lib/epicsim)
install(TARGETS system.vpi DESTINATION lib/epicsim)
install(TARGETS va_math.vpi DESTINATION lib/epicsim)
install(TARGETS v2005_math.vpi DESTINATION lib/epicsim)
install(TARGETS v2009.vpi DESTINATION lib/epicsim)
install(TARGETS vhdl_sys.vpi DESTINATION lib/epicsim)
install(TARGETS vhdl_textio.vpi DESTINATION lib/epicsim)
install(TARGETS vpi_debug.vpi DESTINATION lib/epicsim)