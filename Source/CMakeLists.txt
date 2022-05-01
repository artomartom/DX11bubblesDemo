cmake_minimum_required(VERSION 3.22)
project(DX11bubblesDemo)
set(CMAKE_CXX_STANDARD 20)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/App.cpp
  ${PROJECT_SOURCE_DIR}/DX11bubblesDemo.exe.manifest
  ${PROJECT_SOURCE_DIR}/Visual/DeviceResource.cpp
  ${PROJECT_SOURCE_DIR}/Visual/Renderer.cpp
  ${PROJECT_SOURCE_DIR}/Visual/Instance.cpp
  
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Visual)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Shader/Debug)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Shader/Release)

option(MAKEGUI "build  as a GUI rather then console app" OFF)

if(MAKEGUI)
  set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
  target_compile_definitions(${PROJECT_NAME} PUBLIC GUI)
else()
  target_compile_definitions(${PROJECT_NAME} PUBLIC CONSOLE)
endif(MAKEGUI)

unset(MAKEGUI CACHE) # <---- this is important

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(${PROJECT_NAME} PRIVATE  
  /options:strict # unrecognized compiler options are an error
  /nologo #suppress copyright message    
  #/GA #optimize for Windows Application
  #/O1 # maximum optimizations (favor space)
  #/Os #  favor code space
  /favor:INTEL64 #  favor code space
  #/analyze #Enable native analysis      
  #/external:env:<var>      # location of external headers
  /external:anglebrackets # treat all headers included via <> as external
  /external:I "C://Program Files//Microsoft Visual Studio//2022//Community" #All recursive subdirectories of path are considered external
  #/Wall #enable all warnings
  /external:W0
  )
  endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  

target_link_libraries(${PROJECT_NAME} PRIVATE D3D11 DXGI dxguid)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)
