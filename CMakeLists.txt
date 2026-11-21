cmake_minimum_required(VERSION 3.15)

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet")

set(CMAKE_TOOLCHAIN_FILE "C:/Users/Svetlana/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(DP_SCAN)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS locale)

add_subdirectory(Utility)

add_executable(DP_SCAN DP_SCAN/main_scan.cpp DP_SCAN/TextProcessor.cpp DP_SCAN/ThreadPool.cpp DP_SCAN/FileScanner.cpp)

target_include_directories(DP_SCAN PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DP_SCAN PRIVATE DP_Utility)
target_link_libraries(DP_SCAN PRIVATE Boost::locale)

if(WIN32)
    target_link_libraries(DP_SCAN PRIVATE
        ws2_32
        secur32
        shell32
        advapi32
        user32
    )
endif()

add_executable(DP_SEARCH DP_SEARCH/main_search.cpp DP_SEARCH/QueryParser.cpp)

target_include_directories(DP_SEARCH PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DP_SEARCH PRIVATE DP_Utility)
target_link_libraries(DP_SEARCH PRIVATE Boost::locale)

if(WIN32)
    target_link_libraries(DP_SEARCH PRIVATE
        ws2_32
        secur32
        shell32
        advapi32
        user32
    )
endif()
