#pragma once

#include "gtest/gtest.h"
#include "Factorio.h"

TEST(SampleTest, sampleTestCase)
{
    EXPECT_EQ(1, 1);
    EXPECT_EQ(1, 0);
    EXPECT_EQ(1, Factorial(-5));
    EXPECT_EQ(1, Factorial(-1));
    EXPECT_GT(Factorial(-10), 0);
}

TEST(YetAnotherTest, sampleTestCase)
{
    EXPECT_EQ(1, 1);
    EXPECT_EQ(1, Factorial(-5));
    EXPECT_EQ(1, Factorial(-1));
    EXPECT_GT(Factorial(-10), 0);
}