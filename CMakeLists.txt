project(varjo-dx11-minimal)
cmake_minimum_required(VERSION 3.18)

add_definitions(
    -DASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    -DNOMINMAX
)

set(_varjo_sdk_dir ${CMAKE_CURRENT_SOURCE_DIR}/varjo_sdk/)

add_library(VarjoLib SHARED IMPORTED GLOBAL)
set_target_properties(VarjoLib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_varjo_sdk_dir}/include/"
    IMPORTED_LOCATION "${_varjo_sdk_dir}/bin/VarjoLib.dll"
    IMPORTED_IMPLIB "${_varjo_sdk_dir}/lib/VarjoLib.lib"
)

add_subdirectory(submodules/glm)

file(GLOB _source_list ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

set(_target varjo-dx11-minimal)
add_executable(${_target} WIN32 ${_source_list})
target_link_libraries(${_target} PUBLIC VarjoLib d3d11 glm::glm d3dcompiler dxgi # dxguid
 )
 target_include_directories(${_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/submodules/stb/)

add_custom_command(
    TARGET ${_target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_varjo_sdk_dir}/bin/VarjoLib.dll" $<TARGET_FILE_DIR:${_target}>
)
