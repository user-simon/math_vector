#include "gtest/gtest.h"
#include "math_vector.h"

#define EXPECT_EQ_FL(a, b) EXPECT_TRUE(std::abs(a - b) < 0.0001)

constexpr uint2d constexpr_a = uint2d(7, 2);

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

    float3d c(9.5f);
    EXPECT_TRUE(c.x == 9.5f, c.y == 9.5f, c.z == 9.5f);
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
    EXPECT_EQ_FL(a.normalize().length(), 1);
    EXPECT_EQ_FL(a.scale_to(3).length(), 3);
    EXPECT_EQ_FL(a.distance(b), 3.60555);
}

TEST(math, linear)
{
    int2d a(13, 7);
    int2d b(34, 23);

    EXPECT_EQ_FL(a.angle(b), 0.10081);
    EXPECT_EQ(a.dot(b), 603);
}

TEST(math, misc)
{
    double2d a(0.123, 0.987);
    double2d b(-123.321, -987.789);

    EXPECT_EQ_FL(a.sum(), 1.11);
    EXPECT_EQ(double2d(b.abs()), double2d(123.321, 987.789));
}

TEST(utility, utility)
{
    uint2d a(1,2);
    
    EXPECT_EQ(a.to_string(), "x: 1\ny: 2\n");
    EXPECT_EQ(a.to_string(false), "x: 1 y: 2 ");
    EXPECT_EQ(a.to_string("asdf", false), "asdf  x: 1 y: 2 ");
}
