cmake_minimum_required (VERSION 3.12)
project ("void")

add_executable (void engine/main.cpp)

set_property(TARGET void PROPERTY CXX_STANDARD 17)
