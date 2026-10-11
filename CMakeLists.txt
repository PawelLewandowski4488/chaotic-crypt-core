cmake_minimum_required(VERSION 3.20)

project(chaotic_crypt_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(imagecrypto SHARED
    src/crypto.cpp
    src/crypto_api.cpp
    src/map.cpp
    src/sha256.cpp
)

target_include_directories(imagecrypto PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(imagecrypto PRIVATE CRYPTO_BUILD)

add_executable(crypto_test
    tests/main.cpp
    bindings/dotnet/ImageCrypto.cs
)

target_include_directories(crypto_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_parties
)

target_link_libraries(crypto_test PRIVATE imagecrypto)