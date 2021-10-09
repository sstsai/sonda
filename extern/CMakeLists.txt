set(BUILD_SHARED_LIBS  OFF)
set(GLFW_BUILD_EXAMPLES  OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_VULKAN_STATIC OFF)
add_subdirectory(glfw)
add_subdirectory(vulkan-headers)
add_subdirectory(fmt)
add_subdirectory(boost)

# asio
add_library(asio INTERFACE)
add_library(asio::asio ALIAS asio)
target_include_directories(asio INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/asio/asio/include>
  $<INSTALL_INTERFACE:include>
)

# volk
add_library(volk_headers INTERFACE)
add_library(volk::volk_headers ALIAS volk_headers)
target_include_directories(volk_headers INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/volk>
  $<INSTALL_INTERFACE:include>
)
target_compile_definitions(volk_headers INTERFACE VK_NO_PROTOTYPES)
if (WIN32)
  target_compile_definitions(volk_headers INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
  target_compile_definitions(volk_headers INTERFACE VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
  target_compile_definitions(volk_headers INTERFACE VK_USE_PLATFORM_XLIB_KHR)
endif()
if (NOT WIN32)
  target_link_libraries(volk_headers INTERFACE dl)
endif()
target_link_libraries(volk_headers INTERFACE Vulkan::Headers)

# imgui
add_library(imgui)
add_library(imgui::imgui ALIAS imgui)
target_sources(imgui
  PUBLIC
    imgui/imgui.h
  PRIVATE
    imgui/imconfig.h
    imgui/imgui_internal.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>
  $<INSTALL_INTERFACE:include>
)

add_library(imgui_glfw)
add_library(imgui::glfw ALIAS imgui_glfw)
target_sources(imgui_glfw
  PUBLIC
    imgui/backends/imgui_impl_glfw.h
  PRIVATE
    imgui/backends/imgui_impl_glfw.cpp
)
target_include_directories(imgui_glfw PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(imgui_glfw PUBLIC imgui glfw)

add_library(imgui_vulkan)
add_library(imgui::vulkan ALIAS imgui_vulkan)
target_sources(imgui_vulkan
  PUBLIC
    imgui/backends/imgui_impl_vulkan.h
  PRIVATE
    imgui/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui_vulkan PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends>
  $<INSTALL_INTERFACE:include>
)
target_compile_definitions(imgui_vulkan
  PUBLIC
    IMGUI_IMPL_VULKAN_NO_PROTOTYPES
    GLFW_INCLUDE_VULKAN
    GLFW_INCLUDE_NONE
  PRIVATE
    IMGUI_USER_CONFIG=<volk.h>
)
target_link_libraries(imgui_vulkan PUBLIC imgui volk::volk_headers)

