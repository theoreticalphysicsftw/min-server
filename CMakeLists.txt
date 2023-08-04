# This file is part of min-server.
#
# min-server is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# min-server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with min-server.  If not, see <http:#www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.20)

if(UNIX)
    set(CMAKE_C_COMPILER "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif(UNIX)
project(min-server CXX)
enable_language(C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    add_compile_options("/Zc:__cplusplus")
endif()


if(UNIX)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    add_link_options("-fuse-ld=lld")

    set(
         CMAKE_CXX_FLAGS
         "${CMAKE_CXX_FLAGS} -march=native"
       )
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2 /arch:AVX")
endif()


if (WIN32)
    set(RELEASE_OPTIONS "/MD /O3 /Ob2 /DNDEBUG")
else()
    set(RELEASE_OPTIONS "-O3 -ffast-math -fno-eceptions")
endif()


if (WIN32)
    set(DOCUMENT_ROOT "C:/min-server")
else()
    set(DOCUMENT_ROOT "/min-server")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DVI_DEBUG)
endif ()

file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB MONGOOSE_SOURCES "third_party/mongoose/*.c")


add_library(${PROJECT_NAME}_mongoose STATIC ${MONGOOSE_SOURCES})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_compile_definitions(${PROJECT_NAME}_mongoose PUBLIC MG_ENABLE_MBEDTLS=1)
target_compile_definitions(${PROJECT_NAME} PUBLIC MG_ENABLE_MBEDTLS=1)

target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_mongoose)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC "bcrypt.lib")
endif(WIN32)

target_compile_definitions(${PROJECT_NAME} PUBLIC DOCUMENT_ROOT="${DOCUMENT_ROOT}")
target_include_directories(${PROJECT_NAME} PUBLIC "third_party")
target_include_directories(${PROJECT_NAME} PUBLIC "src")
target_include_directories(${PROJECT_NAME}_mongoose PUBLIC "third_party/mbedtls/include")

set(ENABLE_TESTS OFF)
set(ENABLE_PROGRAMS OFF)
add_subdirectory("third_party/mbedtls")

target_link_libraries(${PROJECT_NAME} PRIVATE mbedtls)


file(GLOB CLIENT_SIDE_RESOURCES "scripts/*.js" "css/*.css" "icons/*.svg")

foreach(clientSideResource ${CLIENT_SIDE_RESOURCES})
    get_filename_component(resName ${clientSideResource} NAME)
    execute_process(COMMAND "brotli" "-9" "-f" "-o" "${DOCUMENT_ROOT}/res/${resName}.br" ${clientSideResource})
    message("brotli -9 -o ${DOCUMENT_ROOT}/res/${resName}.br ${clientSideResource}")
    configure_file(${clientSideResource} "${DOCUMENT_ROOT}/res/${resName}" COPYONLY)
endforeach()

configure_file("main_page/main_page.html" "${DOCUMENT_ROOT}/main_page.html" COPYONLY)

file(GLOB FONTS "fonts/*.woff" "fonts/*.woff2")

foreach(font ${FONTS})
    get_filename_component(fontName ${font} NAME)
    configure_file(${font} "${DOCUMENT_ROOT}/res/${fontName}" COPYONLY)
endforeach()

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_sources(${PROJECT_NAME}
               PRIVATE
               ${SRC_DIR}
               ${CTL_SRC}
               ${FILTER_SRC}
               ${PLUGIN_SRC}
               ${MODEL_SRC})
