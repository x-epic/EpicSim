include_directories(${PROJECT_SOURCE_DIR}/driver)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/ivl)

file(GLOB tgt_vvp_src
    "*.c" "*.cc"
)

add_library(epicsim_generator SHARED ${tgt_vvp_src})

add_custom_command(
        TARGET epicsim_generator PRE_BUILD
        COMMAND cat ${PROJECT_SOURCE_DIR}/tgt-vvp/vvp.conf.in > ${CMAKE_BUILD_DIR}/tgt-vvp/vvp.conf
        COMMAND echo 'flag:VVP_EXECUTABLE=${CMAKE_INSTALL_PREFIX}/bin/epicsim-vvp' >> ${CMAKE_BUILD_DIR}/tgt-vvp/vvp.conf
        COMMENT "Generating vvp.conf"
)
add_custom_command(
        TARGET epicsim_generator PRE_BUILD
        COMMAND cat ${PROJECT_SOURCE_DIR}/tgt-vvp/vvp-s.conf.in > vvp-s.conf
        COMMAND echo 'flag:VVP_EXECUTABLE=${CMAKE_INSTALL_PREFIX}/bin/epicsim-vvp' >> vvp-s.conf
        COMMENT "Generating vvp-s.conf"
)


install(TARGETS epicsim_generator DESTINATION lib/epicsim)
install(FILES ${CMAKE_BUILD_DIR}/tgt-vvp/vvp.conf DESTINATION lib/epicsim)
install(FILES ${CMAKE_BUILD_DIR}/tgt-vvp/vvp-s.conf DESTINATION lib/epicsim)
