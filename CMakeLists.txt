cmake_minimum_required(VERSION 3.0)

project(CpuFeatures)

# Default Build Type to be Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# BUILD_TESTING is a standard CMake variable, but we declare it here to make it
# prominent in the GUI.
option(BUILD_TESTING "Enable test (depends on googletest)." OFF)

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

# The use of shared libraries is discouraged.
# For API / ABI compatibility reasons, it is recommended to build and use
# cpu_features in a subdirectory of your project or as an embedded dependency.
if(BUILD_SHARED_LIBS)
  set_property(TARGET cpu_features PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

#
# program : list_cpu_features
#

add_executable(list_cpu_features src/list_cpu_features.c)
target_link_libraries(list_cpu_features PRIVATE cpu_features)

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
