#include "gui.h"
#include "glfw_vulkan.h"
#include "logger.h"
struct test {
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    void render()
    {
        // 1. Show the big demo window (Most of the sample code is in
        // ImGui::ShowDemoWindow()! You can browse its code to learn more
        // about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a
        // Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello,
                                           // world!" and append into it.

            ImGui::Text(
                "This is some useful text."); // Display some text (you can
                                              // use a format strings too)
            ImGui::Checkbox("Demo Window",
                            &show_demo_window); // Edit bools storing our
                                                // window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat(
                "float", &f, 0.0f,
                1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3(
                "clear color",
                (float *)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button(
                    "Button")) // Buttons return true when clicked (most
                               // widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window) {
            ImGui::Begin(
                "Another Window",
                &show_another_window); // Pass a pointer to our bool
                                       // variable (the window will have a
                                       // closing button that will clear the
                                       // bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
    }
};

int main(int ac, char **av)
{
    logger log;
    auto glfw_cb = glfw::error_callback(log);
    {
        auto glfw_instance = glfw::instance();
        auto glfw_win = glfw::window(glfw_instance.make_window());
        glfw::vulkan::load(glfw_instance.instance_proc_address());
        auto e = glfw::vulkan::prefer(
            glfw::vulkan::extensions{
                glfw_instance.required_instance_extensions()},
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        auto l = glfw::vulkan::prefer(glfw::vulkan::layers(),
                                      glfw::vulkan::validation_layer);
        auto vk_instance =
            vk::createInstanceUnique(vk::InstanceCreateInfo()
                                         .setPEnabledLayerNames(l.value)
                                         .setPEnabledExtensionNames(e.value));
        volkLoadInstance(*vk_instance);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*vk_instance);
        auto vulkan_cb = glfw::vulkan::error_callback(*vk_instance, log);
    }
    auto app = gui::app();
    auto t = test();
    while (!app.should_close()) {
        auto scoped_frame = gui::frame(app, t.clear_color);
        t.render();
    }
}
