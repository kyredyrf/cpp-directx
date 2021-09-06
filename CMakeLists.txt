cmake_minimum_required(VERSION 3.10)

# set the project name
project(cpp-directx VERSION 0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(./src/config.h.in config.h)

add_definitions(-D_UNICODE -DUNICODE)

# add the executable
file(GLOB_RECURSE SOURCES ./src/*.h ./src/*.cpp)
add_executable(cpp-directx WIN32 ${SOURCES})

file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} NATIVE_PROJECT_SOURCE_DIR)
file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR} NATIVE_PROJECT_BINARY_DIR)
target_include_directories(
    cpp-directx PUBLIC
    "${NATIVE_PROJECT_BINARY_DIR}"
    "${NATIVE_PROJECT_SOURCE_DIR}\\src"
    "..\\..\\..\\microsoft\\DirectX-Headers\\include"
)

target_link_libraries(
    cpp-directx
    d3d12.lib
    d3dcompiler.lib
    dxgi.lib
    dxguid.lib
)

target_compile_options(
    cpp-directx PRIVATE
    /wd4819 # 指定の警告を無効にする -> warning C4819: ファイルは、現在のコード ページ (932) で表示できない文字を含んでいます。データの損失を防ぐために、ファイルを Unicode 形式で保存してください。
    /FIconfig.h # 必ず使用されるインクルードファイル
)

add_custom_command(
    TARGET cpp-directx
    POST_BUILD
    COMMAND copy /Y \"${NATIVE_PROJECT_SOURCE_DIR}\\assets\\shaders\\*\" /B \"$(ProjectDir)\"
)
