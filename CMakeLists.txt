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

include (CheckIncludeFile)
include (CheckSymbolExists)

macro(setup_include_and_definitions TARGET_NAME)
  target_include_directories(${TARGET_NAME}
    PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/internal>
  )
  target_compile_definitions(${TARGET_NAME}
    PUBLIC STACK_LINE_READER_BUFFER_SIZE=1024
  )
endmacro()

set(PROCESSOR_IS_MIPS FALSE)
set(PROCESSOR_IS_ARM FALSE)
set(PROCESSOR_IS_AARCH64 FALSE)
set(PROCESSOR_IS_X86 FALSE)
set(PROCESSOR_IS_POWER FALSE)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
  set(PROCESSOR_IS_MIPS TRUE)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(PROCESSOR_IS_ARM TRUE)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
  set(PROCESSOR_IS_AARCH64 TRUE)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(AMD64)|(^i.86$)")
  set(PROCESSOR_IS_X86 TRUE)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")
  set(PROCESSOR_IS_POWER TRUE)
endif()

macro(add_arch_sources HDRS_LIST_NAME SRCS_LIST_NAME)
  list(APPEND ${HDRS_LIST_NAME} include/cpu_features_macros.h)
  if(PROCESSOR_IS_MIPS)
      list(APPEND ${HDRS_LIST_NAME} include/cpuinfo_mips.h)
      list(APPEND ${SRCS_LIST_NAME} src/cpuinfo_mips.c)
  elseif(PROCESSOR_IS_ARM)
      list(APPEND ${HDRS_LIST_NAME} include/cpuinfo_arm.h)
      list(APPEND ${SRCS_LIST_NAME} src/cpuinfo_arm.c)
  elseif(PROCESSOR_IS_AARCH64)
      list(APPEND ${HDRS_LIST_NAME} include/cpuinfo_aarch64.h)
      list(APPEND ${SRCS_LIST_NAME} src/cpuinfo_aarch64.c)
  elseif(PROCESSOR_IS_X86)
      list(APPEND ${HDRS_LIST_NAME} include/cpuinfo_x86.h)
      list(APPEND ${HDRS_LIST_NAME} include/internal/cpuid_x86.h)
      list(APPEND ${SRCS_LIST_NAME} src/cpuinfo_x86.c)
  elseif(PROCESSOR_IS_POWER)
      list(APPEND ${HDRS_LIST_NAME} include/cpuinfo_ppc.h)
      list(APPEND ${SRCS_LIST_NAME} src/cpuinfo_ppc.c)
  else()
    message(FATAL_ERROR "Unsupported architectures ${CMAKE_SYSTEM_PROCESSOR}")
  endif()
endmacro()

#
# library : utils
#

add_library(utils OBJECT
  include/internal/bit_utils.h
  include/internal/filesystem.h
  include/internal/stack_line_reader.h
  include/internal/string_view.h
  src/filesystem.c
  src/stack_line_reader.c
  src/string_view.c
)
setup_include_and_definitions(utils)

#
# library : linux_based_hardware_detection
#

if(UNIX)
  add_library(linux_based_hardware_detection OBJECT
    include/internal/hwcaps.h
    include/internal/linux_features_aggregator.h
    src/hwcaps.c
    src/linux_features_aggregator.c
  )
  setup_include_and_definitions(linux_based_hardware_detection)
  check_include_file(dlfcn.h HAVE_DLFCN_H)
  if(HAVE_DLFCN_H)
    target_compile_definitions(linux_based_hardware_detection PRIVATE HAVE_DLFCN_H)
  endif()
  check_symbol_exists(getauxval "sys/auxv.h" HAVE_STRONG_GETAUXVAL)
  if(HAVE_STRONG_GETAUXVAL)
    target_compile_definitions(linux_based_hardware_detection PRIVATE HAVE_STRONG_GETAUXVAL)
  endif()
endif()

#
# library : cpu_features
#
set (CPU_FEATURES_HDRS)
set (CPU_FEATURES_SRCS)
add_arch_sources(CPU_FEATURES_HDRS CPU_FEATURES_SRCS)
list(APPEND CPU_FEATURES_SRCS $<TARGET_OBJECTS:utils>)
if(NOT PROCESSOR_IS_X86 AND UNIX)
  list(APPEND CPU_FEATURES_SRCS $<TARGET_OBJECTS:linux_based_hardware_detection>)
endif()
add_library(cpu_features ${CPU_FEATURES_HDRS} ${CPU_FEATURES_SRCS})
setup_include_and_definitions(cpu_features)
target_link_libraries(cpu_features PUBLIC ${CMAKE_DL_LIBS})

#
# program : list_cpu_features
#

add_executable(list_cpu_features src/utils/list_cpu_features.c)
target_link_libraries(list_cpu_features PRIVATE cpu_features)
add_executable(CpuFeature::list_cpu_features ALIAS list_cpu_features)

#
# library : NDK compat
#
if(UNIX)
  find_package(Threads)
  set (NDK_COMPAT_HDRS ndk_compat/cpu-features.h)
  set (NDK_COMPAT_SRCS
    ndk_compat/cpu-features.c
    $<TARGET_OBJECTS:utils>
    $<TARGET_OBJECTS:linux_based_hardware_detection>
  )
  add_arch_sources(NDK_COMPAT_HDRS NDK_COMPAT_SRCS)
  add_library(ndk_compat ${NDK_COMPAT_HDRS} ${NDK_COMPAT_SRCS})
  setup_include_and_definitions(ndk_compat)
  target_link_libraries(ndk_compat PUBLIC ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

#
# program : NDK compat test program
#
if(UNIX)
  add_executable(ndk-compat-test ndk_compat/ndk-compat-test.c)
  target_link_libraries(ndk-compat-test PUBLIC ndk_compat)
endif()

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
