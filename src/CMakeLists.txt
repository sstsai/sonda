add_executable(sonda "")
target_sources(sonda
  PRIVATE
    "main.cpp"
    "gui.cpp"
  PUBLIC
    "gui.h"
    "logger.h"
    "strong_type.h"
    "mqtt.h"
    "fmap.h"
    "endian.h"
    "bytes.h"
 )
target_link_libraries(sonda
  PRIVATE
)
target_compile_definitions(sonda
  PUBLIC
    _USE_MATH_DEFINES
)
target_compile_features(sonda PUBLIC cxx_std_20)
if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12.25835)
  set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
  set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()
target_link_libraries(sonda
  PRIVATE
    imgui::vulkan imgui::glfw asio::asio Boost::describe Boost::system fmt::fmt
)

if (WIN32)
  target_compile_definitions(sonda
    PUBLIC
      WIN32_LEAN_AND_MEAN
      _CRT_SECURE_NO_WARNINGS
      _WIN32_WINNT=0x0601
  )
  target_link_options(sonda
    PRIVATE
      "/ENTRY:mainCRTStartup"
      "/SUBSYSTEM:WINDOWS"
  )
endif()
if (UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(sonda PRIVATE Threads::Threads)
endif ()
