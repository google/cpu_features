cmake_minimum_required(VERSION 3.0)

project(CpuFeatures)

#
# library : cpu_features
#

add_library(cpu_features
  include/cpuinfo_aarch64.h
  include/cpuinfo_arm.h
  include/cpuinfo_mips.h
  include/cpuinfo_x86.h
  include/internal/bit_utils.h
  include/internal/linux_features_aggregator.h
  include/internal/cpuid_x86.h
  include/internal/filesystem.h
  include/internal/hwcaps.h
  include/internal/stack_line_reader.h
  include/internal/string_view.h
  include/cpu_features_macros.h
  src/linux_features_aggregator.c
  src/cpuid_x86_clang.c
  src/cpuid_x86_gcc.c
  src/cpuid_x86_msvc.c
  src/cpuinfo_aarch64.c
  src/cpuinfo_arm.c
  src/cpuinfo_mips.c
  src/cpuinfo_x86.c
  src/filesystem.c
  src/hwcaps.c
  src/stack_line_reader.c
  src/string_view.c
)

target_include_directories(cpu_features PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_include_directories(cpu_features PRIVATE include/internal)
target_compile_definitions(cpu_features PUBLIC STACK_LINE_READER_BUFFER_SIZE=1024)
target_link_libraries(cpu_features PUBLIC ${CMAKE_DL_LIBS})

#
# program : list_cpu_features
#

add_executable(list_cpu_features src/list_cpu_features.cc)
target_link_libraries(list_cpu_features PRIVATE cpu_features)
target_compile_features(list_cpu_features PRIVATE cxx_range_for)

#
# tests
#

include(CTest)
if(BUILD_TESTING)
  # Download and unpack googletest at configure time.
  configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)

  execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()

  execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker settings on Windows.
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines the gtest and gtest_main
  # targets.
  add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                   ${CMAKE_BINARY_DIR}/googletest-build
                   EXCLUDE_FROM_ALL)

  # The gtest/gtest_main targets carry header search path dependencies
  # automatically when using CMake 2.8.11 or later. Otherwise we have to add
  # them here ourselves.
  if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
  endif()
  
  add_subdirectory(test)
endif()
