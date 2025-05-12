cmake_minimum_required(VERSION 3.20)

project(CppDiplomProject)

set(OPENSSL_ROOT_DIR "C:\\Program Files\\OpenSSL-Win64")


set(BOOST_ROOT "C:\\Boost\\boost_1_82_0\\boost")


set(libpqxx_DIR "C:\\libpqxx")


add_compile_definitions(_WIN32_WINNT=0x0A01)


# Boost:
find_package(Boost 1.80.0 REQUIRED COMPONENTS system thread locale)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost")
endif()


# PQXX
set(PostgreSQL_ADDITIONAL_VERSIONS "15")

add_subdirectory(${libpqxx_DIR} libpqxx-build) 


# OpenSSL
find_package(OpenSSL REQUIRED)

add_subdirectory(spider)

add_subdirectory(http_server)

