#include <gtest/gtest.h>

int main(int a, char **b)
{
    testing::InitGoogleTest(&a, b);
    return ::RUN_ALL_TESTS();
}
