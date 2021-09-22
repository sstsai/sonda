#pragma once
#include <vulkan/vulkan.hpp>
#include <fmt/format.h>
#include <vector>
#include <string>
struct logger {
private:
    std::string              buffer;
    std::vector<std::size_t> indexes;

public:
    void operator()(int code, char const *description)
    {
        indexes.push_back(buffer.size());
        fmt::format_to(std::back_inserter(buffer), "[glfw][Error] {}:{}\n",
                       code, description);
    }
    void operator()(vk::DebugUtilsMessageSeverityFlagBitsEXT    severity,
                    vk::DebugUtilsMessageTypeFlagsEXT           type,
                    VkDebugUtilsMessengerCallbackDataEXT const *data)
    {
        indexes.push_back(buffer.size());
        fmt::format_to(std::back_inserter(buffer), "[vulkan][{}] {}:{}\n",
                       to_string(severity), to_string(type), data->pMessage);
    }
};
