cmake_minimum_required(VERSION 4.3)
project(Slnx2CompileCmd VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
pugixml
GIT_REPOSITORY https://github.com/zeux/pugixml.git
GIT_TAG v1.15
)
FetchContent_MakeAvailable(pugixml)

include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)


file(GLOB SRC src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE pugixml argparse nlohmann_json::nlohmann_json)