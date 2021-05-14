add_executable(sonda "")
target_sources(sonda
  PUBLIC
    "gui.h"
  PRIVATE
    "main.cpp"
    "gui.cpp"
)
target_link_libraries(sonda
  PRIVATE
)
target_compile_definitions(sonda
  PUBLIC
    _USE_MATH_DEFINES
)
target_compile_features(sonda PUBLIC cxx_std_20)
target_link_libraries(sonda PRIVATE imgui::vulkan imgui::glfw)

if (WIN32)
  target_compile_definitions(sonda
    PUBLIC
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
