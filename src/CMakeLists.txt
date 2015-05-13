ADD_EXECUTABLE(analysis analysis.c node.c lib.c)
ADD_EXECUTABLE(analysis_hp analysis_hp.c node.c lib.c)

TARGET_LINK_LIBRARIES(analysis pthread ${LIBS})
TARGET_LINK_LIBRARIES(analysis_hp pthread ${LIBS})

######################################
# Compiler flags 
#
SET (CMAKE_C_FLAGS "-Wall -Werror -O2")
SET (CMAKE_C_FLAGS_DEBUG "-g -Wall -Werror")
SET (CMAKE_C_FLAGS_MINSIZEREL  "-Os -Werror")
SET (CMAKE_C_FLAGS_RELEASE  "-O4 -Werror")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -Werror")
SET (CMAKE_LINKER "/usr/bin/ld")


