add_executable(sonda-test "")
target_sources(sonda-test
  PRIVATE
    "main.cpp"
)
target_link_libraries(sonda-test
  PRIVATE
)
target_compile_definitions(sonda-test
  PUBLIC
    _USE_MATH_DEFINES
)
target_compile_features(sonda-test PUBLIC cxx_std_20)

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
