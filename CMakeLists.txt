cmake_minimum_required(VERSION 3.10)
project(SelfDrivingRobot C)

# Strikta kompilatorflaggor
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")

# Lägg till källkod och header-mappar
include_directories(${PROJECT_SOURCE_DIR}/Include)

# Lista alla källfiler
file(GLOB SOURCES
     "${PROJECT_SOURCE_DIR}/Sourcefiles/*.c")

# Skapa exekverbar fil
add_executable(SelfDrivingRobot ${SOURCES})
