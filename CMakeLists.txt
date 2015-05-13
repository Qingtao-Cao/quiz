
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

# CMP0011 should be set here, otherwise policy set in module won't affect
# here.
CMAKE_POLICY(SET CMP0011 OLD)

# Compiler 
SET(CMAKE_C_COMPILER "gcc")


######################################
# Project details
#
PROJECT(C)
SET(PRJ_SUMMARY "Cog System quiz")

SET(PRJ_DESCRIPTION
        "Technical Quiz"
   )
SET(LICENSE "GPLv3+")


######################################
# Create an out-of-source build directory.
# Keeps source trees clean.
#
SET (CMAKE_BUILD_DIR "build")
ADD_SUBDIRECTORY(src ${CMAKE_BUILD_DIR})

######################################
# make quiz-clean
#
ADD_CUSTOM_TARGET(quiz-clean
    COMMENT "Removing: build/, CMakeCache.txt, CMakeFiles/"
    COMMAND rm -rf ${CMAKE_BUILD_DIR} CMakeCache.txt CMakeFiles)

