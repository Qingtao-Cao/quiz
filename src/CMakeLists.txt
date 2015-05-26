IF (CMAKE_BUILD_TYPE MATCHES THREADS)
	ADD_EXECUTABLE(analysis_m analysis_m.c node.c lib.c)
	TARGET_LINK_LIBRARIES(analysis_m pthread ${LIBS})
ELSE (CMAKE_BUILD_TYPE MATCHES THREADS)
	ADD_EXECUTABLE(analysis_s analysis_s.c node.c lib.c)
	TARGET_LINK_LIBRARIES(analysis_s ${LIBS})
ENDIF (CMAKE_BUILD_TYPE MATCHES THREADS)

######################################
# Compiler flags 
#
SET (CMAKE_C_FLAGS "-Wall -Werror -O2")
SET (CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
SET (CMAKE_C_FLAGS_THREADS "-DMULTI_THREADS")
SET (CMAKE_C_FLAGS_MINSIZEREL  "-Os -Werror")
SET (CMAKE_LINKER "/usr/bin/ld")
