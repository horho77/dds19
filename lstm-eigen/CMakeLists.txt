cmake_minimum_required(VERSION 3.19)

set(name lstm-eigen)
project(${name} VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

add_executable(${name}
        src/main.cpp)

include(cpm/CPM.cmake)
CPMAddPackage("gh:adamstark/AudioFile#master")
CPMAddPackage(
        NAME nlohmann_json
        GITHUB_REPOSITORY nlohmann/json
        VERSION 3.10.2)

find_package (Eigen3 REQUIRED NO_MODULE)

target_link_libraries(${name}
    PRIVATE
        AudioFile
        nlohmann_json
        Eigen3::Eigen)