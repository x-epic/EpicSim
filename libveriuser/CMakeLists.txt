include_directories(${PROJECT_SOURCE_DIR}/driver)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/ivl)


file(GLOB veriuser_src
    "*.c"
)

add_library(veriuser STATIC ${veriuser_src})

install(TARGETS veriuser DESTINATION lib/epicsim)
