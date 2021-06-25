#pragma once
#include <volk.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>
#include <concepts>
namespace glfw {
template <class From, class To>
concept constructible_to = requires(std::add_rvalue_reference_t<From> (&f)())
{
    static_cast<To>(f());
};
template <typename T> concept HintKey = requires(T a)
{
    {
        a.key
    }
    ->std::convertible_to<int>;
};
template <typename T> concept HintValueInt = requires(T a)
{
    {
        a.value
    }
    ->constructible_to<int>;
};
template <typename T> concept HintValueString = requires(T a)
{
    {
        a.value
    }
    ->std::convertible_to<char const *>;
};
template <typename T>
concept InitHint = HintKey<T> &&T::key > 0x00050000 && HintValueInt<T>;
template <typename T>
concept WindowHint = HintKey<T> &&T::key < 0x00050000 &&
                     (HintValueInt<T> || HintValueString<T>);
template <typename T>
concept ErrorHandler = std::invocable<T, int, char const *>;
namespace detail {
template <auto Key, typename T, auto Default> struct key_value {
    static constexpr auto key = Key;
    T value = Default;
};
template <auto Key> struct key_value_s {
    static constexpr auto key = Key;
    char const *value = "";
};
enum class create_window_params { width, height, title, monitor, share };
template <typename T> concept WindowParam = requires(T a)
{
    {
        a.key
    }
    ->std::convertible_to<create_window_params>;
};
template <typename T> concept WindowArg = WindowHint<T> || WindowParam<T>;
struct release_instance {
    using pointer = int;
    void operator()(pointer) const { glfwTerminate(); }
};
using instance = std::unique_ptr<void, release_instance>;
struct destroy_window {
    using pointer = GLFWwindow *;
    void operator()(pointer p) const { glfwDestroyWindow(p); }
};
using window = std::unique_ptr<void, destroy_window>;
struct params {
    int width = 640;
    int height = 480;
    char const *title = "";
    GLFWmonitor *monitor = nullptr;
    GLFWwindow *share = nullptr;
};
void setup_window(params &p, WindowArg auto &&arg)
{
    if constexpr (WindowParam<decltype(arg)>) {
        if constexpr (arg.key == create_window_params::width) {
            p.width = arg.value;
        } else if constexpr (arg.key == create_window_params::height) {
            p.height = arg.value;
        } else if constexpr (arg.key == create_window_params::title) {
            p.title = arg.value;
        } else if constexpr (arg.key == create_window_params::monitor) {
            p.monitor = arg.value;
        } else if constexpr (arg.key == create_window_params::share) {
            p.share = arg.value;
        }
    } else {
        if constexpr (HintValueInt<decltype(arg)>) {
            glfwWindowHint(arg.key, static_cast<int>(arg.value));
        } else if constexpr (HintValueString<decltype(arg)>) {
            glfwWindowHintString(arg.key, arg.value);
        }
    }
}
} // namespace detail
inline namespace init_hint {
using joystick_hat_buttons =
    detail::key_value<GLFW_JOYSTICK_HAT_BUTTONS, bool, true>;
using cocoa_chdir_resources =
    detail::key_value<GLFW_COCOA_CHDIR_RESOURCES, bool, true>;
using cocoa_menubar = detail::key_value<GLFW_COCOA_MENUBAR, bool, true>;
} // namespace init_hint
inline namespace window_hint {
using width = detail::key_value<detail::create_window_params::width, int, 640>;
using height =
    detail::key_value<detail::create_window_params::height, int, 480>;
using title = detail::key_value_s<detail::create_window_params::title>;
using monitor = detail::key_value<detail::create_window_params::monitor,
                                  GLFWmonitor *, nullptr>;
using share = detail::key_value<detail::create_window_params::share,
                                GLFWwindow *, nullptr>;
using resizable = detail::key_value<GLFW_RESIZABLE, bool, true>;
using visible = detail::key_value<GLFW_VISIBLE, bool, true>;
using decorated = detail::key_value<GLFW_DECORATED, bool, true>;
using focused = detail::key_value<GLFW_FOCUSED, bool, true>;
using auto_iconify = detail::key_value<GLFW_AUTO_ICONIFY, bool, true>;
using floating = detail::key_value<GLFW_FLOATING, bool, false>;
using maximized = detail::key_value<GLFW_MAXIMIZED, bool, false>;
using center_cursor = detail::key_value<GLFW_CENTER_CURSOR, bool, true>;
using transparent_framebuffer =
    detail::key_value<GLFW_TRANSPARENT_FRAMEBUFFER, bool, false>;
using focus_on_show = detail::key_value<GLFW_FOCUS_ON_SHOW, bool, true>;
using scale_to_monitor = detail::key_value<GLFW_SCALE_TO_MONITOR, bool, false>;
using red_bits = detail::key_value<GLFW_RED_BITS, int, 8>;
using green_bits = detail::key_value<GLFW_GREEN_BITS, int, 8>;
using blue_bits = detail::key_value<GLFW_BLUE_BITS, int, 8>;
using alpha_bits = detail::key_value<GLFW_ALPHA_BITS, int, 8>;
using depth_bits = detail::key_value<GLFW_DEPTH_BITS, int, 24>;
using stencil_bits = detail::key_value<GLFW_STENCIL_BITS, int, 8>;
using accum_red_bits = detail::key_value<GLFW_ACCUM_RED_BITS, int, 0>;
using accum_green_bits = detail::key_value<GLFW_ACCUM_GREEN_BITS, int, 0>;
using accum_blue_bits = detail::key_value<GLFW_ACCUM_BLUE_BITS, int, 0>;
using accum_alpha_bits = detail::key_value<GLFW_ACCUM_ALPHA_BITS, int, 0>;
using aux_buffers = detail::key_value<GLFW_AUX_BUFFERS, int, 0>;
using samples = detail::key_value<GLFW_SAMPLES, int, 0>;
using refresh_rate = detail::key_value<GLFW_REFRESH_RATE, int, GLFW_DONT_CARE>;
using stereo = detail::key_value<GLFW_STEREO, bool, false>;
using srgb_capable = detail::key_value<GLFW_SRGB_CAPABLE, bool, false>;
using doublebuffer = detail::key_value<GLFW_DOUBLEBUFFER, bool, true>;
enum class api {
    opengl = GLFW_OPENGL_API,
    opengl_es = GLFW_OPENGL_ES_API,
    none = GLFW_NO_API
};
using client_api = detail::key_value<GLFW_CLIENT_API, api, api::opengl>;
enum class context_api {
    native = GLFW_NATIVE_CONTEXT_API,
    egl = GLFW_EGL_CONTEXT_API,
    osmesa = GLFW_OSMESA_CONTEXT_API
};
using context_creation_api =
    detail::key_value<GLFW_CONTEXT_CREATION_API, context_api,
                      context_api::native>;
using context_version_major =
    detail::key_value<GLFW_CONTEXT_VERSION_MAJOR, int, 1>;
using context_version_minor =
    detail::key_value<GLFW_CONTEXT_VERSION_MINOR, int, 0>;
enum class robustness {
    none = GLFW_NO_ROBUSTNESS,
    no_reset_notification = GLFW_NO_RESET_NOTIFICATION,
    lose_context_on_reset = GLFW_LOSE_CONTEXT_ON_RESET,
    any_release_behavior = GLFW_ANY_RELEASE_BEHAVIOR
};
using context_robustness =
    detail::key_value<GLFW_CONTEXT_ROBUSTNESS, robustness, robustness::none>;
using opengl_forward_compat =
    detail::key_value<GLFW_OPENGL_FORWARD_COMPAT, bool, false>;
using opengl_debug_context =
    detail::key_value<GLFW_OPENGL_DEBUG_CONTEXT, bool, false>;
enum class profile {
    any = GLFW_OPENGL_ANY_PROFILE,
    compat = GLFW_OPENGL_COMPAT_PROFILE,
    core = GLFW_OPENGL_CORE_PROFILE
};
using opengl_profile =
    detail::key_value<GLFW_OPENGL_PROFILE, profile, profile::any>;
using cocoa_retina_framebufer =
    detail::key_value<GLFW_COCOA_RETINA_FRAMEBUFFER, bool, true>;
using cocoa_frame_name = detail::key_value_s<GLFW_COCOA_FRAME_NAME>;
using cocoa_graphics_switching =
    detail::key_value<GLFW_COCOA_GRAPHICS_SWITCHING, bool, false>;
using x11_class_name = detail::key_value_s<GLFW_X11_CLASS_NAME>;
using x11_instance_name = detail::key_value_s<GLFW_X11_INSTANCE_NAME>;
} // namespace window_hint
struct window {
private:
    detail::window win;

public:
    window(GLFWwindow *win) : win(win) {}
    bool should_close() const { return glfwWindowShouldClose(win.get()); }
    void should_close(bool close) const
    {
        glfwSetWindowShouldClose(win.get(), close ? GLFW_TRUE : GLFW_FALSE);
    }
    operator GLFWwindow *() const { return win.get(); }
};
inline auto version()
{
    struct {
        int major;
        int minor;
        int revision;
    } version;
    glfwGetVersion(&version.major, &version.minor, &version.revision);
    return version;
}
inline auto version_string() { return glfwGetVersionString(); }
inline auto error()
{
    struct {
        int code;
        char const *description;
    } error;
    error.code = glfwGetError(&error.description);
    return error;
}
template <typename Fn> struct error_callback {
private:
    inline static error_callback<Fn> *self;
    Fn fn;

public:
    template <ErrorHandler Handler>
    error_callback(Handler &&fn) : fn(std::forward<Handler>(fn))
    {
        self = this;
        glfwSetErrorCallback(error_handler);
    }
    ~error_callback() { glfwSetErrorCallback(nullptr); }

private:
    static void error_handler(int code, char const *description)
    {
        self->fn(code, description);
    }
};
template <typename Fn> error_callback(Fn &fn) -> error_callback<Fn &>;
template <typename Fn> error_callback(Fn &&fn) -> error_callback<Fn>;
struct instance {
private:
    detail::instance sys;

public:
    instance(InitHint auto &&...hints)
    {
        (..., glfwInitHint(hints.key, static_cast<int>(hints.value)));
        sys.reset(glfwInit());
    }
    auto make_window(detail::WindowArg auto &&...args) -> GLFWwindow *
    {
        detail::params p;
        glfwDefaultWindowHints();
        (..., detail::setup_window(p, std::forward<decltype(args)>(args)));
        return glfwCreateWindow(p.width, p.height, p.title, p.monitor, p.share);
    }
    auto required_instance_extensions() -> std::vector<char const *>
    {
        uint32_t count;
        auto extensions = glfwGetRequiredInstanceExtensions(&count);
        return std::vector<char const *>(extensions, extensions + count);
    }
    auto instance_proc_address() -> PFN_vkGetInstanceProcAddr
    {
        return (PFN_vkGetInstanceProcAddr)glfwGetInstanceProcAddress(
            NULL, "vkGetInstanceProcAddr");
    }
};
namespace vulkan {
template <typename T>
concept ErrorHandler =
    std::invocable<T, vk::DebugUtilsMessageSeverityFlagBitsEXT,
                   vk::DebugUtilsMessageTypeFlagsEXT,
                   VkDebugUtilsMessengerCallbackDataEXT const *>;
template <typename Fn> struct error_callback {
private:
    Fn fn;
    vk::UniqueDebugUtilsMessengerEXT debug;

public:
    template <ErrorHandler Handler>
    error_callback(vk::Instance instance, Handler &&fn)
        : fn(std::forward<Handler>(fn)),
          debug(instance.createDebugUtilsMessengerEXTUnique(
              {{},
               {vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose},
               {vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance},
               &callback,
               this}))
    {}

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
             VkDebugUtilsMessageTypeFlagsEXT type,
             VkDebugUtilsMessengerCallbackDataEXT const *data, void *user)
    {
        static_cast<error_callback<Fn> *>(user)->fn(
            static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity),
            static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(type), data);
        return VK_FALSE;
    }
};
template <typename Fn>
error_callback(vk::Instance, Fn &) -> error_callback<Fn &>;
template <typename Fn>
error_callback(vk::Instance, Fn &&) -> error_callback<Fn>;
inline constexpr struct prefer_fn {
    template <typename T, typename... Args>
    constexpr auto operator()(T &&obj, Args &&...args) const
    {
        return tag_invoke(prefer_fn{}, std::forward<T>(obj),
                          std::forward<Args>(args)...);
    }
} prefer;
inline constexpr struct require_fn {
    template <typename T, typename... Args>
    constexpr auto operator()(T &&obj, Args &&...args) const
    {
        return tag_invoke(require_fn{}, std::forward<T>(obj),
                          std::forward<Args>(args)...);
    }
} require;
inline constexpr auto validation_layer = "VK_LAYER_LUNARG_standard_validation";
struct extensions {
    std::vector<char const *> value;

private:
    template <typename T, typename... Args>
        requires std::same_as<T, extensions> &&
        (... && std::convertible_to<Args, char const *>)friend auto tag_invoke(
            prefer_fn, T &&e, Args &&...args)
    {
        auto match = [properties = vk::enumerateInstanceExtensionProperties()](
                         auto name) {
            for (auto const &prop : properties) {
                if (std::string_view(prop.extensionName) == name) {
                    return true;
                }
            }
            return false;
        };
        auto push_back_if = [&](auto name) {
            if (match(name))
                e.value.push_back(name);
        };
        (..., push_back_if(args));
        return e;
    }
    template <typename T, typename... Args>
        requires std::same_as<T, extensions> &&
        (... && std::convertible_to<Args, char const *>)friend auto tag_invoke(
            require_fn, T &&e, Args &&...args)
    {
        auto push_back = [&](auto name) { e.value.push_back(name); };
        (..., push_back(args));
        return e;
    }
};
struct layers {
    std::vector<char const *> value;

private:
    template <typename T, typename... Args>
        requires std::same_as<T, layers> &&
        (... && std::convertible_to<Args, char const *>)friend auto tag_invoke(
            prefer_fn, T &&l, Args &&...args)
    {
        auto match = [properties =
                          vk::enumerateInstanceLayerProperties()](auto name) {
            for (auto const &prop : properties) {
                if (std::string_view(prop.layerName) == name) {
                    return true;
                }
            }
            return false;
        };
        auto push_back_if = [&](auto name) {
            if (match(name))
                l.value.push_back(name);
        };
        (..., push_back_if(args));
        return l;
    }
    template <typename T, typename... Args>
        requires std::same_as<T, layers> &&
        (... && std::convertible_to<Args, char const *>)friend auto tag_invoke(
            require_fn, T &&l, Args &&...args)
    {
        auto push_back = [&](auto name) { l.value.push_back(name); };
        (..., push_back(args));
        return l;
    }
    friend auto tag_invoke(prefer_fn, layers &e, char const *name) -> layers &
    {
        auto properties = vk::enumerateInstanceLayerProperties();
        for (auto const &prop : properties) {
            if (std::string_view(prop.layerName) == name) {
                e.value.push_back(name);
                break;
            }
        }
        return e;
    }
    friend auto tag_invoke(require_fn, layers &e, char const *name) -> layers &
    {
        e.value.push_back(name);
        return e;
    }
};
inline void load(PFN_vkGetInstanceProcAddr addr = nullptr)
{
    if (addr) {
        volkInitializeCustom(addr);
    } else {
        volkInitialize();
    }
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}
inline auto make_instance(vk::InstanceCreateInfo const &info)
    -> vk::UniqueInstance
{
    auto instance = vk::createInstanceUnique(info);
    volkLoadInstance(*instance);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
    return instance;
}
} // namespace vulkan
} // namespace glfw
struct app {
private:
    glfw::instance gi = glfw::instance();
    glfw::window gw =
        glfw::window(gi.make_window(glfw::client_api{glfw::api::none}));
    // vk::UniqueInstance vi = glfw::vulkan::make_instance(
    //    gi.instance_proc_address(),
    //    {}.setPEnabledLayerNames(glfw::vulkan::debug_layers()));
public:
};
