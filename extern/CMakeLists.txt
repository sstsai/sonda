add_subdirectory(vulkan-headers)
# volk
add_library(volk_headers INTERFACE)
add_library(volk::volk_headers ALIAS volk_headers)
target_include_directories(volk_headers INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/volk>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(volk_headers INTERFACE Vulkan::Headers)
if (NOT WIN32)
  target_link_libraries(volk_headers INTERFACE dl)
endif()
# volk end