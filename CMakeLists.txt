PROJECT(linetimer C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include(CheckCCompilerFlag)

check_c_compiler_flag(-std=c99 temp)
if(temp)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()
check_c_compiler_flag(-Wall temp)
if(temp)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()
check_c_compiler_flag(-Wextra temp)
if(temp)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
endif()
check_c_compiler_flag(-pedantic temp)
if(temp)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
endif()

include(FindOpenMP)
if(OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()

add_executable(linetimer main.c gopt.c)
add_executable(test test.c)

target_link_libraries(linetimer m rt)
target_link_libraries(test rt)

install(TARGETS linetimer RUNTIME DESTINATION bin)

