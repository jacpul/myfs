cmake_minimum_required(VERSION 3.5)
project(fly_swamp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        log.c
        log.h
        disk.c
        disk.h
        myfs.c
        myfs.h
        fly_swamp.c
        fly_swamp.h
        params.h)

add_executable(fly_swamp ${SOURCE_FILES})
target_link_libraries(fly_swamp ${FUSE_LIBRARIES})
target_include_directories(fly_swamp PUBLIC ${FUSE_INCLUDE_DIRS})
target_compile_options(fly_swamp PUBLIC ${FUSE_CFLAGS_OTHER})
