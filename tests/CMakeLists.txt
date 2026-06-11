include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

add_executable(packlang_tests test_main.cpp)
target_link_libraries(packlang_tests PRIVATE packlang_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(packlang_tests)
