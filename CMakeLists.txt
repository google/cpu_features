cmake_minimum_required(VERSION 3.0)

project(CpuFeatures VERSION 0.1.0)

# Default Build Type to be Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# BUILD_TESTING is a standard CMake variable, but we declare it here to make it
# prominent in the GUI.
option(BUILD_TESTING "Enable test (depends on googletest)." OFF)
# BUILD_SHARED_LIBS is a standard CMake variable, but we declare it here to make
# it prominent in the GUI.
option(BUILD_SHARED_LIBS "Build library as shared." OFF)

#
# library : cpu_features
#

include (CheckIncludeFile)
include (CheckSymbolExists)

check_include_file(dlfcn.h HAVE_DLFCN_H)
check_symbol_exists(getauxval "sys/auxv.h" HAVE_STRONG_GETAUXVAL)

set(_HDRS
  include/cpu_features_macros.h
)

set(_SRCS
  include/internal/bit_utils.h
)

macro(add_linux_detection)
  if(NOT UNIX)
    message(FATAL_ERROR "Use of add_linux_detection() on non Linux-like OS")
  endif()
  list(APPEND _SRCS
    include/internal/linux_features_aggregator.h
    src/linux_features_aggregator.c
    include/internal/hwcaps.h
    src/hwcaps.c
    include/internal/filesystem.h
    src/filesystem.c
    include/internal/stack_line_reader.h
    src/stack_line_reader.c
    include/internal/string_view.h
    src/string_view.c
  )
endmacro()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
    list(APPEND _HDRS include/cpuinfo_mips.h)
    list(APPEND _SRCS src/cpuinfo_mips.c)
    add_linux_detection()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    list(APPEND _HDRS include/cpuinfo_arm.h)
    list(APPEND _SRCS src/cpuinfo_arm.c)
    add_linux_detection()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
    list(APPEND _HDRS include/cpuinfo_aarch64.h)
    list(APPEND _SRCS src/cpuinfo_aarch64.c)
    add_linux_detection()
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR
       CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR
       CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
    list(APPEND _HDRS include/cpuinfo_x86.h)
    list(APPEND _HDRS include/internal/cpuid_x86.h)
    list(APPEND _SRCS src/cpuinfo_x86.c)
    # add_linux_detection() is not needed on x86, we fetch the features directly
    # from the CPU.
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")
    list(APPEND _HDRS include/cpuinfo_ppc.h)
    list(APPEND _SRCS src/cpuinfo_ppc.c)
    add_linux_detection()
else()
  message(FATAL_ERROR "Unsupported architectures ${CMAKE_SYSTEM_PROCESSOR}")
endif()

add_library(cpu_features ${_HDRS} ${_SRCS})

target_include_directories(cpu_features
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/cpu_features>
  PRIVATE
  include/internal
)
set_target_properties(cpu_features PROPERTIES PUBLIC_HEADER "${_HDRS}")
target_compile_definitions(cpu_features PUBLIC STACK_LINE_READER_BUFFER_SIZE=1024)
if(HAVE_DLFCN_H)
  target_compile_definitions(cpu_features PRIVATE HAVE_DLFCN_H)
endif()
if(HAVE_STRONG_GETAUXVAL)
  target_compile_definitions(cpu_features PRIVATE HAVE_STRONG_GETAUXVAL)
endif()
target_link_libraries(cpu_features PUBLIC ${CMAKE_DL_LIBS})

# The use of shared libraries is discouraged.
# For API / ABI compatibility reasons, it is recommended to build and use
# cpu_features in a subdirectory of your project or as an embedded dependency.
if(BUILD_SHARED_LIBS)
  set_property(TARGET cpu_features PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
add_library(CpuFeature::cpu_features ALIAS cpu_features)

#
# program : list_cpu_features
#

add_executable(list_cpu_features src/utils/list_cpu_features.c)
target_link_libraries(list_cpu_features PRIVATE cpu_features)
add_executable(CpuFeature::list_cpu_features ALIAS list_cpu_features)

#
# tests
#

include(CTest)
if(BUILD_TESTING)
  # Automatically incorporate googletest into the CMake Project if target not
  # found.
  if(NOT TARGET gtest OR NOT TARGET gmock_main)
    # Download and unpack googletest at configure time.
    configure_file(
      cmake/googletest.CMakeLists.txt.in
      googletest-download/CMakeLists.txt
    )

    execute_process(
      COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

    if(result)
      message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()

    execute_process(
      COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

    if(result)
      message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker settings on
    # Windows.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines the gtest and
    # gtest_main targets.
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                     ${CMAKE_BINARY_DIR}/googletest-build
                     EXCLUDE_FROM_ALL)
  endif()

  add_subdirectory(test)
endif()

#
# Install
#

include(GNUInstallDirs)
install(TARGETS cpu_features list_cpu_features
  EXPORT CpuFeaturesTargets
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cpu_features
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(EXPORT CpuFeaturesTargets
  NAMESPACE CpuFeatures::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CpuFeatures
  COMPONENT Devel
)
include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/CpuFeaturesConfig.cmake.in
  "${PROJECT_BINARY_DIR}/CpuFeaturesConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CpuFeatures"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/CpuFeaturesConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
  "${PROJECT_BINARY_DIR}/CpuFeaturesConfig.cmake"
  "${PROJECT_BINARY_DIR}/CpuFeaturesConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CpuFeatures"
  COMPONENT Devel
)
