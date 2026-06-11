add_library(packlang_core STATIC core.cpp)
target_include_directories(packlang_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(packc main.cpp)
target_link_libraries(packc PRIVATE packlang_core)
