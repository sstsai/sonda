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
target_compile_features(sonda PUBLIC cxx_std_23)
target_link_libraries(sonda PRIVATE imgui::vulkan imgui::glfw asio::asio Boost::describe)

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
