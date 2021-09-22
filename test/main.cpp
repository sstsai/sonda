#include "mqtt.h"
#include "tiff.h"
#include "endian.h"
#include <cassert>
struct base {
    char c;
};
BOOST_DESCRIBE_STRUCT(base, (), (c))
struct test : base {
    int a;
    float b;
};
BOOST_DESCRIBE_STRUCT(test, (base), (a, b))
int main(int ac, char **av)
{
    std::array<std::byte, 2048> buffer;
    using namespace bytes;
    using namespace tiff;
    using namespace endian;
    to(buffer.data(), types::long_, tags::image_width, be_int16(5),
       test{{'0'}, 6, 1.2f});
    assert(buffer[0] == std::byte{3});
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
    types l;
    tags  i;
    be_int16 b;
    test     t;
    auto ptr = from(buffer.data(), l, i, b, t);
    assert(l == types::long_);
    assert(i == tags::image_width);
    assert(b.value == 5);
    assert(t.c == '0');
    assert(t.a == 6);
    assert(t.b == 1.2f);
}
