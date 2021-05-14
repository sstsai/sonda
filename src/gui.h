#pragma once
#include <imgui.h>
#include <memory>
namespace gui {
struct frame;
struct app {
    app();
    ~app();
    bool should_close() const;

private:
    friend frame;
    struct impl;
    std::unique_ptr<impl> impl_;
};
struct frame {
    frame(app const &a,
          ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f));
    ~frame();
    void clear_color(ImVec4 clear_color);

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};
} // namespace gui
