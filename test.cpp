#include <gtest/gtest.h>
#include "math_vector.h"

constexpr uint2d constexpr_a = uint2d(7, 2);
constexpr float2d constexpr_b = float2d(10.2, 5.6);

TEST(compile_time_evaluation, compile_time_evaluation)
{
    constexpr bool   _1 = constexpr_a;
    constexpr uint   _2 = constexpr_a[0];
    constexpr bool   _3 = constexpr_a == constexpr_b;
    constexpr bool   _4 = constexpr_a != constexpr_b;
    constexpr double _5 = constexpr_a.length2();
    constexpr uint   _6 = constexpr_a.sum();
}

TEST(constructors, constructors)
{
    uint2d a(5.2, 2.5);
    
    EXPECT_TRUE(a.x == a[0] && a.x == 5);
    EXPECT_TRUE(a.y == a[1] && a.y == 2);

    a.x = 10;
    a.y = 15;

    EXPECT_TRUE(a.x == a[0]);
    EXPECT_TRUE(a.y == a[1]);

    double2d b = a;

    EXPECT_TRUE(b.x == a.x);
    EXPECT_TRUE(b.y == a.y);

    b.x = 100;
    b.y = 200;

    EXPECT_TRUE(b.x != a.x);
    EXPECT_TRUE(b.y != a.y);

    binary2d c = uint2d(5, 0);
    EXPECT_TRUE(c.x);
    EXPECT_FALSE(c.y);
}

TEST(operators, bool_conversion)
{
    EXPECT_FALSE(double2d(0, 0));
    EXPECT_TRUE (double2d(0, 1));
    EXPECT_TRUE (int2d(-1, 1));
}

TEST(operators, equals)
{
    EXPECT_EQ(uint2d(1, 1), double2d(1, 1));
}

TEST(operators, plus_minus)
{
    int2d a(5, 2);
    int2d b(7, 4);

    float2d c = a + b;
    EXPECT_EQ(c, float2d(a.x + b.x, a.y + b.y ));

    c += char2d(2, 5);
    EXPECT_EQ(c, float2d(a.x + b.x + 2, a.y + b.y + 5));
}

TEST(operators, mult_div)
{
    float2d a(10, 9);
    float2d b(5.5, 9.25);
    float2d c(10.123, 999.123);

    float2d d = a * b / c;
    EXPECT_EQ(d, float2d(a.x * b.x / c.x, a.y * b.y / c.y));

    float2d e = d * 5;
    EXPECT_EQ(e, float2d(d.x * 5, d.y * 5));

    float2d f = e / 100;
    EXPECT_EQ(f, float2d(e.x / 100, e.y / 100));

    int2d g(10, 4);
    int2d h = g % 3;
    EXPECT_EQ(h, int2d(1, 1));
}

TEST(operators, bitwise)
{
    char2d a(0b11, 0b10);
    char2d b(0b10, 0b01);

    char2d c = a | b;
    EXPECT_EQ(c, char2d(0b11, 0b11));

    char2d d = a & b;
    EXPECT_EQ(d, char2d(0b10, 0b00));

    char2d e = a ^ b;
    EXPECT_EQ(e, char2d(0b01, 0b11));

    char2d f = a >> 1;
    EXPECT_EQ(f, char2d(0b01, 0b01));

    char2d g = b << 1;
    EXPECT_EQ(g, char2d(0b100, 0b010));

    char2d h = ~a;
    EXPECT_EQ(h, char2d(~0b11, ~0b10));
}

TEST(operators, negation)
{
    double2d a(5.5, 3.3);
    double2d b = -a;
    EXPECT_EQ(b, double2d(-a.x, -a.y));
}

TEST(math, length)
{
    uint2d a(1, 1);
    double2d b(3, 4);

    EXPECT_EQ(a.length(), std::sqrt(2));
    EXPECT_EQ(b.length(), 5);
    EXPECT_FLOAT_EQ(a.normalize().length(), 1);
    EXPECT_FLOAT_EQ(a.set_length(3).length(), 3);
    EXPECT_FLOAT_EQ(a.distance(b), 3.6055512);
}

TEST(math, linear)
{
    int2d a(13, 7);
    int2d b(34, 23);

    EXPECT_FLOAT_EQ(a.angle(), 0.49394137);
    EXPECT_FLOAT_EQ(a.delta_angle(b), 0.10081789);

    double angle = 0.10081;
    double2d coords = mv_util::angle_coords(angle);
    EXPECT_EQ(coords, double2d(std::cos(angle), std::sin(angle)));

    EXPECT_EQ(a.dot(b), 603);
}

TEST(math, misc)
{
    double2d a(0.123, 0.987);
    double2d b(-123.321, -987.789);

    EXPECT_FLOAT_EQ(a.sum(), 1.11);
    EXPECT_EQ(double2d(b.abs()), double2d(123.321, 987.789));
}

TEST(utility, utility)
{
    uint2d a(1,2);

    EXPECT_EQ(a.to_string({}, true), "x: 1\ny: 2\n");
    EXPECT_EQ(a.to_string(), "x: 1 y: 2 ");
    EXPECT_EQ(a.to_string("asdf"), "asdf  x: 1 y: 2 ");
}
