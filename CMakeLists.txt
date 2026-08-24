cmake_minimum_required(VERSION 3.16)

project(chaotic-crypt-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chaotic-crypt-core main.cpp)

include(GNUInstallDirs)
install(TARGETS chaotic-crypt-core
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
