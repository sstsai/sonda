#include "gui.h"
#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__) || defined(__unix__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
#else
#error "Platform not supported by this example."
#endif

#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <stdio.h>
#include <stdlib.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

struct imgui_vulkan {
public:
    imgui_vulkan(GLFWwindow *window)
    {
        uint32_t extensions_count = 0;
        const char **extensions =
            glfwGetRequiredInstanceExtensions(&extensions_count);
        SetupVulkan(extensions, extensions_count);

        // Create Window Surface
        VkSurfaceKHR surface;
        VkResult err =
            glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface);
        check_vk_result(err);

        // Create Framebuffers
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        ImGui_ImplVulkanH_Window *wd = &g_MainWindowData;
        SetupVulkanWindow(wd, surface, w, h);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable
        // Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |=
            ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
                                              // Platform Windows
        // io.ConfigViewportsNoAutoMerge = true;
        // io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so
        // platform windows can look identical to regular ones.
        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = g_Instance;
        init_info.PhysicalDevice = g_PhysicalDevice;
        init_info.Device = g_Device;
        init_info.QueueFamily = g_QueueFamily;
        init_info.Queue = g_Queue;
        init_info.PipelineCache = g_PipelineCache;
        init_info.DescriptorPool = g_DescriptorPool;
        init_info.Allocator = g_Allocator;
        init_info.MinImageCount = g_MinImageCount;
        init_info.ImageCount = wd->ImageCount;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You
        // can also load multiple fonts and use ImGui::PushFont()/PopFont() to
        // select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if
        // you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please
        // handle those errors in your application (e.g. use an assertion, or
        // display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and
        // stored into a texture when calling
        // ImFontAtlas::Build()/GetTexDataAsXXXX(), which
        // ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a
        // string literal you need to write a double backslash \\ !
        // io.Fonts->AddFontDefault();
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        // ImFont* font =
        // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
        // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

        // Upload Fonts
        {
            // Use any command queue
            VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
            VkCommandBuffer command_buffer =
                wd->Frames[wd->FrameIndex].CommandBuffer;

            err = vkResetCommandPool(g_Device, command_pool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(command_buffer, &begin_info);
            check_vk_result(err);

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            err = vkEndCommandBuffer(command_buffer);
            check_vk_result(err);
            err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
            check_vk_result(err);

            err = vkDeviceWaitIdle(g_Device);
            check_vk_result(err);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }
    ~imgui_vulkan()
    {
        auto err = vkDeviceWaitIdle(g_Device);
        check_vk_result(err);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        CleanupVulkanWindow();
        CleanupVulkan();
    }

private:
    friend gui::frame;
    VkAllocationCallbacks *g_Allocator = NULL;
    VkInstance g_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice g_Device = VK_NULL_HANDLE;
    uint32_t g_QueueFamily = (uint32_t)-1;
    VkQueue g_Queue = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
    VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
    VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

    ImGui_ImplVulkanH_Window g_MainWindowData;
    int g_MinImageCount = 2;
    bool g_SwapChainRebuild = false;

    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(
        VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
        uint64_t object, size_t location, int32_t messageCode,
        const char *pLayerPrefix, const char *pMessage, void *pUserData)
    {
        (void)flags;
        (void)object;
        (void)location;
        (void)messageCode;
        (void)pUserData;
        (void)pLayerPrefix; // Unused arguments
        fprintf(stderr,
                "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n",
                objectType, pMessage);
        return VK_FALSE;
    }
#endif // IMGUI_VULKAN_DEBUG_REPORT

    void SetupVulkan(const char **extensions, uint32_t extensions_count)
    {
        VkResult err;

        // Create Vulkan Instance
        {
            VkInstanceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.enabledExtensionCount = extensions_count;
            create_info.ppEnabledExtensionNames = extensions;
#ifdef IMGUI_VULKAN_DEBUG_REPORT
            // Enabling validation layers
            const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
            create_info.enabledLayerCount = 1;
            create_info.ppEnabledLayerNames = layers;

            // Enable debug report extension (we need additional storage, so we
            // duplicate the user array to add our new extension to it)
            const char **extensions_ext = (const char **)malloc(
                sizeof(const char *) * (extensions_count + 1));
            memcpy(extensions_ext, extensions,
                   extensions_count * sizeof(const char *));
            extensions_ext[extensions_count] = "VK_EXT_debug_report";
            create_info.enabledExtensionCount = extensions_count + 1;
            create_info.ppEnabledExtensionNames = extensions_ext;

            // Create Vulkan Instance
            err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
            check_vk_result(err);
            free(extensions_ext);

            // Get the function pointer (required for any extensions)
            auto vkCreateDebugReportCallbackEXT =
                (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
                    g_Instance, "vkCreateDebugReportCallbackEXT");
            IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

            // Setup the debug report callback
            VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
            debug_report_ci.sType =
                VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                    VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            debug_report_ci.pfnCallback = debug_report;
            debug_report_ci.pUserData = NULL;
            err = vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci,
                                                 g_Allocator, &g_DebugReport);
            check_vk_result(err);
#else
            // Create Vulkan Instance without any debug feature
            err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
            check_vk_result(err);
            IM_UNUSED(g_DebugReport);
#endif
        }

        volkLoadInstance(g_Instance);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(g_Instance);

        // Select GPU
        {
            uint32_t gpu_count;
            err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, NULL);
            check_vk_result(err);
            IM_ASSERT(gpu_count > 0);

            VkPhysicalDevice *gpus = (VkPhysicalDevice *)malloc(
                sizeof(VkPhysicalDevice) * gpu_count);
            err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus);
            check_vk_result(err);

            // If a number >1 of GPUs got reported, find discrete GPU if
            // present, or use first one available. This covers most common
            // cases (multi-gpu/integrated+dedicated graphics). Handling more
            // complicated setups (multiple dedicated GPUs) is out of scope of
            // this sample.
            int use_gpu = 0;
            for (int i = 0; i < (int)gpu_count; i++) {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(gpus[i], &properties);
                if (properties.deviceType ==
                    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    use_gpu = i;
                    break;
                }
            }

            g_PhysicalDevice = gpus[use_gpu];
            free(gpus);
        }

        // Select graphics queue family
        {
            uint32_t count;
            vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count,
                                                     NULL);
            VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(
                sizeof(VkQueueFamilyProperties) * count);
            vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count,
                                                     queues);
            for (uint32_t i = 0; i < count; i++)
                if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    g_QueueFamily = i;
                    break;
                }
            free(queues);
            IM_ASSERT(g_QueueFamily != (uint32_t)-1);
        }

        // Create Logical Device (with 1 queue)
        {
            int device_extension_count = 1;
            const char *device_extensions[] = {"VK_KHR_swapchain"};
            const float queue_priority[] = {1.0f};
            VkDeviceQueueCreateInfo queue_info[1] = {};
            queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[0].queueFamilyIndex = g_QueueFamily;
            queue_info[0].queueCount = 1;
            queue_info[0].pQueuePriorities = queue_priority;
            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount =
                sizeof(queue_info) / sizeof(queue_info[0]);
            create_info.pQueueCreateInfos = queue_info;
            create_info.enabledExtensionCount = device_extension_count;
            create_info.ppEnabledExtensionNames = device_extensions;
            err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator,
                                 &g_Device);
            check_vk_result(err);
            volkLoadDevice(g_Device);
            VULKAN_HPP_DEFAULT_DISPATCHER.init(g_Device);
            vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
        }

        // Create Descriptor Pool
        {
            VkDescriptorPoolSize pool_sizes[] = {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator,
                                         &g_DescriptorPool);
            check_vk_result(err);
        }
    }

    // All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers
    // used by the demo.
    // Your real engine/app may not use them.
    void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface,
                           int width, int height)
    {
        wd->Surface = surface;

        // Check for WSI support
        VkBool32 res;
        vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily,
                                             wd->Surface, &res);
        if (res != VK_TRUE) {
            fprintf(stderr, "Error no WSI support on physical device 0\n");
            exit(-1);
        }

        // Select Surface Format
        const VkFormat requestSurfaceImageFormat[] = {
            VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
            VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
        const VkColorSpaceKHR requestSurfaceColorSpace =
            VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
            g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat,
            (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
            requestSurfaceColorSpace);

        // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
        VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR,
                                            VK_PRESENT_MODE_IMMEDIATE_KHR,
                                            VK_PRESENT_MODE_FIFO_KHR};
#else
        VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
        wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
            g_PhysicalDevice, wd->Surface, &present_modes[0],
            IM_ARRAYSIZE(present_modes));
        // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

        // Create SwapChain, RenderPass, Framebuffer, etc.
        IM_ASSERT(g_MinImageCount >= 2);
        ImGui_ImplVulkanH_CreateOrResizeWindow(
            g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily,
            g_Allocator, width, height, g_MinImageCount);
    }

    void CleanupVulkan()
    {
        vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
        // Remove the debug report callback
        auto vkDestroyDebugReportCallbackEXT =
            (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
                g_Instance, "vkDestroyDebugReportCallbackEXT");
        vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

        vkDestroyDevice(g_Device, g_Allocator);
        vkDestroyInstance(g_Instance, g_Allocator);
    }

    void CleanupVulkanWindow()
    {
        ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData,
                                        g_Allocator);
    }

    void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data)
    {
        VkResult err;

        VkSemaphore image_acquired_semaphore =
            wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore =
            wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX,
                                    image_acquired_semaphore, VK_NULL_HANDLE,
                                    &wd->FrameIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
            g_SwapChainRebuild = true;
            return;
        }
        check_vk_result(err);

        ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
        {
            err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE,
                                  UINT64_MAX); // wait indefinitely instead of
                                               // periodically checking
            check_vk_result(err);

            err = vkResetFences(g_Device, 1, &fd->Fence);
            check_vk_result(err);
        }
        {
            err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
            check_vk_result(err);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = wd->RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = wd->Width;
            info.renderArea.extent.height = wd->Height;
            info.clearValueCount = 1;
            info.pClearValues = &wd->ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info,
                                 VK_SUBPASS_CONTENTS_INLINE);
        }

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkPipelineStageFlags wait_stage =
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;

            err = vkEndCommandBuffer(fd->CommandBuffer);
            check_vk_result(err);
            err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
            check_vk_result(err);
        }
    }

    void FramePresent(ImGui_ImplVulkanH_Window *wd)
    {
        if (g_SwapChainRebuild)
            return;
        VkSemaphore render_complete_semaphore =
            wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &wd->Swapchain;
        info.pImageIndices = &wd->FrameIndex;
        VkResult err = vkQueuePresentKHR(g_Queue, &info);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
            g_SwapChainRebuild = true;
            return;
        }
        check_vk_result(err);
        wd->SemaphoreIndex =
            (wd->SemaphoreIndex + 1) %
            wd->ImageCount; // Now we can use the next set of semaphores
    }
};
struct imgui_glfw {
private:
    GLFWwindow *window;

public:
    imgui_glfw()
    {
        if (volkInitialize() != VK_SUCCESS)
            throw std::runtime_error("volkInitialize failure");
        // Setup GLFW window
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            throw std::runtime_error("glfwInit failure");

        // ImGui_ImplVulkan_LoadFunctions([](char const *function_name, void *)
        // {
        //    return glfwGetInstanceProcAddress(nullptr, function_name);
        //});
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+Vulkan example",
                                  NULL, NULL);

        // Setup Vulkan
        if (!glfwVulkanSupported()) {
            printf("GLFW: Vulkan Not Supported\n");
            abort();
        }
    }
    ~imgui_glfw()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    GLFWwindow *win() const { return window; }

private:
    static void glfw_error_callback(int error, const char *description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }
};

namespace gui {
struct app::impl {
    imgui_glfw glfw = imgui_glfw();
    imgui_vulkan vulkan = imgui_vulkan(glfw.win());
};
app::app() : impl_(std::make_unique<impl>()) {}
app::~app() = default;
bool app::should_close() const
{
    return glfwWindowShouldClose(impl_->glfw.win());
}

struct frame::impl {
    imgui_vulkan &vulkan;
    GLFWwindow *window;
    ImVec4 clear_color;
};
frame::frame(app const &a, ImVec4 clear_color)
    : impl_(std::make_unique<impl>(a.impl_->vulkan, a.impl_->glfw.win(),
                                   clear_color))
{
    glfwPollEvents();

    auto &vulkan = impl_->vulkan;
    auto *window = impl_->window;

    // Resize swap chain?
    if (vulkan.g_SwapChainRebuild) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (width > 0 && height > 0) {
            ImGui_ImplVulkan_SetMinImageCount(vulkan.g_MinImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(
                vulkan.g_Instance, vulkan.g_PhysicalDevice, vulkan.g_Device,
                &vulkan.g_MainWindowData, vulkan.g_QueueFamily,
                vulkan.g_Allocator, width, height, vulkan.g_MinImageCount);
            vulkan.g_MainWindowData.FrameIndex = 0;
            vulkan.g_SwapChainRebuild = false;
        }
    }

    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
frame::~frame()
{
    auto &vulkan = impl_->vulkan;
    auto &clear_color = impl_->clear_color;

    ImGui_ImplVulkanH_Window *wd = &vulkan.g_MainWindowData;
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Rendering
    ImGui::Render();
    ImDrawData *main_draw_data = ImGui::GetDrawData();
    const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f ||
                                    main_draw_data->DisplaySize.y <= 0.0f);
    wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
    wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
    wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
    wd->ClearValue.color.float32[3] = clear_color.w;
    if (!main_is_minimized)
        vulkan.FrameRender(wd, main_draw_data);

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    // Present Main Platform Window
    if (!main_is_minimized)
        vulkan.FramePresent(wd);
}
void frame::clear_color(ImVec4 clear_color)
{
    impl_->clear_color = clear_color;
}
} // namespace gui
