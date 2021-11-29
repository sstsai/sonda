#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <boost/mp11.hpp>
#include <algorithm>
#include <tuple>
namespace widget {
struct vec2 {
    float x;
    float y;
    constexpr vec2() : vec2(0.0f, 0.0f) {}
    constexpr vec2(float x) : vec2(x, 0.0f) {}
    constexpr vec2(float x, float y) : x(x), y(y) {}
    operator ImVec2() const { return ImVec2(x, y); }

private:
    friend constexpr auto operator+(vec2 const &a, vec2 const &b) -> vec2
    {
        return vec2{a.x + b.x, a.y + b.y};
    }
};
inline auto to_vec(ImVec2 v) -> vec2 { return vec2{v.x, v.y}; }
struct bounds {
    vec2 ul;
    vec2 lr;
    constexpr auto width() const { return lr.x - ul.x; }
    constexpr auto height() const { return lr.y - ul.y; }
};
inline auto window_bounds() -> bounds
{
    auto pos = to_vec(ImGui::GetCursorScreenPos());
    return {pos, pos + to_vec(ImGui::GetContentRegionAvail())};
}
struct spacer {
    vec2 size_;
    constexpr spacer() : size_() {}
    constexpr spacer(float x) : size_(x) {}
    constexpr spacer(float x, float y) : size_(x, y) {}
    constexpr void render(bounds const &r) const {}
    constexpr auto size() const -> vec2 { return size_; }
};
struct label {
    char const *text;
    ImU32 color                      = IM_COL32_WHITE;
    ImFont *font                     = nullptr;
    float font_size                  = 0.0f;
    float wrap_width                 = 0.0f;
    const ImVec4 *cpu_fine_clip_rect = nullptr;
    void render(bounds const &r) const
    {
        ImGui::GetWindowDrawList()->AddText(font, font_size, r.ul, color, text,
                                            nullptr, wrap_width,
                                            cpu_fine_clip_rect);
    }
    auto size() const -> vec2 { return to_vec(ImGui::CalcTextSize(text)); }
};
struct rect {
    ImU32 color;
    float rounding    = 0.0f;
    ImDrawFlags flags = ImDrawFlags_None;
    float thickness   = 1.0f;
    void render(bounds const &r) const
    {
        ImGui::GetWindowDrawList()->AddRect(r.ul, r.lr, color, rounding, flags,
                                            thickness);
    }
    constexpr auto size() const -> vec2 { return {}; }
};
struct filled_rect {
    ImU32 color;
    float rounding    = 0.0f;
    ImDrawFlags flags = ImDrawFlags_None;
    auto calc(bounds const &r) const -> bounds { return r; }
    void render(bounds const &r) const
    {
        ImGui::GetWindowDrawList()->AddRectFilled(r.ul, r.lr, color, rounding,
                                                  flags);
    }
    constexpr auto size() const -> vec2 { return {}; }
};
struct circle {
    ImU32 color;
    int num_segments = 0;
    float thickness  = 1.0f;
    auto calc(bounds const &r) const -> bounds { return r; }
    void render(bounds const &r) const
    {
        auto center = (r.lr + r.ul) / 2;
        auto radius = std::min(r.lr.x - r.ul.x, r.lr.y - r.ul.y) / 2;
        ImGui::GetWindowDrawList()->AddCircle(center, radius, color,
                                              num_segments, thickness);
    }
    constexpr auto size() const -> vec2 { return {}; }
};
struct circle_filled {
    ImU32 color;
    int num_segments = 0;
    auto calc(bounds const &r) const -> bounds { return r; }
    void render(bounds const &r) const
    {
        auto center = (r.lr + r.ul) / 2;
        auto radius = std::min(r.lr.x - r.ul.x, r.lr.y - r.ul.y) / 2;
        ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, color,
                                                    num_segments);
    }
    constexpr auto size() const -> vec2 { return {}; }
};
namespace colors {
constexpr ImU32 alice_blue             = IM_COL32(240, 248, 255, 255);
constexpr ImU32 antique_white          = IM_COL32(250, 235, 215, 255);
constexpr ImU32 aquamarine             = IM_COL32(50, 191, 193, 255);
constexpr ImU32 azure                  = IM_COL32(240, 255, 255, 255);
constexpr ImU32 beige                  = IM_COL32(245, 245, 220, 255);
constexpr ImU32 bisque                 = IM_COL32(255, 228, 196, 255);
constexpr ImU32 black                  = IM_COL32(0, 0, 0, 255);
constexpr ImU32 blanched_almond        = IM_COL32(255, 235, 205, 255);
constexpr ImU32 blue                   = IM_COL32(0, 0, 255, 255);
constexpr ImU32 blue_violet            = IM_COL32(138, 43, 226, 255);
constexpr ImU32 brown                  = IM_COL32(165, 42, 42, 255);
constexpr ImU32 burly_wood             = IM_COL32(222, 184, 135, 255);
constexpr ImU32 cadet_blue             = IM_COL32(95, 146, 158, 255);
constexpr ImU32 kchartreuse            = IM_COL32(127, 255, 0, 255);
constexpr ImU32 chocolate              = IM_COL32(210, 105, 30, 255);
constexpr ImU32 coral                  = IM_COL32(255, 114, 86, 255);
constexpr ImU32 cornflower_blue        = IM_COL32(34, 34, 152, 255);
constexpr ImU32 corn_silk              = IM_COL32(255, 248, 220, 255);
constexpr ImU32 cyan                   = IM_COL32(0, 255, 255, 255);
constexpr ImU32 dark_goldenrod         = IM_COL32(184, 134, 11, 255);
constexpr ImU32 dark_green             = IM_COL32(0, 86, 45, 255);
constexpr ImU32 dark_khaki             = IM_COL32(189, 183, 107, 255);
constexpr ImU32 dark_olive_green       = IM_COL32(85, 86, 47, 255);
constexpr ImU32 dark_orange            = IM_COL32(255, 140, 0, 255);
constexpr ImU32 dark_orchid            = IM_COL32(139, 32, 139, 255);
constexpr ImU32 dark_salmon            = IM_COL32(233, 150, 122, 255);
constexpr ImU32 dark_sea_green         = IM_COL32(143, 188, 143, 255);
constexpr ImU32 dark_slate_blue        = IM_COL32(56, 75, 102, 255);
constexpr ImU32 dark_slate_gray        = IM_COL32(47, 79, 79, 255);
constexpr ImU32 dark_turquoise         = IM_COL32(0, 166, 166, 255);
constexpr ImU32 dark_violet            = IM_COL32(148, 0, 211, 255);
constexpr ImU32 deep_pink              = IM_COL32(255, 20, 147, 255);
constexpr ImU32 deep_sky_blue          = IM_COL32(0, 191, 255, 255);
constexpr ImU32 dim_gray               = IM_COL32(84, 84, 84, 255);
constexpr ImU32 dodger_blue            = IM_COL32(30, 144, 255, 255);
constexpr ImU32 firebrick              = IM_COL32(142, 35, 35, 255);
constexpr ImU32 floral_white           = IM_COL32(255, 250, 240, 255);
constexpr ImU32 forest_green           = IM_COL32(80, 159, 105, 255);
constexpr ImU32 gains_boro             = IM_COL32(220, 220, 220, 255);
constexpr ImU32 ghost_white            = IM_COL32(248, 248, 255, 255);
constexpr ImU32 gold                   = IM_COL32(218, 170, 0, 255);
constexpr ImU32 goldenrod              = IM_COL32(239, 223, 132, 255);
constexpr ImU32 green                  = IM_COL32(0, 255, 0, 255);
constexpr ImU32 green_yellow           = IM_COL32(173, 255, 47, 255);
constexpr ImU32 honeydew               = IM_COL32(240, 255, 240, 255);
constexpr ImU32 hot_pink               = IM_COL32(255, 105, 180, 255);
constexpr ImU32 indian_red             = IM_COL32(107, 57, 57, 255);
constexpr ImU32 ivory                  = IM_COL32(255, 255, 240, 255);
constexpr ImU32 khaki                  = IM_COL32(179, 179, 126, 255);
constexpr ImU32 lavender               = IM_COL32(230, 230, 250, 255);
constexpr ImU32 lavender_blush         = IM_COL32(255, 240, 245, 255);
constexpr ImU32 lawn_green             = IM_COL32(124, 252, 0, 255);
constexpr ImU32 lemon_chiffon          = IM_COL32(255, 250, 205, 255);
constexpr ImU32 light_blue             = IM_COL32(176, 226, 255, 255);
constexpr ImU32 light_coral            = IM_COL32(240, 128, 128, 255);
constexpr ImU32 light_cyan             = IM_COL32(224, 255, 255, 255);
constexpr ImU32 light_goldenrod        = IM_COL32(238, 221, 130, 255);
constexpr ImU32 light_goldenrod_yellow = IM_COL32(250, 250, 210, 255);
constexpr ImU32 light_gray             = IM_COL32(168, 168, 168, 255);
constexpr ImU32 light_pink             = IM_COL32(255, 182, 193, 255);
constexpr ImU32 light_salmon           = IM_COL32(255, 160, 122, 255);
constexpr ImU32 light_sea_green        = IM_COL32(32, 178, 170, 255);
constexpr ImU32 light_sky_blue         = IM_COL32(135, 206, 250, 255);
constexpr ImU32 light_slate_blue       = IM_COL32(132, 112, 255, 255);
constexpr ImU32 light_slate_gray       = IM_COL32(119, 136, 153, 255);
constexpr ImU32 light_steel_blue       = IM_COL32(124, 152, 211, 255);
constexpr ImU32 light_yellow           = IM_COL32(255, 255, 224, 255);
constexpr ImU32 lime_green             = IM_COL32(0, 175, 20, 255);
constexpr ImU32 linen                  = IM_COL32(250, 240, 230, 255);
constexpr ImU32 magenta                = IM_COL32(255, 0, 255, 255);
constexpr ImU32 maroon                 = IM_COL32(143, 0, 82, 255);
constexpr ImU32 medium_aquamarine      = IM_COL32(0, 147, 143, 255);
constexpr ImU32 medium_blue            = IM_COL32(50, 50, 204, 255);
constexpr ImU32 medium_forest_green    = IM_COL32(50, 129, 75, 255);
constexpr ImU32 medium_goldenrod       = IM_COL32(209, 193, 102, 255);
constexpr ImU32 medium_orchid          = IM_COL32(189, 82, 189, 255);
constexpr ImU32 medium_purple          = IM_COL32(147, 112, 219, 255);
constexpr ImU32 medium_sea_green       = IM_COL32(52, 119, 102, 255);
constexpr ImU32 medium_slate_blue      = IM_COL32(106, 106, 141, 255);
constexpr ImU32 medium_spring_green    = IM_COL32(35, 142, 35, 255);
constexpr ImU32 medium_turquoise       = IM_COL32(0, 210, 210, 255);
constexpr ImU32 medium_violet_red      = IM_COL32(213, 32, 121, 255);
constexpr ImU32 midnight_blue          = IM_COL32(47, 47, 100, 255);
constexpr ImU32 mint_cream             = IM_COL32(245, 255, 250, 255);
constexpr ImU32 misty_rose             = IM_COL32(255, 228, 225, 255);
constexpr ImU32 moccasin               = IM_COL32(255, 228, 181, 255);
constexpr ImU32 navajo_white           = IM_COL32(255, 222, 173, 255);
constexpr ImU32 navy                   = IM_COL32(35, 35, 117, 255);
constexpr ImU32 navy_blue              = IM_COL32(35, 35, 117, 255);
constexpr ImU32 old_lace               = IM_COL32(253, 245, 230, 255);
constexpr ImU32 olive_drab             = IM_COL32(107, 142, 35, 255);
constexpr ImU32 orange                 = IM_COL32(255, 135, 0, 255);
constexpr ImU32 orange_red             = IM_COL32(255, 69, 0, 255);
constexpr ImU32 orchid                 = IM_COL32(239, 132, 239, 255);
constexpr ImU32 pale_goldenrod         = IM_COL32(238, 232, 170, 255);
constexpr ImU32 pale_green             = IM_COL32(115, 222, 120, 255);
constexpr ImU32 pale_turquoise         = IM_COL32(175, 238, 238, 255);
constexpr ImU32 pale_violet_red        = IM_COL32(219, 112, 147, 255);
constexpr ImU32 papaya_whip            = IM_COL32(255, 239, 213, 255);
constexpr ImU32 peach_puff             = IM_COL32(255, 218, 185, 255);
constexpr ImU32 peru                   = IM_COL32(205, 133, 63, 255);
constexpr ImU32 pink                   = IM_COL32(255, 181, 197, 255);
constexpr ImU32 plum                   = IM_COL32(197, 72, 155, 255);
constexpr ImU32 powder_blue            = IM_COL32(176, 224, 230, 255);
constexpr ImU32 purple                 = IM_COL32(160, 32, 240, 255);
constexpr ImU32 red                    = IM_COL32(255, 0, 0, 255);
constexpr ImU32 rosy_brown             = IM_COL32(188, 143, 143, 255);
constexpr ImU32 royal_blue             = IM_COL32(65, 105, 225, 255);
constexpr ImU32 saddle_brown           = IM_COL32(139, 69, 19, 255);
constexpr ImU32 salmon                 = IM_COL32(233, 150, 122, 255);
constexpr ImU32 sandy_brown            = IM_COL32(244, 164, 96, 255);
constexpr ImU32 sea_green              = IM_COL32(82, 149, 132, 255);
constexpr ImU32 sea_shell              = IM_COL32(255, 245, 238, 255);
constexpr ImU32 sienna                 = IM_COL32(150, 82, 45, 255);
constexpr ImU32 sky_blue               = IM_COL32(114, 159, 255, 255);
constexpr ImU32 slate_blue             = IM_COL32(126, 136, 171, 255);
constexpr ImU32 slate_gray             = IM_COL32(112, 128, 144, 255);
constexpr ImU32 snow                   = IM_COL32(255, 250, 250, 255);
constexpr ImU32 spring_green           = IM_COL32(65, 172, 65, 255);
constexpr ImU32 steel_blue             = IM_COL32(84, 112, 170, 255);
constexpr ImU32 tan                    = IM_COL32(222, 184, 135, 255);
constexpr ImU32 thistle                = IM_COL32(216, 191, 216, 255);
constexpr ImU32 tomato                 = IM_COL32(255, 99, 71, 255);
constexpr ImU32 transparent            = IM_COL32(0, 0, 1, 255);
constexpr ImU32 turquoise              = IM_COL32(25, 204, 223, 255);
constexpr ImU32 violet                 = IM_COL32(156, 62, 206, 255);
constexpr ImU32 violet_red             = IM_COL32(243, 62, 150, 255);
constexpr ImU32 wheat                  = IM_COL32(245, 222, 179, 255);
constexpr ImU32 white                  = IM_COL32(255, 255, 255, 255);
constexpr ImU32 white_smoke            = IM_COL32(245, 245, 245, 255);
constexpr ImU32 yellow                 = IM_COL32(255, 255, 0, 255);
constexpr ImU32 yellow_green           = IM_COL32(50, 216, 56, 255);

// greys
constexpr ImU32 gray[] = {
    IM_COL32(0, 0, 0, 0),       IM_COL32(3, 3, 3, 0),
    IM_COL32(5, 5, 5, 0),       IM_COL32(8, 8, 8, 0),
    IM_COL32(10, 10, 10, 0),    IM_COL32(13, 13, 13, 0),
    IM_COL32(15, 15, 15, 0),    IM_COL32(18, 18, 18, 0),
    IM_COL32(20, 20, 20, 0),    IM_COL32(23, 23, 23, 0),
    IM_COL32(26, 26, 26, 0),    IM_COL32(28, 28, 28, 0),
    IM_COL32(31, 31, 31, 0),    IM_COL32(33, 33, 33, 0),
    IM_COL32(36, 36, 36, 0),    IM_COL32(38, 38, 38, 0),
    IM_COL32(41, 41, 41, 0),    IM_COL32(43, 43, 43, 0),
    IM_COL32(46, 46, 46, 0),    IM_COL32(48, 48, 48, 0),
    IM_COL32(51, 51, 51, 0),    IM_COL32(54, 54, 54, 0),
    IM_COL32(56, 56, 56, 0),    IM_COL32(59, 59, 59, 0),
    IM_COL32(61, 61, 61, 0),    IM_COL32(64, 64, 64, 0),
    IM_COL32(66, 66, 66, 0),    IM_COL32(69, 69, 69, 0),
    IM_COL32(71, 71, 71, 0),    IM_COL32(74, 74, 74, 0),
    IM_COL32(77, 77, 77, 0),    IM_COL32(79, 79, 79, 0),
    IM_COL32(82, 82, 82, 0),    IM_COL32(84, 84, 84, 0),
    IM_COL32(87, 87, 87, 0),    IM_COL32(89, 89, 89, 0),
    IM_COL32(92, 92, 92, 0),    IM_COL32(94, 94, 94, 0),
    IM_COL32(97, 97, 97, 0),    IM_COL32(99, 99, 99, 0),
    IM_COL32(102, 102, 102, 0), IM_COL32(105, 105, 105, 0),
    IM_COL32(107, 107, 107, 0), IM_COL32(110, 110, 110, 0),
    IM_COL32(112, 112, 112, 0), IM_COL32(115, 115, 115, 0),
    IM_COL32(117, 117, 117, 0), IM_COL32(120, 120, 120, 0),
    IM_COL32(122, 122, 122, 0), IM_COL32(125, 125, 125, 0),
    IM_COL32(127, 127, 127, 0), IM_COL32(130, 130, 130, 0),
    IM_COL32(133, 133, 133, 0), IM_COL32(135, 135, 135, 0),
    IM_COL32(138, 138, 138, 0), IM_COL32(140, 140, 140, 0),
    IM_COL32(143, 143, 143, 0), IM_COL32(145, 145, 145, 0),
    IM_COL32(148, 148, 148, 0), IM_COL32(150, 150, 150, 0),
    IM_COL32(153, 153, 153, 0), IM_COL32(156, 156, 156, 0),
    IM_COL32(158, 158, 158, 0), IM_COL32(161, 161, 161, 0),
    IM_COL32(163, 163, 163, 0), IM_COL32(166, 166, 166, 0),
    IM_COL32(168, 168, 168, 0), IM_COL32(171, 171, 171, 0),
    IM_COL32(173, 173, 173, 0), IM_COL32(176, 176, 176, 0),
    IM_COL32(179, 179, 179, 0), IM_COL32(181, 181, 181, 0),
    IM_COL32(184, 184, 184, 0), IM_COL32(186, 186, 186, 0),
    IM_COL32(189, 189, 189, 0), IM_COL32(191, 191, 191, 0),
    IM_COL32(194, 194, 194, 0), IM_COL32(196, 196, 196, 0),
    IM_COL32(199, 199, 199, 0), IM_COL32(201, 201, 201, 0),
    IM_COL32(204, 204, 204, 0), IM_COL32(207, 207, 207, 0),
    IM_COL32(209, 209, 209, 0), IM_COL32(212, 212, 212, 0),
    IM_COL32(214, 214, 214, 0), IM_COL32(217, 217, 217, 0),
    IM_COL32(219, 219, 219, 0), IM_COL32(222, 222, 222, 0),
    IM_COL32(224, 224, 224, 0), IM_COL32(227, 227, 227, 0),
    IM_COL32(229, 229, 229, 0), IM_COL32(232, 232, 232, 0),
    IM_COL32(235, 235, 235, 0), IM_COL32(237, 237, 237, 0),
    IM_COL32(240, 240, 240, 0), IM_COL32(242, 242, 242, 0),
    IM_COL32(245, 245, 245, 0), IM_COL32(247, 247, 247, 0),
    IM_COL32(250, 250, 250, 0), IM_COL32(252, 252, 252, 0),
    IM_COL32(255, 255, 255, 0),
};
} // namespace colors
inline namespace align {
template <typename T> struct align_left {
    T widget;
    constexpr void render(bounds const &r) const
    {
        widget.render({r.ul, vec2(r.ul.x + size().x, r.lr.y)});
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_center {
    T widget;
    constexpr void render(bounds const &r) const
    {
        auto half   = size().x / 2.0f;
        auto center = (r.ul.x + r.lr.x) / 2.0f;
        widget.render(
            {vec2(center - half, r.ul.y), vec2(center + half, r.lr.y)});
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_right {
    T widget;
    constexpr void render(bounds const &r) const
    {
        widget.render({vec2(r.lr.x - size().x, r.ul.y), r.lr});
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_top {
    T widget;
    constexpr void render(bounds const &r) const
    {
        widget.render({r.ul, vec2{r.lr.x, r.ul.y + size().y}});
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_middle {
    T widget;
    constexpr void render(bounds const &r) const
    {
        auto half   = size().y / 2.0f;
        auto middle = (r.ul.y + r.lr.y) / 2.0f;
        widget.render(
            {vec2(r.ul.x, middle - half), vec2(r.lr.x, middle + half)});
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_bottom {
    T widget;
    constexpr void render(bounds const &r) const
    {
        widget.render({vec2(r.ul.x, r.lr.y - size().y), r.lr});
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_left_top {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_left{align_top{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_center_top {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_center{align_top{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_right_top {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_right{align_top{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_left_middle {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_left{align_middle{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_center_middle {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_center{align_middle{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_right_middle {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_right{align_middle{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_left_bottom {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_left{align_bottom{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_center_bottom {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_center{align_bottom{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
template <typename T> struct align_right_bottom {
    T widget;
    constexpr void render(bounds const &r) const
    {
        align_right{align_bottom{widget}}.render(r);
    }
    constexpr auto size() const -> vec2 { return widget.size(); }
};
} // namespace align
inline namespace layouts {
// renders: first to last in order applying the size of the largest element
// size: largest element
template <typename... Ts> struct stack {
    std::tuple<Ts...> items;
    constexpr stack(Ts &&...ts) : items{ts...} {}
    constexpr void render(bounds const &r) const
    {
        boost::mp11::tuple_for_each(items,
                                    [&r](const auto &x) { x.render(r); });
    }
    constexpr auto size() const -> vec2
    {
        auto max = vec2();
        boost::mp11::tuple_for_each(items, [&max](const auto &x) mutable {
            auto sz = x.size();
            max     = vec2(std::max(max.x, sz.x), std::max(max.y, sz.y));
        });
        return max;
    }
};
template <typename... Ts> struct vertical {
    std::tuple<Ts...> items;
    constexpr vertical(Ts &&...ts) : items{ts...} {}
    constexpr void render(bounds const &r) const
    {
        boost::mp11::tuple_for_each(items, [remain = r](const auto &x) mutable {
            x.render(remain);
            remain.ul.y += x.size().y;
        });
    }
    constexpr auto size() const -> vec2
    {
        auto total = vec2();
        boost::mp11::tuple_for_each(items, [&total](const auto &x) mutable {
            auto sz = x.size();
            total   = vec2(std::max(total.x, sz.x), total.y + sz.y);
        });
        return total;
    }
};
template <typename... Ts> struct horizontal {
    std::tuple<Ts...> items;
    constexpr horizontal(Ts &&...ts) : items{ts...} {}
    constexpr void render(bounds const &r) const
    {
        boost::mp11::tuple_for_each(items, [remain = r](const auto &x) mutable {
            x.render(remain);
            remain.ul.x += x.size().x;
        });
    }
    constexpr auto size() const -> vec2
    {
        auto total = vec2();
        boost::mp11::tuple_for_each(items, [&total](const auto &x) mutable {
            auto sz = x.size();
            total   = vec2(total.x + sz.x, std::max(total.y, sz.y));
        });
        return total;
    }
};
} // namespace layouts
} // namespace widget
