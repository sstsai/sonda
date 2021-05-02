#if defined(_WIN32)
#   define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__) || defined(__unix__)
#   define VK_USE_PLATFORM_XLIB_KHR
#elif defined(__APPLE__)
#   define VK_USE_PLATFORM_MACOS_MVK
#else
#   error "Platform not supported by this example."
#endif

#define VOLK_IMPLEMENTATION
#include <volk.h>
int main(int ac, char **av)
{
    auto result = volkInitialize();
    if (result != VK_SUCCESS)
        return -1;
    VkInstance instance{};
    volkLoadInstance(instance);
}
