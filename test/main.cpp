#include "mqtt.h"
#include "tiff.h"
#include "endian.h"
#include <cassert>
#include <fstream>
struct base {
    char c;
};
BOOST_DESCRIBE_STRUCT(base, (), (c))
struct test : base {
    int   a;
    float b;
};
BOOST_DESCRIBE_STRUCT(test, (base), (a, b))
auto test_serial()
{
    std::array<std::byte, 2048> buffer;
    using namespace bytes;
    using namespace tiff;
    using namespace endian;
    to(buffer.data(), types::long_, tags::image_width, be_int16(5),
       test{{'0'}, 6, 1.2f});
    assert(buffer[0] == std::byte{4});
    assert(buffer[1] == std::byte{0});
    assert(buffer[2] == std::byte{0});
    assert(buffer[3] == std::byte{1});
    assert(buffer[4] == std::byte{0});
    assert(buffer[5] == std::byte{5});
    assert(buffer[6] == std::byte{0x30});
    assert(buffer[7] == std::byte{6});
    assert(buffer[8] == std::byte{0});
    assert(buffer[9] == std::byte{0});
    assert(buffer[10] == std::byte{0});
    types    l;
    tags     i;
    be_int16 b;
    test     t;
    auto     ptr = from(buffer.data(), l, i, b, t);
    assert(l == types::long_);
    assert(i == tags::image_width);
    assert(b.value == 5);
    assert(t.c == '0');
    assert(t.a == 6);
    assert(t.b == 1.2f);
}
auto test_tiff()
{
    std::array<std::byte, 2048>      buffer;
    constexpr auto                   width  = uint32_t{256};
    constexpr auto                   height = uint32_t{256};
    std::array<char, width * height> image;
    std::iota(image.begin(), image.end(), uint8_t{0});
    auto ptr = tiff::header(buffer.data(), width * height + 8 + 16);
    ptr      = bytes::to(ptr, width, uint32_t{1}, height, uint32_t{1});
    auto ofs = std::ofstream("test.tif", std::ios_base::binary);
    ofs.write(reinterpret_cast<char *>(buffer.data()), ptr - buffer.data());
    ofs.write(image.data(), image.size());
    ptr =
        tiff::grayscale_ifd(buffer.data(), width, height, 8, 8 + 16, 8, 8 + 8);
    ptr = bytes::to(ptr, uint32_t{0});
    ofs.write(reinterpret_cast<char *>(buffer.data()), ptr - buffer.data());
}
auto test_tiff16()
{
    std::array<std::byte, 2048>      buffer;
    constexpr auto                   width  = uint32_t{256};
    constexpr auto                   height = uint32_t{256};
    std::array<char, width * height * 2> image;
    std::iota(image.begin(), image.end(), uint8_t{0});
    auto ptr = tiff::header(buffer.data(), width * height * 2 + 8 + 16);
    ptr      = bytes::to(ptr, width, uint32_t{1}, height, uint32_t{1});
    auto ofs = std::ofstream("test16.tif", std::ios_base::binary);
    ofs.write(reinterpret_cast<char *>(buffer.data()), ptr - buffer.data());
    ofs.write(image.data(), image.size());
    ptr =
        tiff::grayscale_ifd(buffer.data(), width, height, 16, 8 + 16, 8, 8 + 8);
    ptr = bytes::to(ptr, uint32_t{0});
    ofs.write(reinterpret_cast<char *>(buffer.data()), ptr - buffer.data());
}
int main(int ac, char **av)
{
    test_serial();
    test_tiff();
    test_tiff16();
}
