cmake_minimum_required(VERSION 3.16)
project(limit_order_book LANGUAGES CXX)

add_library(order_book src/order_book.cpp)
target_include_directories(order_book PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(order_book PUBLIC cxx_std_17)
set_target_properties(order_book PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(order_book PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(order_book_demo examples/demo.cpp)
target_link_libraries(order_book_demo PRIVATE order_book)
set_target_properties(order_book_demo PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(order_book_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()
