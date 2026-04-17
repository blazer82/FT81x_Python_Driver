cmake_minimum_required(VERSION 3.20)
project(ft81x C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

add_library(ft81x SHARED
    src/ft81x.c
    src/spi_linux.c
    src/gpio_linux.c
)

target_include_directories(ft81x PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GPIOD_INCLUDE_DIRS}
)

target_link_libraries(ft81x PRIVATE ${GPIOD_LIBRARIES})

set_target_properties(ft81x PROPERTIES
    OUTPUT_NAME "libft81x"
    PREFIX ""
    SUFFIX ".so"
)

install(TARGETS ft81x
    LIBRARY DESTINATION ft81x
)
