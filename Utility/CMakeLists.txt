find_package(libpqxx CONFIG REQUIRED)

add_library(DP_Utility STATIC
    MyDatabase.cpp ini_parser.cpp Utils.cpp ConnectionPool.cpp
)

target_include_directories(DP_Utility PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(DP_Utility PUBLIC libpqxx::pqxx)
