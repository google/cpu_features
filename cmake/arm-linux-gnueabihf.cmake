SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc -mfloat-abi=hard)
SET(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++ -mfloat-abi=hard)
SET(CMAKE_AR arm-linux-gnueabihf-ar CACHE FILEPATH "Archiver")

SET(THREADS_PTHREAD_ARG "2" CACHE STRING "Forcibly set by CMakeLists.txt." FORCE)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH  $ENV{TOOLCHAIN})

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
