

# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER  /usr/bin/armv5tel-softfloat-linux-gnueabi-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/armv5tel-softfloat-linux-gnueabi-g++)
SET(CMAKE_FORCE_C_COMPILER  /usr/bin/armv5tel-softfloat-linux-gnueabi-gcc)
SET(CMAKE_FORCE_CXX_COMPILER /usr/bin/armv5tel-softfloat-linux-gnueabi-g++)
# specify the cross compiler

# where is the target environment 

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
