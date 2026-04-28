cmake_minimum_required(VERSION 3.20)
project(rvmparser LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RVMPARSER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIBTESS2_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/libtess2/Source)
set(LIBTESS2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/libtess2/Include)
set(RAPIDJSON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/rapidjson/include)

set(LIBTESS2_SOURCES
    ${LIBTESS2_SRC_DIR}/bucketalloc.c
    ${LIBTESS2_SRC_DIR}/dict.c
    ${LIBTESS2_SRC_DIR}/geom.c
    ${LIBTESS2_SRC_DIR}/mesh.c
    ${LIBTESS2_SRC_DIR}/priorityq.c
    ${LIBTESS2_SRC_DIR}/sweep.c
    ${LIBTESS2_SRC_DIR}/tess.c
)

set(RVMPARSER_CORE_SOURCES
    ${RVMPARSER_SRC_DIR}/AddGroupBBox.cpp
    ${RVMPARSER_SRC_DIR}/AddStats.cpp
    ${RVMPARSER_SRC_DIR}/Align.cpp
    ${RVMPARSER_SRC_DIR}/ChunkTiny.cpp
    ${RVMPARSER_SRC_DIR}/Colorizer.cpp
    ${RVMPARSER_SRC_DIR}/Common.cpp
    ${RVMPARSER_SRC_DIR}/Connect.cpp
    ${RVMPARSER_SRC_DIR}/DiscardGroups.cpp
    ${RVMPARSER_SRC_DIR}/DumpNames.cpp
    ${RVMPARSER_SRC_DIR}/ExportGLTF.cpp
    ${RVMPARSER_SRC_DIR}/ExportJson.cpp
    ${RVMPARSER_SRC_DIR}/ExportObj.cpp
    ${RVMPARSER_SRC_DIR}/ExportRev.cpp
    ${RVMPARSER_SRC_DIR}/Flatten.cpp
    ${RVMPARSER_SRC_DIR}/FlattenRegex.cpp
    ${RVMPARSER_SRC_DIR}/LinAlgOps.cpp
    ${RVMPARSER_SRC_DIR}/ParserAtt.cpp
    ${RVMPARSER_SRC_DIR}/ParserRVM.cpp
    ${RVMPARSER_SRC_DIR}/Store.cpp
    ${RVMPARSER_SRC_DIR}/Tessellator.cpp
    ${RVMPARSER_SRC_DIR}/TriangulationFactory.cpp
)

add_library(rvmparser_core STATIC
    ${RVMPARSER_CORE_SOURCES}
    ${LIBTESS2_SOURCES}
)

target_include_directories(rvmparser_core PUBLIC
    ${RVMPARSER_SRC_DIR}
    ${LIBTESS2_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)

if (MSVC)
    target_compile_options(rvmparser_core PRIVATE /W4)
else()
    target_compile_options(rvmparser_core PRIVATE -Wall -Wextra)
endif()

add_executable(rvmparser
    ${RVMPARSER_SRC_DIR}/main.cpp
)

target_link_libraries(rvmparser PRIVATE rvmparser_core)

add_library(rvm_parser SHARED
    ${RVMPARSER_SRC_DIR}/RvmParserDll.cpp
)

target_compile_definitions(rvm_parser PRIVATE RVMPARSER_DLL_BUILD)
target_link_libraries(rvm_parser PRIVATE rvmparser_core)
set_target_properties(rvm_parser PROPERTIES
    OUTPUT_NAME "rvm_parser"
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
)

install(TARGETS rvmparser rvm_parser
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(FILES ${RVMPARSER_SRC_DIR}/RvmParserDll.h DESTINATION include)
