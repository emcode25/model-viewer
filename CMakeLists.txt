# CMakeList.txt : CMake project for model-viewer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("model-viewer")

# -----SDL----
# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(thirdparty/SDL EXCLUDE_FROM_ALL)

# -----END SDL-----

# Add source to this project's executable.
file(GLOB SRC_FILES CONFIGURE_DEPENDS "src/*/*.cpp" "src/*/*.c" "src/*.c" "src/*.cpp" "thirdparty/glad/src/glad.c" "thirdparty/imgui/*.cpp")
add_executable (model-viewer ${SRC_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET model-viewer PROPERTY CXX_STANDARD 20)
endif()

# Link to the actual SDL3 library.
target_link_libraries(model-viewer PRIVATE SDL3::SDL3)

target_include_directories(model-viewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)