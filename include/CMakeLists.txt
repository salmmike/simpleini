
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    .
)

set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=.;
    -extra-arg=-std=c++20)
