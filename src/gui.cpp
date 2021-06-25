#define VOLK_IMPLEMENTATION
#include "gui.h"
#include "glfw_vulkan.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <stdio.h>
#include <stdlib.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

struct imgui_vulkan {
private:
    vk::UniqueInstance instance;
    vk::UniqueDebugReportCallbackEXT debug_cb;
    vk::UniqueDevice device;
    vk::UniqueDescriptorPool pool;

public:
    imgui_vulkan(GLFWwindow *window)
    {
        SetupVulkan();

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

    void SetupVulkan()
    {
        VkResult err;

        // Initialize vulkan
        constexpr auto debug = false;
        instance = vk_instance(debug);
        if (debug)
            debug_cb = vk_debug_report(*instance);
        g_Instance = *instance;

        // Select GPU
        g_PhysicalDevice = vk_physical_device(g_Instance);

        // Select graphics queue family
        g_QueueFamily = vk_queue_family(g_PhysicalDevice);

        // Create Logical Device (with 1 queue)
        device = vk_device(g_PhysicalDevice, g_QueueFamily);
        g_Queue = device->getQueue(g_QueueFamily, 0);
        g_Device = *device;

        // Create Descriptor Pool
        pool = vk_pool(g_Device);
        g_DescriptorPool = *pool;
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
    static auto vk_resize()
    {
        // Create SwapChain, RenderPass, Framebuffer, etc.
        // IM_ASSERT(g_MinImageCount >= 2);
        // ImGui_ImplVulkanH_CreateOrResizeWindow(
        //    g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily,
        //    g_Allocator, width, height, g_MinImageCount);
    }
    static auto vk_swap(vk::PhysicalDevice physical_device, vk::Device device,
                        vk::SurfaceKHR surface, int width, int height)
    {
        auto caps = physical_device.getSurfaceCapabilitiesKHR(surface);
        auto form = vk_select_surface_format(physical_device, surface);
        device.waitIdle();
        device.createSwapchainKHRUnique(vk::SwapchainCreateInfoKHR{
            {},
            surface,
            vk_min_image(vk_present_mode(physical_device, surface), caps),
            form.format,
            form.colorSpace,
            vk_extent(width, height, caps),
        });
    }
    static auto vk_extent(uint32_t width, uint32_t height,
                          vk::SurfaceCapabilitiesKHR surface_capabilities)
        -> vk::Extent2D
    {
        if (surface_capabilities.currentExtent.width == 0xFFFFFFFF &&
            surface_capabilities.currentExtent.height == 0xFFFFFFFF)
            return {width, height};
        return surface_capabilities.currentExtent;
    }
    static auto vk_min_image(vk::PresentModeKHR present_mode,
                             vk::SurfaceCapabilitiesKHR surface_capabilities)
        -> uint32_t
    {
        uint32_t min_images;
        switch (present_mode) {
        case vk::PresentModeKHR::eMailbox:
            min_images = 3;
            break;
        case vk::PresentModeKHR::eFifo:
        case vk::PresentModeKHR::eFifoRelaxed:
            min_images = 2;
            break;
        case vk::PresentModeKHR::eImmediate:
        default:
            min_images = 1;
            break;
        }
        if (surface_capabilities.maxImageCount >=
            surface_capabilities.minImageCount)
            return std::clamp(min_images, surface_capabilities.minImageCount,
                              surface_capabilities.maxImageCount);
        return std::clamp(min_images, surface_capabilities.minImageCount,
                          min_images);
    }
    static auto vk_present_mode(vk::PhysicalDevice physical_device,
                                vk::SurfaceKHR surface) -> vk::PresentModeKHR
    {
        constexpr std::array<vk::PresentModeKHR, 2> present_modes = {
            vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate};
        auto const &modes = physical_device.getSurfacePresentModesKHR(surface);
        for (auto const &mode : present_modes)
            if (std::find(modes.begin(), modes.end(), mode) != modes.end())
                return mode;
        return vk::PresentModeKHR::eFifo;
    }
    static auto vk_select_surface_format(vk::PhysicalDevice physical_device,
                                         vk::SurfaceKHR surface)
        -> vk::SurfaceFormatKHR
    {
        for (auto const &format :
             physical_device.getSurfaceFormatsKHR(surface)) {
            if (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                switch (format.format) {
                case vk::Format::eB8G8R8A8Unorm:
                case vk::Format::eR8G8B8A8Unorm:
                case vk::Format::eB8G8R8Unorm:
                case vk::Format::eR8G8B8Unorm:
                    return format;
                }
            }
        }
        return vk::SurfaceFormatKHR();
    }
    static auto vk_check_surface_support(vk::PhysicalDevice physical_device,
                                         uint32_t queue_family,
                                         vk::SurfaceKHR surface) -> bool
    {
        return physical_device.getSurfaceSupportKHR(queue_family, surface);
    }
    static auto vk_pool(vk::Device device) -> vk::UniqueDescriptorPool
    {
        std::array<vk::DescriptorPoolSize, 11> pool_sizes = {
            vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler,
                                   1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer,
                                   1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer,
                                   1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic,
                                   1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic,
                                   1000},
            vk::DescriptorPoolSize{vk::DescriptorType::eInputAttachment, 1000},
        };
        return device.createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo(
            {}, 1000 * pool_sizes.size(), pool_sizes));
    }
    static auto vk_device(vk::PhysicalDevice physical_device,
                          uint32_t queue_family) -> vk::UniqueDevice
    {
        std::array<char const *, 1> device_extensions = {"VK_KHR_swapchain"};
        std::array<const float, 1> queue_priority = {1.0f};
        auto create_queue =
            vk::DeviceQueueCreateInfo({}, queue_family, queue_priority);
        auto device = physical_device.createDeviceUnique(
            vk::DeviceCreateInfo({}, create_queue, {}, device_extensions));
        volkLoadDevice(*device);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
        return device;
    }
    static auto vk_queue_family(vk::PhysicalDevice device) -> uint32_t
    {
        auto queues = device.getQueueFamilyProperties();
        for (auto i = 0; i < queues.size(); i++) {
            if (queues[i].queueFlags & vk::QueueFlagBits::eGraphics)
                return i;
        }
        return (uint32_t)-1;
    }
    static auto vk_physical_device(vk::Instance instance) -> vk::PhysicalDevice
    {
        auto devices = instance.enumeratePhysicalDevices();
        // If a number >1 of GPUs got reported, find discrete GPU if
        // present, or use first one available. This covers most common
        // cases (multi-gpu/integrated+dedicated graphics). Handling more
        // complicated setups (multiple dedicated GPUs) is out of scope of
        // this sample.
        int use_gpu = 0;
        for (auto i = 0; i < devices.size(); i++) {
            if (devices[i].getProperties().deviceType ==
                vk::PhysicalDeviceType::eDiscreteGpu) {
                use_gpu = i;
                break;
            }
        }
        return devices[use_gpu];
    }
    static auto vk_instance(bool enable_debug = false) -> vk::UniqueInstance
    {
        volkInitializeCustom(
            (PFN_vkGetInstanceProcAddr)glfwGetInstanceProcAddress(
                NULL, "vkGetInstanceProcAddr"));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
        uint32_t count;
        auto extensions = glfwGetRequiredInstanceExtensions(&count);
        auto vec_extensions =
            std::vector<char const *>(extensions, extensions + count);
        auto vec_layers = std::vector<char const *>();
        auto validation_layer = "VK_LAYER_KHRONOS_validation";
        auto debug_report = "VK_EXT_debug_report";
        if (enable_debug) {
            vec_layers.push_back(validation_layer);
            vec_extensions.push_back(debug_report);
        }
        auto instance = vk::createInstanceUnique(
            vk::InstanceCreateInfo()
                .setPEnabledExtensionNames(vec_extensions)
                .setPEnabledLayerNames(vec_layers));
        volkLoadInstance(*instance);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
        return instance;
    }
    static auto vk_debug_report(vk::Instance instance)
        -> vk::UniqueDebugReportCallbackEXT
    {
        return instance.createDebugReportCallbackEXTUnique(
            {{}, (PFN_vkDebugReportCallbackEXT)debug_report, {}});
    }
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
};
namespace gui {
struct app::impl {
    glfw::instance glfw_instance = glfw::instance();
    glfw::window glfw_window =
        glfw_instance.make_window(glfw::client_api{glfw::api::none});
    imgui_vulkan vulkan = imgui_vulkan(glfw_window);
};
app::app() : impl_(std::make_unique<impl>()) {}
app::~app() = default;
bool app::should_close() const
{
    return impl_->glfw_window.should_close();
}

struct frame::impl {
    imgui_vulkan &vulkan;
    GLFWwindow *window;
    ImVec4 clear_color;
};
frame::frame(app const &a, ImVec4 clear_color)
    : impl_(std::make_unique<impl>(a.impl_->vulkan, a.impl_->glfw_window,
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
