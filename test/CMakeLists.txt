add_executable(sonda-test "")
target_sources(sonda-test
  PRIVATE
    "main.cpp"
)
target_include_directories(sonda-test PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(sonda-test
  PRIVATE
    imgui::vulkan imgui::glfw asio::asio Boost::describe Boost::system fmt::fmt
)
target_compile_definitions(sonda-test
  PUBLIC
    _USE_MATH_DEFINES
)
target_compile_features(sonda-test PUBLIC cxx_std_20)
if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12.25835)
  set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
  set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()
if (WIN32)
  target_compile_definitions(sonda-test
    PUBLIC
      _CRT_SECURE_NO_WARNINGS
      _WIN32_WINNT=0x0601
  )
endif ()
if (UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(sonda-test PRIVATE Threads::Threads)
endif ()
