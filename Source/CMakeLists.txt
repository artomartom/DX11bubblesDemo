cmake_minimum_required(VERSION 3.22)
project(bubblesDemo)
set(CMAKE_CXX_STANDARD 20)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/App.cpp
  ${PROJECT_SOURCE_DIR}/bubblesDemo.exe.manifest
  ${PROJECT_SOURCE_DIR}/Visual/DeviceResource.cpp
  
)

option(MAKEGUI "build  as a GUI rather then console app" OFF)

if(MAKEGUI)
  set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
  target_compile_definitions(${PROJECT_NAME} PUBLIC GUI)
else()
  target_compile_definitions(${PROJECT_NAME} PUBLIC CONSOLE)
endif(MAKEGUI)

unset(MAKEGUI CACHE) # <---- this is important

target_link_libraries(${PROJECT_NAME} PRIVATE D3D11 DXGI dxguid)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)
