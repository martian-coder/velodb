cmake_minimum_required(VERSION 3.20)
project(velodb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG        asio-1-30-2
)
FetchContent_MakeAvailable(asio)

# Target for asio if it doesn't create one
if(NOT TARGET asio)
  add_library(asio INTERFACE)
  target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
endif()

# ==== Core library ==========================================================
add_library(velodb STATIC
    src/db.cpp
    src/lf_btree.cpp
    src/hlsm.cpp
    src/sai.cpp
    src/pwac.cpp
    src/snapshot.cpp
)

target_include_directories(velodb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ==== Server ================================================================
add_executable(velodb-server server/server.cpp)
target_link_libraries(velodb-server PRIVATE velodb asio)
target_compile_definitions(velodb-server PRIVATE ASIO_STANDALONE)

# ==== Client (CLI) =========================================================
add_executable(velodb-client server/client.cpp)
target_link_libraries(velodb-client PRIVATE velodb asio)
target_compile_definitions(velodb-client PRIVATE ASIO_STANDALONE)

# ==== Python bindings =======================================================
option(BUILD_PYTHON "Build Python bindings" ON)
if(BUILD_PYTHON)
    find_package(Python3 3.6 COMPONENTS Interpreter Development REQUIRED)
    
    include(FetchContent)
    FetchContent_Declare(
      pybind11
      GIT_REPOSITORY https://github.com/pybind/pybind11
      GIT_TAG        v2.11.1
    )
    FetchContent_MakeAvailable(pybind11)
    
    pybind11_add_module(velodb_py python/velodb_py.cpp)
    target_link_libraries(velodb_py PRIVATE velodb)
    
    # Ensure Windows generates a .pyd file and doesn't conflict with MSVC flags
    if(MSVC)
        target_compile_options(velodb_py PRIVATE /bigobj)
    endif()
endif()

# ==== Tests =================================================================
enable_testing()
add_subdirectory(tests)
