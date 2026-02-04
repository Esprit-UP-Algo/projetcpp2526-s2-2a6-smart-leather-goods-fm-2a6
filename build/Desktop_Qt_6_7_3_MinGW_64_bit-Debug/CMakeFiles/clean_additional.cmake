# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\ProjetCpp_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ProjetCpp_autogen.dir\\ParseCache.txt"
  "ProjetCpp_autogen"
  )
endif()
