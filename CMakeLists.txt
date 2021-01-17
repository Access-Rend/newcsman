cmake_minimum_required(VERSION 3.4)
project(csman)
set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_subdirectory(imports/mozart)
include_directories(imports/mozart/include)

include_directories(imports/zip/include)
add_library(zip STATIC imports/zip/zip.c)

ADD_COMPILE_OPTIONS("-g")

add_executable(csman command.cpp idx.hpp pac_repo.hpp global.hpp http.hpp dir.hpp)

target_link_libraries(csman curl mozart zip)

if(WIN32)
    target_compile_definitons(csman PRIVATE CURL_STATICLIB)
    target_link_libraries(csman nghttp2 ssl crypto zstd ssh2 z brotlidec-static brotlicommon-static ws2_32 wldap32 ccrypt32)
endif()

#add_executable(csman sourcesreader.cpp)