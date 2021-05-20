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
}

TEST(expressions, expressions)
{
    int2d a(5 ,2);
    int2d b(7 ,4);
    
    float2d c = a + b;

    EXPECT_EQ(a.x + b.x, c.x);
    EXPECT_EQ(a.y + b.y, c.y);

    c = float2d(10, 9);

    float2d d = a + b * c;

    EXPECT_EQ(a.x + b.x * c.x, d.x);
    EXPECT_EQ(a.y + b.y * c.y, d.y);
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

TEST(math, length)
{
    uint2d a(1, 1);
    double2d b(3, 4);

    EXPECT_EQ(a.length(), std::sqrt(2));
    EXPECT_EQ(b.length(), 5);
    EXPECT_FLOAT_EQ(a.normalize().length(), 1);
    EXPECT_FLOAT_EQ(a.scale_to(3).length(), 3);
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

    EXPECT_EQ(coords.x, std::cos(angle));
    EXPECT_EQ(coords.y, std::sin(angle));
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
    
    EXPECT_EQ(a.to_string(), "x: 1\ny: 2\n");
    EXPECT_EQ(a.to_string(false), "x: 1 y: 2 ");
    EXPECT_EQ(a.to_string("asdf", false), "asdf  x: 1 y: 2 ");
}
